/****************************************************************************
 *                                                          __              *
 *                ____ ___  ____ __  ______ ___  ____  ____/ /              *
 *               / __ `__ \/ __ `/ |/ / __ `__ \/ __ \/ __  /               *
 *              / / / / / / /_/ />  </ / / / / / /_/ / /_/ /                *
 *             /_/ /_/ /_/\__,_/_/|_/_/ /_/ /_/\____/\__,_/                 *
 *                                                                          *
 *                         Stereo Deinterleaving                            *
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

#include <calico/asm.inc>

#define rNum   r0 // number of samples
#define rIn    r1 // input
#define rOut   r2 // output
#define rRight r3 // right offset

/********************************************************************
 * 8-bit stereo
 *
 * De-interleaved copy
 ********************************************************************/

FUNC_START32 _mmDeinterleave8
	push {r4-r8,lr}
	ldr  r12, =0xff00ff00

1:
	ldmia rIn!, {r4-r5}    // r5-r4 = R4L4R3L3 R2L2R1L1

	and r6, r12, r4, lsl#8 // r6 = L2--L1--
	orr r6, r6, lsl#8      // r6 = L2L1xx-- 2 samples deinterleaved
	and r7, r12, r5, lsl#8 // r7 = L4--L3--
	orr r7, r7, lsl#8      // r7 = L4L3xx-- 2 more samples
	bic r7, #0xff00        // r7 = L4L3----
	orr r8, r7, r6, lsr#16 // r7 = L4L3L2L1 4 samples

	and r6, r12, r4        // r6 = R2--R1--
	orr r6, r6, lsl#8      // r6 = R2R1xx--
	and r7, r12, r5        // r7 = R4--R3--
	orr r7, r7, lsl#8      // r7 = R4R3xx--
	bic r7, #0xff00        // r7 = R4R3----
	orr r7, r7, r6, lsr#16 // r7 = R4R3R2R1

	str r7, [rOut, rRight] // write to right output
	str r8, [rOut], #4     // write to left output & increment

	subs rNum, #4
	bne  1b

	pop {r4-r8,pc}
FUNC_END

/********************************************************************
 * 16-bit stereo
 *
 * De-interleaved copy
 ********************************************************************/

FUNC_START32 _mmDeinterleave16
	push {r4-r8,lr}

1:
	ldmia rIn!, {r4-r7}    // r7-r4 = R4L4 R3L3 R2L2 R1L1

	mov r8,  r4, lsr#16    // r8  = --R1
	mov r12, r5, lsr#16    // r12 = --R2
	orr r8, r12, lsl#16    // r12 = R2R1

	str r8, [rOut, rRight] // write to right output

	mov r8,  r4, lsl#16    // r8  = L1--
	mov r12, r5, lsl#16    // r12 = L2--
	orr r12, r8, lsr#16    // r12 = L2L1

	str r12, [rOut], #4    // write to left output & increment

	mov r8,  r6, lsr#16    // r8  = --R3
	mov r12, r7, lsr#16    // r12 = --R4
	orr r8, r12, lsl#16    // r12 = R4R3

	str r8, [rOut, rRight] // write to right output

	mov r8,  r6, lsl#16    // r8 = L3--
	mov r12, r7, lsl#16    // r12 = L4--
	orr r12, r8, lsr#16    // r12 = L4L3

	str r12, [rOut], #4    // write to left output & increment

	subs rNum, #4
	bne  1b

	pop {r4-r8,pc}
FUNC_END
