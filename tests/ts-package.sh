#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-or-later
# The npm package of the reply shapes as a consumer meets it: `npm pack`, the tarball unpacked as a dependency, and a
# TypeScript file that imports the types by the package's exports, compiled strict. The types it declares compile, and a
# value of the wrong type does not.
#
#   OMX_CONTRACT_DIR=<omx-contract checkout, npm ci run> sh tests/ts-package.sh     (make test-ts-package)
set -eu
: "${OMX_CONTRACT_DIR:?set OMX_CONTRACT_DIR to a checkout of Open-Mixer/omx-contract with npm ci run in it}"
tsc=$OMX_CONTRACT_DIR/node_modules/.bin/tsc
[ -x "$tsc" ] || { echo "ts-package: no $tsc: run npm ci in $OMX_CONTRACT_DIR" >&2; exit 1; }
root=$(pwd)
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT
fail() { echo "ts-package: $*" >&2; exit 1; }

(cd "$work" && npm pack --silent "$root" >/dev/null) || fail "npm pack failed"
dep=$work/app/node_modules/@openmixer/plugin-hostd-protocol
mkdir -p "$dep"
tar -xzf "$work"/openmixer-plugin-hostd-protocol-*.tgz -C "$dep" --strip-components=1
n=$(find "$dep/ts/shapes" -name 'reply_*.d.ts' | wc -l)
s=$(find "$dep/shapes" -name 'reply_*.schema.json' | wc -l)
[ "$n" -ge 1 ] && [ "$n" = "$s" ] || fail "the tarball holds $n renders and $s schemas"
find "$root/shapes" -name 'reply_*.schema.json' | wc -l | grep -qx "$s" || fail "the tarball does not hold every schema of shapes/"
for f in client.mjs client.d.mts lines.mjs lines.d.mts vocabulary.mjs vocabulary.d.mts; do
    [ -f "$dep/ts/client/$f" ] || fail "the tarball holds no ts/client/$f"
done

cat > "$work/app/good.ts" <<'T'
import type { InstanceInfoReply } from '@openmixer/plugin-hostd-protocol/shapes/reply_instance_info';
import type { WorkerListReply, WorkerRecord } from '@openmixer/plugin-hostd-protocol/shapes/reply_worker_list';
import type { CurvePointsReply } from '@openmixer/plugin-hostd-protocol/shapes/reply_curve_points';

const info: InstanceInfoReply = { worker: 'w3', pid: 4242, state: 'up', crashes: 0, quarantined: 0 };
const w: WorkerRecord = { worker: 'w3', pid: 4242, format: 'lv2', state: 'up', core: '-', pool: 'on', place: 'own', instances: ['7', '8!'] };
const list: WorkerListReply = { workers: [w] };
const curve: CurvePointsReply = { count: 1, points: [{ x: 20, y: 0 }] };
export const used = [info.pid + list.workers.length + curve.points[0]!.x];
import type { Client, Point } from '@openmixer/plugin-hostd-protocol/client';
import { VERBS } from '@openmixer/plugin-hostd-protocol/vocabulary';
declare const c: Client;
export const pts: Promise<Point[]> = c.curvePoints(3, 0, 128);
export const forms = VERBS.curve_points.forms;
T
sed '0,/pid: 4242/s//pid: "4242"/' "$work/app/good.ts" > "$work/app/bad.ts"
for f in good bad; do
    printf '{ "compilerOptions": { "strict": true, "noEmit": true, "module": "esnext", "moduleResolution": "bundler", "target": "es2022", "types": [] }, "files": ["%s.ts"] }\n' "$f" > "$work/app/tsconfig.$f.json"
done
(cd "$work/app" && "$tsc" -p tsconfig.good.json >"$work/log" 2>&1) || { cat "$work/log"; fail "good.ts does not compile against the package"; }
echo "ok   good.ts imports $n reply types from the packed tarball and compiles strict"
if (cd "$work/app" && "$tsc" -p tsconfig.bad.json >"$work/log" 2>&1); then
    fail "bad.ts, a pid that is a string, compiles"
fi
grep -q "^bad.ts(5,.*TS2322" "$work/log" || { cat "$work/log"; fail "bad.ts is refused, but not at its pid (line 5, TS2322)"; }
echo "ok   bad.ts, a pid that is a string, is refused: $(head -n1 "$work/log" | head -c 100)"
(cd "$work/app" && node --input-type=module -e "
import { connect, HostdError } from '@openmixer/plugin-hostd-protocol/client';
import { VERBS } from '@openmixer/plugin-hostd-protocol/vocabulary';
import { encode } from '@openmixer/plugin-hostd-protocol/lines';
if (typeof connect !== 'function' || typeof HostdError !== 'function' || !VERBS.curve_points) process.exit(1);
if (encode('curve_points', 3, 0, 128) !== 'curve_points 3 0 128') process.exit(2);
") || fail "the client does not import from the packed tarball, or does not write curve_points"
echo "ok   the client imports from the packed tarball and writes curve_points 3 0 128"
