#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-or-later
# The client's command() goes through the codec: in a scratch copy, command() that quotes by hand (a join of its words)
# instead of protocol().encode must turn tests/client_codec_test.py red.
#
#   client-codec-sabotage.sh
set -eu
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT
cp -r clients tests "$work/"
file="$work/clients/python/plugin_hostd_client/__init__.py"
cp "$file" "$work/before"
sed -i 's/return self.send(protocol().encode(verb, args, tag), timeout)/return self.send(" ".join([verb] + [str(a) for a in args]), timeout)/' "$file"
if cmp -s "$work/before" "$file"; then echo "FAIL the sabotage changed nothing"; exit 1; fi
if CLIENT_CODEC_TREE="$work" python3 "$work/tests/client_codec_test.py" >"$work/log" 2>&1; then
    echo "FAIL a hand-quoting command() passes client_codec_test"; exit 1
fi
echo "ok   a hand-quoting command() fails client_codec_test ($(grep -m1 '^FAIL' "$work/log" | head -c 100))"
