#include <maxmod9.h>
#include <mm_msl.h>
#include "mm_pxi.h"

mm_ds_system s_mmState;
static mm_bool s_mmActiveStatus;
static mm_callback s_mmCallback;

static struct {
	u16 used_mask;
	u8 instances[16];
} s_mmEffState;

MM_INLINE void _mmIssueCmd(mmPxiCmd cmd, unsigned imm, const void* arg, size_t arg_size)
{
	unsigned arg_size_words = (arg_size + 3) / 4;
	unsigned msg = mmPxiMakeCmdMsg(cmd, imm);

	// TODO: Check credits

	if (arg_size) {
		pxiSendWithData(PxiChannel_Maxmod, msg, (const u32*)arg, arg_size_words);
	} else {
		pxiSend(PxiChannel_Maxmod, mmPxiMakeCmdMsg(cmd, imm));
	}
}

static void _mmPxiHandler(void* user, u32 msg)
{
	mmPxiEvent evt = mmPxiEventGetType(msg);
	unsigned imm = mmPxiEventGetImm(msg);

	switch (evt) {
		default: break;

		case mmPxiEvent_Credits: {
			// TODO: Handle
			break;
		}

		case mmPxiEvent_EffEnd: {
			s_mmEffState.used_mask &= ~imm;
			break;
		}

		case mmPxiEvent_SongEnd: {
			s_mmActiveStatus = false;
			// fallthrough
		}

		case mmPxiEvent_SongMsg: {
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

	pxiSetHandler(PxiChannel_Maxmod, _mmPxiHandler, NULL);
	pxiWaitRemote(PxiChannel_Maxmod);

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
	s_mmActiveStatus = true;
}

void mmPause(void)
{
	_mmIssueCmd(mmPxiCmd_Pause, 0, NULL, 0);
	s_mmActiveStatus = false;
}

void mmResume(void)
{
	_mmIssueCmd(mmPxiCmd_Resume, 0, NULL, 0);
	s_mmActiveStatus = true;
}

void mmStop(void)
{
	_mmIssueCmd(mmPxiCmd_Stop, 0, NULL, 0);
	s_mmActiveStatus = false;
}

void mmPosition(mm_word position)
{
	_mmIssueCmd(mmPxiCmd_Position, position, NULL, 0);
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

mm_bool mmActive(void)
{
	return s_mmActiveStatus;
}

MM_INLINE bool _mmValidateEffectHandle(mm_sfxhand handle)
{
	int id = (handle & 0xff) - 1;
	int inst = handle >> 8;

	return id >= 0 && id < 16 && (s_mmEffState.used_mask & (1U<<id)) && s_mmEffState.instances[id] == inst;
}

static mm_sfxhand _mmCreateEffectHandle(void)
{
	IrqState st = irqLock();

	int id = __builtin_ffs((u16)~s_mmEffState.used_mask)-1;
	if (id >= 0) {
		s_mmEffState.used_mask |= 1U << id;
	}

	irqUnlock(st);

	if (id < 0) {
		return 0;
	}

	return (id+1) | ((++s_mmEffState.instances[id]) << 8);
}

mm_sfxhand mmEffect(mm_word sample_ID)
{
	mmPxiArgEffect arg = {
		.handle = _mmCreateEffectHandle(),
		.arg    = sample_ID,
	};

	if (arg.handle) {
		_mmIssueCmd(mmPxiCmd_Effect, 0, &arg, sizeof(arg));
	}

	return arg.handle;
}

mm_sfxhand mmEffectEx(const mm_sound_effect* sound)
{
	mm_sound_effect arg = *sound; // struct copy

	if (!_mmValidateEffectHandle(arg.handle)) {
		arg.handle = _mmCreateEffectHandle();
	}

	if (arg.handle) {
		_mmIssueCmd(mmPxiCmd_EffectEx, 0, &arg, sizeof(arg));
	}

	return arg.handle;
}

void mmEffectVolume(mm_sfxhand handle, mm_byte volume)
{
	mmPxiArgEffect arg = {
		.handle = handle,
		.arg    = volume,
	};

	_mmIssueCmd(mmPxiCmd_EffectVol, 0, &arg, sizeof(arg));
}

void mmEffectPanning(mm_sfxhand handle, mm_byte panning)
{
	mmPxiArgEffect arg = {
		.handle = handle,
		.arg    = panning,
	};

	_mmIssueCmd(mmPxiCmd_EffectPan, 0, &arg, sizeof(arg));
}

void mmEffectRate(mm_sfxhand handle, mm_word rate)
{
	mmPxiArgEffect arg = {
		.handle = handle,
		.arg    = rate,
	};

	_mmIssueCmd(mmPxiCmd_EffectRate, 0, &arg, sizeof(arg));
}

void mmEffectScaleRate(mm_sfxhand handle, mm_word factor)
{
	mmPxiArgEffect arg = {
		.handle = handle,
		.arg    = factor,
	};

	_mmIssueCmd(mmPxiCmd_EffectMulRate, 0, &arg, sizeof(arg));
}

static void _mmEffectOpt(mm_sfxhand handle, unsigned opt)
{
	mmPxiImmEffectOpt u = {
		.handle = handle,
		.opt    = opt,
	};

	_mmIssueCmd(mmPxiCmd_EffectOpt, u.imm, NULL, 0);
}

void mmEffectCancel(mm_sfxhand handle)
{
	_mmEffectOpt(handle, 0);
}

void mmEffectRelease(mm_sfxhand handle)
{
	_mmEffectOpt(handle, 1);
}

void mmSetEffectsVolume(mm_word volume)
{
	_mmIssueCmd(mmPxiCmd_MasterEffectVol, volume, NULL, 0);
}

void mmEffectCancelAll()
{
	_mmIssueCmd(mmPxiCmd_EffectCancelAll, 0, NULL, 0);
}
