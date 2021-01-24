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

#include <stdlib.h>

#include "maxmod.h"

#include "mp_mas_structs.h"

#define mixlen 1056 // 16 KHz

static uint32_t mixbuffer[mixlen / sizeof(uint32_t)];

#define MM_SIZEOF_MODCH     40
#define MM_SIZEOF_MIXCH     24

void mmInitDefault(mm_addr soundbank, mm_word number_of_channels)
{
    // Allocate buffer
    size_t size_of_channel = MM_SIZEOF_MODCH + sizeof(mm_active_channel) + MM_SIZEOF_MIXCH;
    size_t size_of_buffer = mixlen + number_of_channels * size_of_channel;
    mm_addr buffer = malloc(size_of_buffer);

    // TODO: The original library didn't check the return value of malloc.

    // Split up buffer
    mm_addr wave_memory, module_channels, active_channels, mixing_channels;

    wave_memory = buffer;
    module_channels = wave_memory + mixlen;
    active_channels = module_channels + number_of_channels * MM_SIZEOF_MODCH;
    mixing_channels = active_channels + number_of_channels * sizeof(mm_active_channel);

    mm_gba_system setup =
    {
        .mixing_mode = 3,
        .mod_channel_count = number_of_channels,
        .mix_channel_count = number_of_channels,
        .module_channels = module_channels,
        .active_channels = active_channels,
        .mixing_channels = mixing_channels,
        .mixing_memory = (mm_addr)&mixbuffer[0],
        .wave_memory = wave_memory,
        .soundbank = soundbank
    };

    mmInit(&setup);
}
