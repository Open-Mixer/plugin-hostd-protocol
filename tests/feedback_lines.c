/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
 *
 * tests/feedback_lines.c: one line of each shape plugin-hostd writes on its feedback port, printed through the formats
 * it prints them with (PHD_EVENT_*_FMT of protocol.h), so tests/feedback_args.py decodes what the daemon writes, not a
 * copy of it. abi_clobber and xruns are omx-clap-core's worker lines (HOST_EVENT_*_FMT of its src/host_verbs.h, not
 * installed), relayed as the worker wrote them: their formats are the two written here. */
#include <stdio.h>

#include "plugin-hostd/protocol.h"

int main(void)
{
    printf(PHD_EVENT_WORKER_DIED_FMT "\n", 3, 4242, "signal:11", "3,4");
    printf(PHD_EVENT_WORKER_DIED_FMT "\n", 1, 77, "exit:1", PHD_WORD_NONE);
    printf(PHD_EVENT_VERB_DROPPED_FMT "\n", 3, "param_set 3 gain 0.5");
    printf(PHD_EVENT_VERB_DROPPED_SUSPECT_FMT "\n", 3, 12, "preset_load 3 warm");
    printf(PHD_EVENT_WORKER_BACKOFF_FMT "\n", 3, 250);
    printf(PHD_EVENT_WORKER_RESPAWNED_FMT "\n", 3, 4243, 5, 18);
    printf(PHD_EVENT_INSTANCE_RESTORED_FMT "\n", 4, 3);
    printf(PHD_EVENT_REPLAY_REFUSED_FMT "\n", 4, -508);
    printf(PHD_EVENT_INSTANCE_QUARANTINED_FMT "\n", 4, 3);
    printf(PHD_EVENT_SUPERVISOR_GAVE_UP_FMT "\n", 3, 5, 10000);
    printf(PHD_EVENT_PIN_CHANGED_FMT "\n", "clap:/usr/lib/clap/Surge%20XT.clap#surge-xt", "files");
    printf(PHD_EVENT_PIN_CHANGED_FMT "\n", "clap:/usr/lib/clap/x.clap#x", "layout");
    printf(PHD_EVENT_LV2_ADAPTER_REFUSED_FMT "\n", "urn:example:delay", "worker-missing", "omx-clap-host");
    printf(PHD_EVENT_CHAIN_FORMED_FMT "\n", "2", 5, "7,8,9");
    printf(PHD_EVENT_CHAIN_FORMED_FMT "\n", "4" PHD_PLACE_PLAN_SEPARATOR "1", 6, PHD_WORD_NONE);
    printf(PHD_EVENT_CHAIN_REFUSED_FMT "\n", "2", -501);
    printf(PHD_EVENT_CHAIN_STALLED_FMT "\n", "2", "8", "add", -903);
    printf(PHD_EVENT_CHAIN_STALLED_FMT "\n", "2", PHD_WORD_NONE, "spawn", -903);
    printf(PHD_EVENT_PLAN_REASON_FMT "\n", 4, "7:budget,8:sidechain");
    printf(PHD_EVENT_REMOTE_PAGES_CHANGED_FMT "\n", 7);
    printf(PHD_EVENT_REMOTE_PAGE_SUGGEST_FMT "\n", 7, 2u);
    printf(PHD_EVENT_PARAM_GESTURE_FMT "\n", 7, "gain", PHD_GESTURE_BEGIN);
    printf(PHD_EVENT_PARAM_GESTURE_FMT "\n", 7, "gain", PHD_GESTURE_END);
    printf(PHD_EVENT_PARAM_SET_FMT "\n", 7, "gain", "0.5");
    printf(PHD_EVENT_XRUNS_FMT "\n", 12ULL);
    printf(PHD_EVENT_ABI_CLOBBER " %d %#x %s %s %llu\n", 7, 0x28u, "rbx,r12", "process", 1024ULL);
    return 0;
}
