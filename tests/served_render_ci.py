#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
"""tests/served_render_ci.py [<ci.yml>]: a served contract package that changes what this repository renders turns
main red when it is served, not at the next pull request (#27). ci.yml runs on a schedule and by hand; on a scheduled
run the declared-protocol job (the one that renders against the channel) runs and every other job is skipped, so the
nightly run is the render check alone; and when that job fails, a step names the contract packages and the versions
the channel served it, beside check-rendered's difference (#26). A planted fault (no schedule, the render job skipped
on it, another job run on it, no step naming the versions) must be seen."""
import copy
import os
import sys

import yaml

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
RENDER_JOB = "protocol"
SKIP = "github.event_name != 'schedule'"


def triggers(doc):
    # YAML 1.1 reads the key `on` as true
    return doc.get("on", doc.get(True)) or {}


def problems(doc):
    out = []
    on = triggers(doc)
    if not isinstance(on, dict) or not any(isinstance(s, dict) and s.get("cron") for s in on.get("schedule") or []):
        out.append("ci.yml has no schedule: a served render change waits for the next pull request")
    if not isinstance(on, dict) or "workflow_dispatch" not in on:
        out.append("ci.yml cannot be run by hand")
    jobs = doc.get("jobs") or {}
    if RENDER_JOB not in jobs:
        out.append("ci.yml has no %s job" % RENDER_JOB)
        return out
    if SKIP in str(jobs[RENDER_JOB].get("if", "")):
        out.append("the %s job, the render check, is skipped on the schedule" % RENDER_JOB)
    for name, job in jobs.items():
        if name != RENDER_JOB and SKIP not in str(job.get("if", "")):
            out.append("the %s job runs on the schedule: the nightly run is the render check alone" % name)
    named = [s for s in jobs[RENDER_JOB].get("steps") or []
             if "failure()" in str(s.get("if", "")) and "/tmp/from-repo" in str(s.get("run", ""))]
    if not named:
        out.append("no step names the served contract packages and their versions when the render check fails")
    return out


def main(argv):
    path = argv[1] if len(argv) > 1 else os.path.join(ROOT, ".github", "workflows", "ci.yml")
    with open(path) as f:
        doc = yaml.safe_load(f)
    found = problems(doc)
    for p in found:
        print("FAIL " + p)
    if found:
        return 1

    def planted(edit):
        d = copy.deepcopy(doc)
        edit(d)
        return problems(d)

    def drop_schedule(d):
        triggers(d).pop("schedule", None)

    def skip_render(d):
        d["jobs"][RENDER_JOB]["if"] = SKIP

    def run_other(d):
        other = next(n for n in d["jobs"] if n != RENDER_JOB)
        d["jobs"][other].pop("if", None)

    def drop_naming(d):
        d["jobs"][RENDER_JOB]["steps"] = [s for s in d["jobs"][RENDER_JOB]["steps"] if "failure()" not in str(s.get("if", ""))]

    for name, edit in (("no schedule", drop_schedule), ("the render job skipped", skip_render),
                       ("another job on the schedule", run_other), ("no step naming the versions", drop_naming)):
        if not planted(edit):
            print("FAIL the check does not see %s" % name)
            return 1
    print("served render ci: ok (scheduled, the render job alone, the versions named on failure; 4 planted faults seen)")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
