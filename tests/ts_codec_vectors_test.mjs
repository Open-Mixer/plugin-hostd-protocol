// SPDX-License-Identifier: GPL-3.0-or-later
// The TypeScript client's lines on the codec's shared vectors, clients/fixtures/codec.json: the same file the Python codec is
// tested on (tests/codec_test.py), so the two clients are held to ONE set of expected lines. What this client does not speak
// (tagged lines, feedback lines, decode, the plan and estimate readers) is left out by `speaks`, and the count of cases it
// does run is held, so a client that quietly skipped everything would be red.
//
//   node --test tests/ts_codec_vectors_test.mjs        (make test-ts-client)
import test from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import { join, dirname } from 'node:path';
import { fileURLToPath } from 'node:url';

const ROOT = join(dirname(fileURLToPath(import.meta.url)), '..');
const FIXTURES = process.env.PLUGIN_HOSTD_CODEC_FIXTURES || join(ROOT, 'clients/fixtures/codec.json');
const { encode, parseReply, HostdError } = await import(join(ROOT, 'clients/typescript/lines.mjs'));
const { cases } = JSON.parse(readFileSync(FIXTURES, 'utf8'));

/** a vector's `parse` expectation, read from what parseReply gives (a success) or throws (a refusal, a protocol error) */
function parsed(line) {
  try {
    const r = parseReply(line);
    return { kind: 'resp', code: r.code, data: r.data };
  } catch (e) {
    if (!(e instanceof HostdError)) throw e;
    if (e.kind === 'refused') return { kind: 'resp', code: e.code, ...(e.word ? { name: e.word } : {}), data: [...e.data] };
    // a parse refusal: the client spells it by its text (`not found`), the vector by its constant (PROTOCOL_NOT_FOUND), so the
    // kind and the number are what is held equal (the number is the table's, -1 for not found)
    if (e.kind === 'protocol') return { kind: 'protocol_error', code: e.code };
    throw e; // malformed: the vector says "error"
  }
}

/** the vectors this client speaks: an untagged `encode`, and a `parse` of a reply or a parse refusal */
const speaks = (c) => (c.op === 'encode' && c.in.length === 2) ||
  (c.op === 'parse' && !c.in.startsWith('@') && !c.in.startsWith('#') && (c.in.startsWith('resp ') || /^(not found|few arguments)$/.test(c.in) || c.error));

const mine = cases.filter(speaks);

test('the client runs a real share of the shared vectors', () => {
  assert.ok(mine.filter((c) => c.op === 'encode').length >= 20, 'encode vectors run');
  assert.ok(mine.filter((c) => c.op === 'parse').length >= 8, 'parse vectors run');
});

for (const c of mine) {
  test(`${c.op} ${JSON.stringify(c.in)}`, () => {
    const run = () => (c.op === 'encode' ? encode(c.in[0], ...c.in[1]) : parsed(c.in));
    if (c.error) return assert.throws(run);
    if (!('expect' in c)) return;
    const { name, ...expect } = c.expect;
    assert.deepEqual(run(), c.expect.kind === 'protocol_error' ? expect : c.expect);
  });
}
