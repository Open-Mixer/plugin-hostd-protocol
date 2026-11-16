#!/bin/bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas
#
# tests/runner_test DEMONSTRATES a refusal: worker_list answered "not found" by a host it is required of. A judge reads
# a line starting with FAIL as red, so the demonstration must ASSERT the refusal, not print it: an "ok" line naming the
# refusal, no FAIL line, exit 0. And a refusal the test did not expect still prints FAIL at column 0 and fails: the
# sabotage (RUNNER_TEST_SABOTAGE=unexpected-refusal: plugin-hostd's run expects no failure) must go red.
#
#   tests/runner-quiet.sh [tests/runner_test]
set -uo pipefail
B=${1:-./tests/runner_test}
bad=0
out=$("$B" 2>&1); rc=$?
[ "$rc" = 0 ] || { echo "runner-quiet: runner_test exits $rc"; bad=1; }
if grep -q '^FAIL' <<<"$out"; then
	echo "runner-quiet: runner_test prints a FAIL line while it passes:"
	grep '^FAIL' <<<"$out" | sed 's/^/  > /'
	bad=1
fi
for want in "^ok   host plugin-hostd: refused 'worker_list': not found, and worker_list is required of plugin-hostd" \
	"^ok   host (none): refused 'worker_list': not found, and worker_list is required of a host"; do
	grep -q -- "$want" <<<"$out" || { echo "runner-quiet: no line ${want#^} in runner_test's output"; bad=1; }
done
out=$(RUNNER_TEST_SABOTAGE=unexpected-refusal "$B" 2>&1); rc=$?
if [ "$rc" = 0 ] || ! grep -q "^FAIL 'worker_list': not found, and worker_list is required of plugin-hostd" <<<"$out"; then
	echo "runner-quiet: sabotage unexpected-refusal stayed green: an unexpected refusal must print FAIL and fail (exit $rc)"
	sed 's/^/  > /' <<<"$out"
	bad=1
fi
[ "$bad" = 0 ] && echo "runner-quiet: the refusal is asserted (ok lines, no FAIL), and an unexpected one fails"
exit "$bad"
