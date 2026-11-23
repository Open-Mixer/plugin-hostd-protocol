#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
#
# domain-renders-test.sh — the C renders of this tree's own domains (make check-domain-renders) read the installed
# contract directory beside contract.d, so a reference of a domain here to an installed owner's domain resolves: the
# domain plugin-hostd-protocol names omx-clap's HOST_REPLY_CODES (its verbs' shared refusals), which omx-clap-contract installs.
#
#   (a) with the contract directories make reads (OMX_CONTRACT_PATH, then the installed one, holding omx-clap's
#       package), check-domain-renders holds pin, estimate and probe byte for byte;
#   (b) with no installed contract directory, so no omx-clap, it is red, refusing the reference by name:
#       contract.ref.unresolved, naming HOST_REPLY_CODES.
#
# Run from the tree's root with make's variables (make test-domain-renders). Exit 0 held; 1 an arm missed.
set -u
failures=0
log=$(mktemp)
empty=$(mktemp -d)
trap 'rm -rf "$log" "$empty"' EXIT

if make --no-print-directory check-domain-renders >"$log" 2>&1; then
  echo "ok   (a) the renders of pin, estimate and probe hold, a reference to omx-clap's HOST_REPLY_CODES resolved"
else
  echo "FAIL (a) check-domain-renders is red with the installed contract directory:"; sed 's/^/     /' "$log"
  failures=$((failures + 1))
fi

if make --no-print-directory check-domain-renders OMX_CONTRACT_PATH= CONTRACTDIR="$empty" >"$log" 2>&1; then
  echo "FAIL (b) check-domain-renders is green with no installed owner of HOST_REPLY_CODES"
  failures=$((failures + 1))
elif grep -q 'contract.ref.unresolved' "$log" && grep -q 'HOST_REPLY_CODES' "$log"; then
  echo "ok   (b) with omx-clap absent, the reference is refused by name: $(grep -m 1 'contract.ref.unresolved' "$log")"
else
  echo "FAIL (b) red, but not on the unresolved HOST_REPLY_CODES:"; sed 's/^/     /' "$log"
  failures=$((failures + 1))
fi

[ "$failures" -eq 0 ] || { echo "domain-renders-test: $failures arms missed"; exit 1; }
echo "domain-renders-test: 2 arms, each as expected"
