/****************************************************************************
 *                                                          __              *
 *                ____ ___  ____ __  ______ ___  ____  ____/ /              *
 *               / __ `__ \/ __ `/ |/ / __ `__ \/ __ \/ __  /               *
 *              / / / / / / /_/ />  </ / / / / / /_/ / /_/ /                *
 *             /_/ /_/ /_/\__,_/_/|_/_/ /_/ /_/\____/\__,_/                 *
 *                                                                          *
 *         Copyright (c) 2008, Mukunda Johnson (mukunda@maxmod.org)         *
 *                                                                          *
 * Permission to use, copy, modify, and/or distribute this software for any *
 * purpose with or without fee is hereby granted, provided that the above   *
 * copyright notice and this permission notice appear in all copies.        *
 *                                                                          *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES *
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF         *
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR  *
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES   *
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN    *
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF  *
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.           *
 ****************************************************************************/

/******************************************************************************
 *
 * Definitions
 *
 ******************************************************************************/

#include "mp_defs.inc"
#include "mp_mas.inc"
#include "mp_mas_structs.inc"
#include "mp_macros.inc"
#include "mp_mixer_ds.inc"

/******************************************************************************
 *
 * Memory
 *
 ******************************************************************************/

	.BSS
	.ALIGN 2

/******************************************************************************
 * mmModuleCount
 *
 * Number of modules in soundbank
 ******************************************************************************/
					.global mmModuleCount
mmModuleCount:	.space 4

/******************************************************************************
 * mmModuleBank
 *
 * Address of module bank
 ******************************************************************************/
					.global mmModuleBank
mmModuleBank:	.space 4

/******************************************************************************
 * mmSampleBank
 *
 * Address of sample bank
 ******************************************************************************/
					.global mmSampleBank
mmSampleBank:	.space 4

/******************************************************************************
 * mm_rds_pchannels, mm_rds_achannels
 *
 * Memory for module/active channels for NDS system
 ******************************************************************************/
mm_rds_pchannels:	.space MCH_SIZE*32
mm_rds_achannels:	.space MCA_SIZE*32

/******************************************************************************
 * mmInitialized
 *
 * Variable that will be 'true' if we are ready for playback
 ******************************************************************************/
					.global mmInitialized
mmInitialized:		.space 1

/******************************************************************************
 *
 * Program
 *
 ******************************************************************************/

//-----------------------------------------------------------------------------
	.TEXT
	.THUMB
	.ALIGN 2
//-----------------------------------------------------------------------------

/******************************************************************************
 * mmLockChannels( mask )
 *
 * Lock audio channels to prevent the sequencer from using them.
 ******************************************************************************/
						.global mmLockChannels
						.thumb_func
mmLockChannels:
	push	{r4, r5, lr}
	ldr	r1,=mm_ch_mask			// clear bits
	ldr	r2, [r1]			//
	bic	r2, r0				//
	str	r2, [r1]			//

	mov	r4, r0
	mov	r5, #0

2:	lsr	r4, #1
	bcc	1f
	mov	r0, r5
	bl	StopActiveChannel
1:	add	r5, #1
	cmp	r4, #0
	bne	2b

	pop	{r4,r5}
	pop	{r3}
	bx	r3

/******************************************************************************
 * StopActiveChannel( index )
 *
 * Stop active channel
 ******************************************************************************/
						.thumb_func
