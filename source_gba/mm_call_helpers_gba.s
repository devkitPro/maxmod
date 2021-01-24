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
 * Program
 *
 ******************************************************************************/

/******************************************************************************
 * mpp_call_*
 *
 * Functions to branch to a register
 ******************************************************************************/

.section ".iwram", "ax", %progbits
.thumb
.align 2

.global mpp_call_r7i, mpp_call_r2i, mpp_call_r1i

	.thumb_func
//-----------------------------------------------------------------------------
mpp_call_r7i: bx r7
//-----------------------------------------------------------------------------

	.thumb_func
//-----------------------------------------------------------------------------
mpp_call_r2i: bx r2
//-----------------------------------------------------------------------------

	.thumb_func
//-----------------------------------------------------------------------------
mpp_call_r1i: bx r1
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
.end
//-----------------------------------------------------------------------------

