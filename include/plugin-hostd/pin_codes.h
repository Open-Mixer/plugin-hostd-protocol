// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
#ifndef OMX_PIN_CODES_H
#define OMX_PIN_CODES_H
/*
 * GENERATED: DO NOT EDIT BY HAND.
 * Produced by `omx-contract render --target c --package pin` from the domain pin: the codes of its items marked
 * "x-c-codes", one OMX_<ITEM>_<WORD> per word and an OMX_<ITEM>_COUNT per item, and an OMX_<ITEM>(X) X-macro where the
 * item is marked "x-c-xmacro" as well. Nothing else of the domain is rendered: its words are read at run time.
 */

/* PIN_VERDICTS */
#define OMX_PIN_VERDICTS_IN_PROCESS 0u
#define OMX_PIN_VERDICTS_ISOLATED 1u
#define OMX_PIN_VERDICTS_REFUSED 2u
#define OMX_PIN_VERDICTS_COUNT 3u

/* PIN_POOLS */
#define OMX_PIN_POOLS_CORES 0u
#define OMX_PIN_POOLS_OFF 1u
#define OMX_PIN_POOLS_COUNT 2u

/* PIN_KEYS */
#define OMX_PIN_KEYS_VERDICT 0u
#define OMX_PIN_KEYS_POOL 1u
#define OMX_PIN_KEYS_COUNT 2u

/* PIN_CHANGES */
#define OMX_PIN_CHANGES_FILES 0u
#define OMX_PIN_CHANGES_LAYOUT 1u
#define OMX_PIN_CHANGES_COUNT 2u

#endif /* OMX_PIN_CODES_H */
