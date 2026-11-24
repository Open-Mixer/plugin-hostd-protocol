#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
#
# domain-renders-test.sh — the C renders of this tree's own domains (make check-domain-renders) read this tree's
# contract.d first, then the installed contract directory. The domain plugin-hostd-protocol names HOST_REPLY_CODES (its
# verbs' shared refusals), which this tree's domain host declares (#67): the reference resolves in the tree itself.
#
#   (a) with the contract directories make reads (OMX_CONTRACT_PATH, then the installed one), check-domain-renders
#       holds pin, estimate and probe byte for byte;
#   (b) with no installed contract directory at all it holds them too: nothing it reads comes from another package;
#   (c) in a copy of the tree without its domain host, it is red, refusing the reference by name:
#       contract.ref.unresolved, naming HOST_REPLY_CODES.
#
# Run from the tree's root with make's variables (make test-domain-renders). Exit 0 held; 1 an arm missed.
set -u
failures=0
log=$(mktemp)
empty=$(mktemp -d)
copy=$(mktemp -d)
trap 'rm -rf "$log" "$empty" "$copy"' EXIT

if make --no-print-directory check-domain-renders >"$log" 2>&1; then
  echo "ok   (a) the renders of pin, estimate and probe hold with the installed contract directory"
else
  echo "FAIL (a) check-domain-renders is red with the installed contract directory:"; sed 's/^/     /' "$log"
  failures=$((failures + 1))
fi

if make --no-print-directory check-domain-renders OMX_CONTRACT_PATH= CONTRACTDIR="$empty" >"$log" 2>&1; then
  echo "ok   (b) with no installed contract directory they hold: HOST_REPLY_CODES resolves in this tree's domain host"
else
  echo "FAIL (b) check-domain-renders is red with no installed contract directory:"; sed 's/^/     /' "$log"
  failures=$((failures + 1))
fi

cp -R Makefile VERSION contract.d include packaging "$copy"/
mv "$copy/contract.d/host" "$copy/host.away"
if make --no-print-directory -C "$copy" check-domain-renders OMX_CONTRACT_PATH= CONTRACTDIR="$empty" >"$log" 2>&1; then
  echo "FAIL (c) check-domain-renders is green in a tree without its domain host"
  failures=$((failures + 1))
elif grep -q 'contract.ref.unresolved' "$log" && grep -q 'HOST_REPLY_CODES' "$log"; then
  echo "ok   (c) without the domain host, the reference is refused by name: $(grep -m 1 'contract.ref.unresolved' "$log")"
else
  echo "FAIL (c) red, but not on the unresolved HOST_REPLY_CODES:"; sed 's/^/     /' "$log"
  failures=$((failures + 1))
fi

[ "$failures" -eq 0 ] || { echo "domain-renders-test: $failures arms missed"; exit 1; }
echo "domain-renders-test: 3 arms, each as expected"
