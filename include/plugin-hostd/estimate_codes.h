// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
#ifndef OMX_ESTIMATE_CODES_H
#define OMX_ESTIMATE_CODES_H
/*
 * GENERATED: DO NOT EDIT BY HAND.
 * Produced by `omx-contract render --target c --package estimate` from the domain estimate: the codes of its items marked
 * "x-c-codes", one OMX_<ITEM>_<WORD> per word and an OMX_<ITEM>_COUNT per item, and an OMX_<ITEM>(X) X-macro where the
 * item is marked "x-c-xmacro" as well. Nothing else of the domain is rendered: its words are read at run time.
 */

/* ESTIMATE_VERDICTS */
#define OMX_ESTIMATE_VERDICTS_RECOMMEND 0u
#define OMX_ESTIMATE_VERDICTS_WARN 1u
#define OMX_ESTIMATE_VERDICTS_NOT_MEASURED 2u
#define OMX_ESTIMATE_VERDICTS_REFUSE 3u
#define OMX_ESTIMATE_VERDICTS_COUNT 4u

/* ESTIMATE_REASONS */
#define OMX_ESTIMATE_REASONS_UNKNOWN_PLUGIN 0u
#define OMX_ESTIMATE_REASONS_NO_RECORD 1u
#define OMX_ESTIMATE_REASONS_NO_CELL 2u
#define OMX_ESTIMATE_REASONS_CPU_OVER_BUDGET 3u
#define OMX_ESTIMATE_REASONS_RAM_OVER_BUDGET 4u
#define OMX_ESTIMATE_REASONS_RAM_UNKNOWN 5u
#define OMX_ESTIMATE_REASONS_WITHIN 6u
#define OMX_ESTIMATE_REASONS_COUNT 7u

#endif /* OMX_ESTIMATE_CODES_H */
