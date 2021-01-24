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

#ifndef MP_MAS_STRUCTS_H
#define MP_MAS_STRUCTS_H

#include <assert.h>

#include "mm_types.h"

typedef struct {
    mm_byte     tick;
    mm_byte     row;
    mm_byte     position;
    mm_byte     nrows;
    mm_byte     gv;
    mm_byte     speed;
    mm_byte     isplaying;
    mm_byte     bpm;
    mm_word     insttable;
    mm_word     samptable;
    mm_word     patttable;
    mm_word     songadr;
    mm_byte     flags;
    mm_byte     oldeffects;
    mm_byte     pattjump;
    mm_byte     pattjump_row;
    mm_byte     fpattdelay;
    mm_byte     pattdelay;

    mm_byte     ploop_row;
    mm_byte     ploop_times;
    mm_word     ploop_adr;
    mm_word     pattread;
    mm_byte     ploop_jump;
    mm_byte     valid;

    mm_hword    tickrate;  // 1.15 fixed point OR sample count
    union {
        mm_hword    sampcount; // sample timing
        mm_hword    tickfrac;  // vsync  timing 0.16 fixed point
    };

    mm_byte     mode;
    mm_byte     reserved2;
    mm_word     mch_update;
    mm_hword    volume;
    mm_hword    reserved3;
} mpl_layer_information;

static_assert(sizeof(mpl_layer_information) == 56);

typedef struct {
    mm_word     period;     // internal period
    mm_hword    fade;       //    }
    mm_hword    envc_vol;   //    }
    mm_hword    envc_pan;   //    } COMBINED
    mm_hword    envc_pic;   //    } during volume reset
    mm_hword    avib_dep;   //    } AND NEWNOTE, CHECK NEWNOTE ON CHANGES
    mm_hword    avib_pos;   //    }
    mm_byte     fvol;       // } COMBINED for SPEED
    mm_byte     type;       // }
    mm_byte     inst;
    mm_byte     panning;
    mm_byte     volume;
    mm_byte     sample;
    mm_byte     parent;     // } COMBINED
    mm_byte     flags;      // }
    mm_byte     envn_vol;
    mm_byte     envn_pan;
    mm_byte     envn_pic;
    mm_byte     sfx;        // can store this anywhere
} mm_active_channel;

static_assert(sizeof(mm_active_channel) == 28);

// Active Channel Flags
// --------------------

#define MCAF_KEYON      1   // key is on... LOCKED
#define MCAF_FADE       2   // note-fade is activated
#define MCAF_START      4   // [re]start sample
#define MCAF_UPDATED    8   // already updated by pchannel routine
#define MCAF_ENVEND     16  // end of envelope
#define MCAF_VOLENV     32  // volume envelope enabled
#define MCAF_SUB        64  // sublayer.................locked..
#define MCAF_EFFECT     128 // subsublayer.............................LOCKED (mpEffect)

// Active Channel Types
// --------------------

#define ACHN_DISABLED   0 // LOCKED (multiple routines)
#define ACHN_RESERVED   1 // (can't be used [alloc channel])
#define ACHN_BACKGROUND 2 // LOCKED (alloc channel)
#define ACHN_FOREGROUND 3
#define ACHN_CUSTOM     4

#endif // MP_MAS_STRUCTS_H
