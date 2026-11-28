#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-or-later
# make install-node sets every #! file of nodejs-plugin-hostd-protocol's packed tree to 0755, and the packaging RESTATES
# it, because packaging tools reset modes (0.21.6's deb build failed lintian script-not-executable after dh_fixperms reset
# the mode install-node had set): the rpm spec's %install and debian/rules after dh_fixperms. Fails when any of the three
# lacks the step; its sabotages, each of the three without its step, must fail.
set -eu
root=${1:-.}
check() { # <spec> <rules> <makefile>
  grep -q 'PACKAGING-MODE-MARK' "$1" && grep -A2 'PACKAGING-MODE-MARK' "$1" | grep -q 'chmod 0755' || { echo "packaging-modes: FAIL the spec's %install sets no mode for the packed tree's #! files"; return 1; }
  grep -q '^override_dh_fixperms:' "$2" && grep -A4 '^override_dh_fixperms:' "$2" | grep -q 'chmod 0755' || { echo "packaging-modes: FAIL debian/rules restores no mode after dh_fixperms"; return 1; }
  sed -n '/^install-node:/,/^$/p' "$3" | grep 'INSTALL-MODE-MARK' | grep -q 'chmod 0755' || { echo "packaging-modes: FAIL install-node sets no mode for the #! files (make install sets it, the packaging restates it)"; return 1; }
  return 0
}
check "$root/packaging/plugin-hostd-protocol.spec" "$root/debian/rules" "$root/Makefile"
t=$(mktemp -d); trap 'rm -rf "$t"' EXIT
grep -v 'chmod 0755' "$root/packaging/plugin-hostd-protocol.spec" > "$t/spec"
grep -v 'chmod 0755' "$root/debian/rules" > "$t/rules"
if check "$t/spec" "$root/debian/rules" "$root/Makefile" >/dev/null; then echo "packaging-modes: FAIL sabotage: a spec without the step passed"; exit 1; fi
if check "$root/packaging/plugin-hostd-protocol.spec" "$t/rules" "$root/Makefile" >/dev/null; then echo "packaging-modes: FAIL sabotage: rules without the step passed"; exit 1; fi
grep -v 'INSTALL-MODE-MARK' "$root/Makefile" > "$t/mk"
if check "$root/packaging/plugin-hostd-protocol.spec" "$root/debian/rules" "$t/mk" >/dev/null; then echo "packaging-modes: FAIL sabotage: an install-node without the step passed"; exit 1; fi
echo "ok   packaging-modes: install-node sets the #! files' mode, the spec and debian/rules restate it; each sabotage red"
