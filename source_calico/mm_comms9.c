#include <maxmod9.h>
#include <mm_msl.h>
#include "mm_pxi.h"

mm_ds_system s_mmState;
static mm_callback s_mmCallback;

MM_INLINE void _mmIssueCmd(mmPxiCmd cmd, unsigned imm, const void* arg, size_t arg_size)
{
	unsigned arg_size_words = (arg_size + 3) / 4;
	unsigned msg = mmPxiMakeCmdMsg(cmd, imm);

	// TODO: Check credits

#ifdef SYS_CALICO
	if (arg_size) {
		pxiSendWithData(PxiChannel_Maxmod, msg, (const u32*)arg, arg_size_words);
	} else {
		pxiSend(PxiChannel_Maxmod, mmPxiMakeCmdMsg(cmd, imm));
	}
#else
	fifoSendMsg(FIFO_MAXMOD, msg, arg_size_words, (const u32*)arg);
#endif
}

#ifdef SYS_CALICO
static void _mmPxiHandler(void* user, u32 msg)
#else
static void _mmPxiHandler(unsigned extra_words, void* user, u32 msg)
#endif
{
	mmPxiEvent evt = mmPxiEventGetType(msg);
	unsigned imm = mmPxiEventGetImm(msg);

	switch (evt) {
		default: break;

		case mmPxiEvent_Credits: {
			// TODO: Handle
			break;
		}

		case mmPxiEvent_Status: {
			// TODO: Handle
			break;
		}

		case mmPxiEvent_SongMsg:
		case mmPxiEvent_SongEnd: {
			if (s_mmCallback) {
				s_mmCallback(evt == mmPxiEvent_SongMsg ? MMCB_SONGMESSAGE : MMCB_SONGFINISHED, imm);
			}
			break;
		}
	}
}

void mmInit(const mm_ds_system* system)
{
	s_mmState = *system;

#ifdef SYS_CALICO
	pxiSetHandler(PxiChannel_Maxmod, _mmPxiHandler, NULL);
	pxiWaitRemote(PxiChannel_Maxmod);
#else
	fifoSetMsgHandler(FIFO_MAXMOD, _mmPxiHandler, NULL);
	fifoWaitRemote(FIFO_MAXMOD);
#endif

	mmPxiArgBank arg = {
		.num_songs = system->mod_count,
		.mm_bank   = system->mem_bank,
	};

	_mmIssueCmd(mmPxiCmd_Bank, 0, &arg, sizeof(arg));
}

void mmSelectMode(mm_mode_enum mode)
{
	_mmIssueCmd(mmPxiCmd_SelectMode, mode, NULL, 0);
}

static void _mmLockUnlockChannels(mm_word bitmask, unsigned lock)
{
	mmPxiImmSelChan u = {
		.mask = bitmask,
		.lock = lock,
	};

	_mmIssueCmd(mmPxiCmd_SelChan, u.imm, NULL, 0);
}

void mmLockChannels(mm_word bitmask)
{
	_mmLockUnlockChannels(bitmask, 1);
}

void mmUnlockChannels(mm_word bitmask)
{
	_mmLockUnlockChannels(bitmask, 0);
}

void mmSetEventHandler(mm_callback handler)
{
	s_mmCallback = handler;
}

void mmStart(mm_word module_ID, mm_pmode mode)
{
	mmPxiImmStart u = {
		.id   = module_ID,
		.mode = mode,
	};

	_mmIssueCmd(mmPxiCmd_Start, u.imm, NULL, 0);
}

void mmPause(void)
{
	_mmIssueCmd(mmPxiCmd_Pause, 0, NULL, 0);
}

void mmResume(void)
{
	_mmIssueCmd(mmPxiCmd_Resume, 0, NULL, 0);
}

void mmStop(void)
{
	_mmIssueCmd(mmPxiCmd_Stop, 0, NULL, 0);
}

void mmPosition(mm_word position)
{
	// TODO
}

void mmJingle(mm_word module_ID)
{
	_mmIssueCmd(mmPxiCmd_StartSub, module_ID, NULL, 0);
}

void mmSetModuleVolume(mm_word volume)
{
	_mmIssueCmd(mmPxiCmd_MasterVol, volume, NULL, 0);
}

void mmSetJingleVolume(mm_word volume)
{
	_mmIssueCmd(mmPxiCmd_MasterVolSub, volume, NULL, 0);
}

void mmSetModuleTempo(mm_word tempo)
{
	_mmIssueCmd(mmPxiCmd_MasterTempo, tempo, NULL, 0);
}

void mmSetModulePitch(mm_word pitch)
{
	_mmIssueCmd(mmPxiCmd_MasterPitch, pitch, NULL, 0);
}
