#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-or-later
# tests/client_installed_test.py tells the ways the installed client can fail its consumers (issue 45). Each arm must
# turn it red, for the reason it names:
#   no-codec   python3-plugin-hostd-client installed without python3-plugin-hostd-protocol beside it
#   tree-path  the test run without -I from inside this tree, so clients/python is what imports
#
#   client-installed-sabotage.sh SITELIB     SITELIB as test-client-installed staged it (the arms copy it)
# the table, vocabulary and catalogs from the environment, as test-client-installed sets it
set -eu
site=$1
tree=$(pwd)
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT

# no-codec: the client alone
mkdir "$work/site"
cp -r "$site/plugin_hostd_client" "$work/site/"
if (cd "$work" && python3 -I "$tree/tests/client_installed_test.py" "$work/site" "$tree") >"$work/log" 2>&1; then
    echo "FAIL no-codec: the client without its codec passes"; exit 1
fi
grep -q "does not import: No module named 'plugin_hostd_protocol'" "$work/log" || {
    echo "FAIL no-codec: red for another reason:"; cat "$work/log"; exit 1; }
echo "ok   no-codec: $(grep -m1 'does not import' "$work/log")"

# tree-path: this tree's clients/python on the path instead of the installed packages
if PYTHONPATH="$tree/clients/python" python3 "$tree/tests/client_installed_test.py" "$site" "$tree" >"$work/log" 2>&1; then
    echo "FAIL tree-path: a run with the tree on sys.path passes"; exit 1
fi
grep -q "sys.path holds $tree" "$work/log" || { echo "FAIL tree-path: red for another reason:"; cat "$work/log"; exit 1; }
echo "ok   tree-path: $(grep -m1 'sys.path holds' "$work/log" | sed 's/^ *//')"
