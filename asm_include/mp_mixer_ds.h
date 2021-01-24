/****************************************************************************
 *                                                          __              *
 *                ____ ___  ____ __  ______ ___  ____  ____/ /              *
 *               / __ `__ \/ __ `/ |/ / __ `__ \/ __ \/ __  /               *
 *              / / / / / / /_/ />  </ / / / / / /_/ / /_/ /                *
 *             /_/ /_/ /_/\__,_/_/|_/_/ /_/ /_/\____/\__,_/                 *
 *                                                                          *
 *      Copyright (c) 2021, Antonio Niño Díaz (antonio_nd@outlook.com)      *
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

#ifndef MP_MIXER_NDS_H
#define MP_MIXER_NDS_H

#include <assert.h>

#include "mm_types.h"

typedef struct {
    mm_word     samp_cnt;   // 0:23  mainram address
                            // 24:31 keyon:1bit,panning:7bits
    mm_hword    freq;       // unsigned 3.10, top 3 cleared
    mm_hword    vol;        // 0-65535
    mm_word     read;       // unsigned 22.10
    mm_hword    cvol;       // current volume
    mm_hword    size;       // current panning
} mm_mixer_channel;

static_assert(sizeof(mm_mixer_channel) == 16);

// scale = 65536*1024*2 / mixrate
#define MIXER_SCALE         4096 //6151

#define MIXER_CF_START      128
#define MIXER_CF_SOFT       2
#define MIXER_CF_SURROUND   4
#define MIXER_CF_FILTER     8
#define MIXER_CF_REVERSE    16

#endif // MP_MIXER_NDS_H