StopActiveChannel:
	push	{r4}

	GET_MIXCH r1				// stop mixing channel
	mov	r2, #MIXER_CHN_SIZE		//
	mul	r2, r0				//
	add	r1, r2				//
						//
						//
	mov	r2, #0				//
	str	r2, [r1, #MIXER_CHN_SAMP]	//
	strh	r2, [r1, #MIXER_CHN_CVOL]	//
	strh	r2, [r1, #MIXER_CHN_VOL]	//

	ldr	r1,=0x4000400			// stop hardware channel
	lsl	r2, r0, #4			//
	mov	r3, #0				//
	str	r3, [r1, r2]			//

	ldr	r1,=mm_achannels		// disable achn
	ldr	r1, [r1]			//
	mov	r2, #MCA_SIZE			//
	mul	r2, r0				//
	add	r1, r2				//
	mov	r2, #0				//
	ldrb	r4, [r1, #MCA_FLAGS]		//
	strb	r2, [r1, #MCA_FLAGS]		//
	strb	r2, [r1, #MCA_TYPE]		//

	lsr	r1, r4, #8
	bcs	.iseffect

	lsr	r4, #7
	bcs	.issub

	ldr	r1,=mm_pchannels		// stop hooked pchannel
	ldr	r1, [r1]			//
	ldr	r2,=mm_num_mch			//
	ldr	r2, [r2]			//
						//
2:	ldrb	r3, [r1, #MCH_ALLOC]		//
	cmp	r3, r0				//
	bne	1f				//
	mov	r3, #255			//
	strb	r3, [r1, #MCH_ALLOC]		//
	b	.iseffect			//
1:	sub	r2, #1				//
	bne	2b				//

	b	.iseffect			//

.issub:
	// stop sub pchannel
	ldr	r1,=mm_schannels
	mov	r2, #4

2:	ldrb	r3, [r1, #MCH_ALLOC]		//
	cmp	r3, r0				//
	bne	1f				//
	mov	r3, #255			//
	strb	r3, [r1, #MCH_ALLOC]		//
	b	.iseffect			//
1:	sub	r2, #1				//
	bne	2b				//

.iseffect:

	// hope it works out for effects...

	pop	{r4}
	bx	lr


/******************************************************************************
 * mmUnlockChannels( mask )
 *
 * Unlock audio channels so they can be used by the sequencer.
 ******************************************************************************/
						.global mmUnlockChannels
						.thumb_func
mmUnlockChannels:

	ldr	r1,=mm_mixing_mode		// can NOT unlock channels in mode b
	ldrb	r1, [r1]			//
	cmp	r1, #1				//
	beq	1f				//

	ldr	r1,=mm_ch_mask
	ldr	r2, [r1]
	orr	r2, r0
	str	r2, [r1]
1:	bx	lr


/******************************************************************************
 * mmSuspendIRQ_t
 *
 * Function to disable interrupts via the status register
 ******************************************************************************/
						.global mmSuspendIRQ_t
						.thumb_func
mmSuspendIRQ_t:
	ldr	r0,=1f
	bx	r0

.arm
.align 2
1:	mrs	r0, cpsr
	and	r1, r0, #0x80
	orr	r0, #0x80
	msr	cpsr, r0
	str	r1, previous_irq_state
	bx	lr
.thumb

/******************************************************************************
 * mmRestoreIRQ_t
 *
 * Function to enable interrupts via the status register
 ******************************************************************************/
						.global	mmRestoreIRQ_t
						.thumb_func
mmRestoreIRQ_t:
	ldr	r0,=1f
	bx	r0

.arm
.align 2
1:	mrs	r0, cpsr
	ldr	r1, previous_irq_state
	bic	r0, #0x80
	orr	r0, r1
	msr	cpsr, r0
	bx	lr

.thumb

previous_irq_state:
	.space	4

	.thumb_func
/******************************************************************************
 * mmIsInitialized()
 *
 * Returns true if the system is ready for playback
 ******************************************************************************/
						.global mmIsInitialized
						.thumb_func
mmIsInitialized:
	ldr	r0,=mmInitialized
	ldrb	r0, [r0]
	bx	lr

/******************************************************************************
 * mmInit7()
 *
 * Initialize system
 ******************************************************************************/
						.global mmInit7
						.thumb_func
mmInit7:
	push	{lr}
	mov	r0, #0x08
	ldr	r1,=mmFrame
	bl	irqSet

	mov	r0, #0x08
	bl	irqEnable

	ldr	r0,=0x400			// set volumes
	bl	mmSetModuleVolume		//
	ldr	r0,=0x400			//
	bl	mmSetJingleVolume		//
	ldr	r0,=0x400			//
	bl	mmSetEffectsVolume		//

	ldr	r0,=mmInitialized		// set initialized flag
	mov	r1, #42				//
	strb	r1, [r0]			//

	ldr	r0,=0xFFFF			// select all hardware channels
	bl	mmUnlockChannels		//

	ldr	r0,=mm_achannels		// setup channel addresses
	ldr	r1,=mm_rds_achannels		//
	str	r1, [r0]			//
	ldr	r1,=mm_rds_pchannels		//
	str	r1, [r0,#4]			//
	mov	r1, #32				// 32 channels
	str	r1, [r0,#8]			//
	str	r1, [r0,#12]			//

	ldr	r0,=0x400
	bl	mmSetModuleTempo

	ldr	r0,=0x400
	bl	mmSetModulePitch

	bl	mmResetEffects

	bl	mmMixerInit			// setup mixer

	ldr	r0,=mmEventForwarder		// forward events
	bl	mmSetEventHandler

	ldr	r0,=mmInitialized		// set initialized flag
	mov	r1, #42				//
	strb	r1, [r0]			//

.exit_r3:
	pop	{r3}
	bx	r3

/******************************************************************************
 * mmInstall( channel )
 *
 * Install ARM7 system
 ******************************************************************************/
						.global mmInstall
						.thumb_func
mmInstall:
	push	{lr}

	ldr	r1,=mmInitialized		// not initialized until we get soundbank data
	mov	r2, #0				//
	strb	r2, [r1]			//

	bl	mmSetupComms			// setup communication

	b	.exit_r3

/******************************************************************************
 * mmEventForwarder( msg, param )
 *
 * Forward event to arm9
 ******************************************************************************/
						.thumb_func
mmEventForwarder:

	push	{lr}
	lsl	r1, #8
	orr	r0, r1
	mov	r1, #1
	lsl	r1, #20
	orr	r0, r1
	bl	mmARM9msg
	pop	{pc}

/******************************************************************************
 * mmGetSoundBank( n_songs, bank )
 *
 * Load sound bank address
 ******************************************************************************/
						.global mmGetSoundBank
						.thumb_func
mmGetSoundBank:
	ldr	r2,=mmModuleCount		// save data
	stmia	r2!, {r0,r1}			//

	lsl	r0, #2				// also sample bank address
	add	r1, r0				//
	stmia	r2!, {r1}			//

//------------------------------------------------
// initialize system
//------------------------------------------------

	b	mmInit7

/******************************************************************************
 * mmFrame()
 *
 * Routine function
 ******************************************************************************/
						.global mmFrame
						.thumb_func
mmFrame:

	push	{lr}

	ldr	r0,=mmInitialized	// catch not-initialized
	ldrb	r0, [r0]		//
	cmp	r0, #42			//
	bne	1f			//

	bl	mmMixerPre		// <-- critical timing

	ldr	r0,=0x4000208		// enable irq
	mov	r1, #1			//
	strh	r1, [r0]		//

	bl	mmUpdateEffects		// update sound effects
	bl	mmPulse			// update module playback
	bl	mmMixerMix		// update audio


	bl	mmSendUpdateToARM9

1:	bl	mmProcessComms		// process communications

	ret1

.pool

//-----------------------------------------------------------------------------
.end
//-----------------------------------------------------------------------------

