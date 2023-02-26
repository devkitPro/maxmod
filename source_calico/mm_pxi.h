#pragma once

#ifdef SYS_CALICO
#include <calico.h>
#define MM_INLINE    MEOW_INLINE
#define MM_CONSTEXPR MEOW_CONSTEXPR
#define PxiChannel_Maxmod ((PxiChannel)22)
#else
#include <nds.h>
#define MM_INLINE    static inline
#define MM_CONSTEXPR static inline
#define armDCacheFlush DC_FlushRange
#endif

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

typedef struct mmPxiArgBank {
	u16 num_songs;
	void* mm_bank;
} mmPxiArgBank;

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
