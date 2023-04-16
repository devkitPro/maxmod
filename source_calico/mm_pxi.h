#pragma once

#include <calico.h>
#define MM_INLINE    MEOW_INLINE
#define MM_CONSTEXPR MEOW_CONSTEXPR
#define PxiChannel_Maxmod ((PxiChannel)22)

#define MM_PXI_NUM_CREDITS 64

typedef enum mmPxiCmd {
	mmPxiCmd_Bank            = 0,
	mmPxiCmd_SelChan         = 1,
	mmPxiCmd_Start           = 2,
	mmPxiCmd_Pause           = 3,
	mmPxiCmd_Resume          = 4,
	mmPxiCmd_Stop            = 5,
	mmPxiCmd_Position        = 6,
	mmPxiCmd_StartSub        = 7,
	mmPxiCmd_MasterVol       = 8,
	mmPxiCmd_MasterVolSub    = 9,
	mmPxiCmd_MasterTempo     = 10,
	mmPxiCmd_MasterPitch     = 11,
	mmPxiCmd_MasterEffectVol = 12,
	mmPxiCmd_SelectMode      = 15,
	mmPxiCmd_Effect          = 16,
	mmPxiCmd_EffectVol       = 17,
	mmPxiCmd_EffectPan       = 18,
	mmPxiCmd_EffectRate      = 19,
	mmPxiCmd_EffectMulRate   = 20,
	mmPxiCmd_EffectOpt       = 21,
	mmPxiCmd_EffectEx        = 22,
	mmPxiCmd_EffectCancelAll = 29,
} mmPxiCmd;

typedef enum mmPxiEvent {
	mmPxiEvent_Credits = 0,
	mmPxiEvent_EffEnd  = 1,
	mmPxiEvent_SongEnd = 2,
	mmPxiEvent_SongMsg = 3,
} mmPxiEvent;

typedef struct mmPxiArgBank {
	u16 num_songs;
	void* mm_bank;
} mmPxiArgBank;

typedef struct mmPxiArgEffect {
	u32 handle : 16;
	u32 arg    : 16;
} mmPxiArgEffect;

typedef union mmPxiImmSelChan {
	unsigned imm;
	struct {
		unsigned mask : 16;
		unsigned lock : 1;
	};
} mmPxiImmSelChan;

typedef union mmPxiImmStart {
	unsigned imm;
	struct {
		unsigned id   : 16;
		unsigned mode : 1;  // mm_pmode
	};
} mmPxiImmStart;

typedef union mmPxiImmEffectOpt {
	unsigned imm;
	struct {
		unsigned handle : 16;
		unsigned opt    : 5;
	};
} mmPxiImmEffectOpt;

MM_CONSTEXPR u32 mmPxiMakeCmdMsg(mmPxiCmd cmd, unsigned imm)
{
	return (cmd & 0x1f) | (imm << 5);
}

MM_CONSTEXPR mmPxiCmd mmPxiCmdGetType(u32 msg)
{
	return (mmPxiCmd)(msg & 0x1f);
}

MM_CONSTEXPR unsigned mmPxiCmdGetImm(u32 msg)
{
	return msg >> 5;
}

MM_CONSTEXPR u32 mmPxiMakeEventMsg(mmPxiEvent evt, unsigned imm)
{
	return (evt & 3) | (imm << 2);
}

MM_CONSTEXPR mmPxiEvent mmPxiEventGetType(u32 msg)
{
	return (mmPxiCmd)(msg & 3);
}

MM_CONSTEXPR unsigned mmPxiEventGetImm(u32 msg)
{
	return msg >> 2;
}
