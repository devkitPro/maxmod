#include <calico.h>
#include <maxmod7.h>
#include "mm_pxi.h"

#define MM_NUM_MAIL_SLOTS 2
#define MM_MAIL_EXIT      0
#define MM_MAIL_WAKEUP    1
#define MM_MAIL_TIMER     2

static Thread s_mmThread;
alignas(8) static u8 s_mmThreadStack[512];

static Mailbox s_mmMailbox, s_mmPxiMailbox;
static u32 s_mmPxiMailboxSlots[MM_PXI_NUM_CREDITS];

extern u8 mmInitialized;

MEOW_CODE32 MEOW_NOINLINE static void _mmProcessComms(void);

// Missing prototypes for "internal" maxmod functions
void mmMixerPre(void);
void mmUpdateEffects(void);
void mmPulse(void);
void mmMixerMix(void);
void mmGetSoundBank(u16 num_songs, void* mm_bank);

MEOW_CODE32 static void _mmTimerIsr(void)
{
	// Do nothing if maxmod is not initialized
	if (mmInitialized != 42) {
		return;
	}

	// Disable timer IRQ
	REG_TMxCNT_H(1) &= ~TIMER_ENABLE_IRQ;

	// Call timing critical mixer handler
	mmMixerPre();

	// Send timer message to maxmod thread
	mailboxTrySend(&s_mmMailbox, MM_MAIL_TIMER);
}

static void _mmPxiHandler(void* user, u32 data)
{
	// Add message to PXI mailbox
	mailboxTrySend((Mailbox*)user, data);

	// Wake up maxmod thread if needed
	if (s_mmMailbox.pending_slots == 0) {
		mailboxTrySend(&s_mmMailbox, MM_MAIL_WAKEUP);
	}
}

MEOW_INLINE void _mmSendEvent(mmPxiEvent evt, unsigned imm)
{
	pxiSend(PxiChannel_Maxmod, mmPxiMakeEventMsg(evt, imm));
}

static mm_word _mmEventForwarder(mm_word msg, mm_word param)
{
	switch (msg) {
		default: break;

		case MMCB_SONGMESSAGE:
			_mmSendEvent(mmPxiEvent_SongMsg, param);
			break;

		case MMCB_SONGFINISHED:
			_mmSendEvent(mmPxiEvent_SongEnd, param);
			break;
	}
	return 0;
}

static int _mmThreadMain(void* arg)
{
	// Set up timer IRQ
	irqSet(IRQ_TIMER1, _mmTimerIsr);
	irqEnable(IRQ_TIMER1);

	// Set up PXI and mailboxes
	u32 slots[MM_NUM_MAIL_SLOTS];
	mailboxPrepare(&s_mmMailbox, slots, MM_NUM_MAIL_SLOTS);
	mailboxPrepare(&s_mmPxiMailbox, s_mmPxiMailboxSlots, MM_PXI_NUM_CREDITS);
	pxiSetHandler(PxiChannel_Maxmod, _mmPxiHandler, &s_mmPxiMailbox);

	// Set up maxmod event forwarder
	mmSetEventHandler(_mmEventForwarder);

	for (;;) {
		// Get message
		u32 msg = mailboxRecv(&s_mmMailbox);
		if (msg == MM_MAIL_EXIT) {
			break;
		}

		// Handle periodic timer updates
		if (msg == MM_MAIL_TIMER) {
			mmUpdateEffects();
			mmPulse();
			mmMixerMix();

			// Reenable timer IRQ
			REG_TMxCNT_H(1) |= TIMER_ENABLE_IRQ;
		}

		// Handle PXI commands
		_mmProcessComms();
	}

	return 0;
}

void mmInstall(int thread_prio)
{
	threadPrepare(&s_mmThread, _mmThreadMain, NULL, &s_mmThreadStack[sizeof(s_mmThreadStack)], thread_prio);
	threadStart(&s_mmThread);
}

MEOW_NOINLINE static void _mmProcessPxiCmd(mmPxiCmd cmd, unsigned imm, const void* body, unsigned num_words)
{
	switch (cmd) {
		default: {
			//dietPrint("[MM] Unhandled cmd %u\n", cmd);
			break;
		}

		case mmPxiCmd_Bank: {
			const mmPxiArgBank* arg = (const mmPxiArgBank*)body;
			mmGetSoundBank(arg->num_songs, arg->mm_bank);
			break;
		}

		case mmPxiCmd_SelChan: {
			mmPxiImmSelChan u = { imm };
			if (u.lock) {
				mmLockChannels(u.mask);
			} else {
				mmUnlockChannels(u.mask);
			}
			break;
		}

		case mmPxiCmd_Start: {
			mmPxiImmStart u = { imm };
			mmStart(u.id, (mm_pmode)u.mode);
			break;
		}

		case mmPxiCmd_Pause: {
			mmPause();
			break;
		}

		case mmPxiCmd_Resume: {
			mmResume();
			break;
		}

		case mmPxiCmd_Stop: {
			mmStop();
			break;
		}

		case mmPxiCmd_Position: {
			mmPosition(imm);
			break;
		}

		case mmPxiCmd_StartSub: {
			mmJingle(imm);
			break;
		}

		case mmPxiCmd_MasterVol: {
			mmSetModuleVolume(imm);
			break;
		}

		case mmPxiCmd_MasterVolSub: {
			mmSetJingleVolume(imm);
			break;
		}

		case mmPxiCmd_MasterTempo: {
			mmSetModuleTempo(imm);
			break;
		}

		case mmPxiCmd_MasterPitch: {
			mmSetModulePitch(imm);
			break;
		}

		case mmPxiCmd_MasterEffectVol: {
			mmSetEffectsVolume(imm);
			break;
		}

		case mmPxiCmd_SelectMode: {
			mmSelectMode((mm_mode_enum)imm);
			break;
		}

		case mmPxiCmd_EffectCancelAll: {
			mmEffectCancelAll();
			break;
		}
	}
}

void _mmProcessComms(void)
{
	u32 cmdheader;
	unsigned credits = 0;
	while (mailboxTryRecv(&s_mmPxiMailbox, &cmdheader)) {
		unsigned num_words = cmdheader >> 26;
		if (num_words) {
			cmdheader &= (1U<<26)-1;
		}

		mmPxiCmd cmd = mmPxiCmdGetType(cmdheader);
		unsigned imm = mmPxiCmdGetImm(cmdheader);
		credits += 1 + num_words;

		if_likely (num_words == 0) {
			// Process command directly
			_mmProcessPxiCmd(cmd, imm, NULL, 0);
		} else {
			// Receive message body
			u32 body[num_words];
			for (unsigned i = 0; i < num_words; i ++) {
				body[i] = mailboxRecv(&s_mmPxiMailbox);
			}

			// Process command
			_mmProcessPxiCmd(cmd, imm, body, num_words);
		}
	}

	if (credits) {
		// Return credits to arm9
		_mmSendEvent(mmPxiEvent_Credits, credits);
	}
}
