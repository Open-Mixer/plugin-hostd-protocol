#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-or-later
# A file's mode is the packaging's, never the Makefile's: the rpm spec's %install and debian/rules each make every #! file
# of nodejs-plugin-hostd-protocol's packed tree 0755 (lintian script-not-executable failed 0.21.6's deb build after
# dh_fixperms reset the mode install-node had set). Fails when either packaging lacks the step, or when install-node sets
# a mode again. --sabotage: a copy of each packaging without its step must fail.
set -eu
root=${1:-.}
check() { # <spec> <rules> <makefile>
  grep -q 'PACKAGING-MODE-MARK' "$1" && grep -A2 'PACKAGING-MODE-MARK' "$1" | grep -q 'chmod 0755' || { echo "packaging-modes: FAIL the spec's %install sets no mode for the packed tree's #! files"; return 1; }
  grep -q '^override_dh_fixperms:' "$2" && grep -A4 '^override_dh_fixperms:' "$2" | grep -q 'chmod 0755' || { echo "packaging-modes: FAIL debian/rules restores no mode after dh_fixperms"; return 1; }
  sed -n '/^install-node:/,/^$/p' "$3" | grep -q 'chmod 0755' && { echo "packaging-modes: FAIL install-node sets a mode: the packaging owns it"; return 1; }
  return 0
}
check "$root/packaging/plugin-hostd-protocol.spec" "$root/debian/rules" "$root/Makefile"
t=$(mktemp -d); trap 'rm -rf "$t"' EXIT
grep -v 'chmod 0755' "$root/packaging/plugin-hostd-protocol.spec" > "$t/spec"
grep -v 'chmod 0755' "$root/debian/rules" > "$t/rules"
if check "$t/spec" "$root/debian/rules" "$root/Makefile" >/dev/null; then echo "packaging-modes: FAIL sabotage: a spec without the step passed"; exit 1; fi
if check "$root/packaging/plugin-hostd-protocol.spec" "$t/rules" "$root/Makefile" >/dev/null; then echo "packaging-modes: FAIL sabotage: rules without the step passed"; exit 1; fi
echo "ok   packaging-modes: the spec and debian/rules own the #! files' mode; each sabotage red"
