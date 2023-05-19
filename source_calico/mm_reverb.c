#include <calico.h>
#include <maxmod9.h>
#include "mm_pxi.h"

static enum {
	mm_reverb_disabled,
	mm_reverb_stopped,
	mm_reverb_started,
} s_mmReverbState;

static struct {
	mm_addr  buf;
	mm_hword len;
	mm_hword timer;
	mm_hword vol;
	mm_byte  pan;
	mm_byte  fmt;
} s_mmReverbCap[2];

static u8 s_mmReverbDry;

static void _mmReverbCopyDrySettings(void)
{
	soundSetMixerConfig(
		(s_mmReverbDry & 1) ? SoundOutSrc_Mixer : SoundOutSrc_Ch1,
		(s_mmReverbDry & 2) ? SoundOutSrc_Mixer : SoundOutSrc_Ch3,
		false, false);
}

void mmReverbEnable(void)
{
	if (s_mmReverbState != mm_reverb_disabled) {
		return;
	}

	mmLockChannels((1U<<1) | (1U<<3));
	s_mmReverbState = mm_reverb_stopped;
	s_mmReverbDry = 3;
}

void mmReverbDisable(void)
{
	if (s_mmReverbState == mm_reverb_disabled) {
		return;
	}

	if (s_mmReverbState == mm_reverb_started) {
		mmReverbStop(MMRC_BOTH);
	}

	mmUnlockChannels((1U<<1) | (1U<<3));
	s_mmReverbState = mm_reverb_disabled;
}

void mmReverbConfigure(const mm_reverb_cfg* config)
{
	if (s_mmReverbState == mm_reverb_disabled) {
		return;
	}

	mm_word flags = config->flags;
	bool is_started = s_mmReverbState == mm_reverb_started;

	for (unsigned i = 0; i < 2; i ++) {
		if (!(flags & (MMRF_LEFT*(1U<<i)))) {
			continue;
		}

		if (flags & MMRF_MEMORY) {
			s_mmReverbCap[i].buf = config->memory;
		}

		if (flags & MMRF_DELAY) {
			s_mmReverbCap[i].len = config->delay;
		}

		if (flags & MMRF_RATE) {
			s_mmReverbCap[i].timer = config->rate;
			if (is_started) {
				soundChSetTimer(1+2*i, s_mmReverbCap[i].timer);
			}
		}

		if (flags & MMRF_FEEDBACK) {
			s_mmReverbCap[i].vol = config->feedback;
			if (is_started) {
				soundChSetVolume(1+2*i, s_mmReverbCap[i].vol);
			}
		}

		if (flags & MMRF_PANNING) {
			s_mmReverbCap[i].pan = config->panning;
			if (is_started) {
				soundChSetPan(1+2*i, s_mmReverbCap[i].pan);
			}
		}
	}

	if ((flags & (MMRF_LEFT|MMRF_INVERSEPAN)) == (MMRF_LEFT|MMRF_INVERSEPAN)) {
		unsigned pan = 128 - s_mmReverbCap[0].pan;
		s_mmReverbCap[1].pan = pan <= 127 ? pan : 127;
	}

	if (flags & MMRF_8BITLEFT) {
		s_mmReverbCap[0].fmt = SoundCapFmt_Pcm8;
	}

	if (flags & MMRF_16BITLEFT) {
		s_mmReverbCap[0].fmt = SoundCapFmt_Pcm16;
	}

	if (flags & MMRF_8BITRIGHT) {
		s_mmReverbCap[1].fmt = SoundCapFmt_Pcm8;
	}

	if (flags & MMRF_16BITRIGHT) {
		s_mmReverbCap[1].fmt = SoundCapFmt_Pcm16;
	}

	if (flags & (MMRF_NODRYLEFT|MMRF_NODRYRIGHT|MMRF_DRYLEFT|MMRF_DRYRIGHT)) {
		if (flags & MMRF_NODRYLEFT) {
			s_mmReverbDry &= ~1;
		}

		if (flags & MMRF_NODRYRIGHT) {
			s_mmReverbDry &= ~2;
		}

		if (flags & MMRF_DRYLEFT) {
			s_mmReverbDry |= 1;
		}

		if (flags & MMRF_DRYRIGHT) {
			s_mmReverbDry |= 2;
		}

		if (is_started) {
			_mmReverbCopyDrySettings();
		}
	}
}

void mmReverbStart(mm_reverbch channels)
{
	if (s_mmReverbState == mm_reverb_disabled || !(channels & MMRC_BOTH)) {
		return;
	}

	if (s_mmReverbState == mm_reverb_started) {
		mmReverbStop(channels);
	}

	u32 mask = 0;
	for (unsigned i = 0; i < 2; i ++) {
		if (!(channels & (1U<<i))) {
			continue;
		}

		armFillMem32(s_mmReverbCap[i].buf, 0, s_mmReverbCap[i].len*4);
		armDCacheFlush(s_mmReverbCap[i].buf, s_mmReverbCap[i].len*4);

		soundPrepareCap(i, SoundCapDst_ChBNormal, SoundCapSrc_Mixer, true,
			s_mmReverbCap[i].fmt, s_mmReverbCap[i].buf, s_mmReverbCap[i].len);

		soundPreparePcm(1+i*2, s_mmReverbCap[i].vol, s_mmReverbCap[i].pan,
			s_mmReverbCap[i].timer, SoundMode_Repeat,
			s_mmReverbCap[i].fmt == SoundCapFmt_Pcm16 ? SoundFmt_Pcm16 : SoundFmt_Pcm8,
			s_mmReverbCap[i].buf, 0, s_mmReverbCap[i].len);

		mask |= 1U<<(SOUND_NUM_CHANNELS+i);
		mask |= 1U<<(1+i*2);
	}

	_mmReverbCopyDrySettings();
	soundStart(mask);

	s_mmReverbState = mm_reverb_started;
}

void mmReverbStop(mm_reverbch channels)
{
	if (s_mmReverbState != mm_reverb_started || !(channels & MMRC_BOTH)) {
		return;
	}

	u32 mask = 0;
	for (unsigned i = 0; i < 2; i ++) {
		if (channels & (1U<<i)) {
			mask |= 1U<<(SOUND_NUM_CHANNELS+i);
			mask |= 1U<<(1+i*2);
		}
	}

	soundStop(mask);
	soundSetMixerConfig(SoundOutSrc_Mixer, SoundOutSrc_Mixer, false, false);

	s_mmReverbState = mm_reverb_stopped;
}
