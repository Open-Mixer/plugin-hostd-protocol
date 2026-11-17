// SPDX-License-Identifier: GPL-3.0-or-later
// The TypeScript client (doc/ts-client.md, ts/client/) against a scripted peer, and its generator against a table with a
// verb added and a vocabulary edited by hand. The peer answers as the daemon does, NUL-ended, in the replies the golden
// transcript and doc/protocol.md hold for these lines; no daemon is built here, since the library needs omx-dsp's header.
//
//   node --test tests/ts_client_test.mjs        (make test-ts-client)
import test from 'node:test';
import assert from 'node:assert/strict';
import net from 'node:net';
import { spawnSync } from 'node:child_process';
import { mkdtempSync, copyFileSync, readFileSync, writeFileSync, rmSync } from 'node:fs';
import { tmpdir } from 'node:os';
import { join, dirname } from 'node:path';
import { fileURLToPath } from 'node:url';

const ROOT = join(dirname(fileURLToPath(import.meta.url)), '..');
const { connect, HostdError } = await import(join(ROOT, 'ts/client/client.mjs'));
const { encodeWord } = await import(join(ROOT, 'ts/client/lines.mjs'));
const { VERBS, REPLY_CODES } = await import(join(ROOT, 'ts/client/vocabulary.mjs'));

// line -> reply: the daemon's own answers, from tests/golden/plugin-hostd-protocol.txt and doc/protocol.md (Curves)
const SCRIPT = new Map([
  ['curve_points 3 0 3', 'resp 0 3 20.00,0.00 1000,3.18 20000,-0.02'],
  ['curve_points 3 1 3', 'resp -3'],
  ['curve_points 9 0 3', 'resp -537'],
  ['curve_points 4 0 1', 'resp -539'],
  ['curve_points 5 0 3', 'not found'],
  ['curve_points 6 0 3', 'resp -3 reply.host.instance-non-exists'],
  ['curve_points 7 0 3', 'resp 0 2 20.00,0.00'],
  ['ping', 'resp 0'],
]);

/** a peer: the feedback port first or no command is read, as plugin-hostd does; `seen` collects the lines it was sent */
async function peer(opts = {}) {
  const seen = [];
  let feedback = null;
  const sockets = [];
  const cmd = net.createServer((s) => {
    sockets.push(s);
    let buf = Buffer.alloc(0);
    s.on('data', (d) => {
      buf = Buffer.concat([buf, d]);
      let i;
      while ((i = buf.indexOf(0)) >= 0) {
        const line = buf.subarray(0, i).toString();
        buf = buf.subarray(i + 1);
        if (!feedback && !opts.noFeedback) { seen.push('BEFORE-FEEDBACK ' + line); continue; }
        seen.push(line);
        if (opts.mute) continue;
        if (opts.close) { s.destroy(); continue; }
        const reply = Buffer.from((SCRIPT.get(line) ?? 'not found') + '\0');
        s.write(reply.subarray(0, 4));
        setTimeout(() => s.write(reply.subarray(4)), 5);
      }
    });
    s.on('error', () => {});
  });
  const fb = net.createServer((s) => { feedback = s; sockets.push(s); s.on('error', () => {}); });
  for (let attempt = 0; attempt < 50; attempt++) {
    await new Promise((r) => cmd.listen(0, '127.0.0.1', r));
    const port = cmd.address().port;
    try {
      await new Promise((res, rej) => { fb.once('error', rej); fb.listen(port + 1, '127.0.0.1', res); });
      return { port, seen, push: (l) => feedback.write(l + '\0'),
        stop: () => { for (const s of sockets) s.destroy(); cmd.close(); fb.close(); } };
    } catch { await new Promise((r) => cmd.close(r)); }
  }
  throw new Error('no two adjacent ports');
}

test('the transport sends a line and reads its reply, the feedback port opened first', async () => {
  const p = await peer();
  const c = await connect({ port: p.port, timeout: 2000 });
  assert.equal(await c.send('ping'), 'resp 0');
  assert.equal(await c.send('curve_points 3 0 3'), 'resp 0 3 20.00,0.00 1000,3.18 20000,-0.02');
  assert.deepEqual(p.seen, ['ping', 'curve_points 3 0 3']);
  c.close(); p.stop();
});

test('feedback lines go to onFeedback and never into a reply', async () => {
  const p = await peer();
  const got = [];
  const c = await connect({ port: p.port, timeout: 2000, onFeedback: (l) => got.push(l) });
  p.push('#1 output_set 3 level 0.75');
  assert.equal(await c.send('ping'), 'resp 0');
  assert.deepEqual(got, ['#1 output_set 3 level 0.75']);
  c.close(); p.stop();
});

test('a peer that closes, or does not answer, is an error and not a wait', async () => {
  let p = await peer({ close: true });
  let c = await connect({ port: p.port, timeout: 2000 });
  await assert.rejects(c.send('ping'), /closed/);
  p.stop();
  p = await peer({ mute: true });
  c = await connect({ port: p.port, timeout: 100 });
  await assert.rejects(c.send('ping'), /no reply/);
  p.stop();
});

test('the vocabulary is generated from the table and the words of a line come from it', () => {
  assert.deepEqual(VERBS.curve_points.forms.map((f) => f.args), [['int', 'int', 'int']]);
  assert.ok(VERBS.curve_points.codes.includes(-537));
  assert.equal(REPLY_CODES['-537'], 'reply.plugin-hostd.curve-term-missing');
});

function gen(args) { return spawnSync('python3', [join(ROOT, 'tools/ts-client-gen.py'), ...args], { encoding: 'utf8', cwd: ROOT }); }

test('a verb added to the table shows up in the vocabulary after the generator runs', () => {
  const dir = mkdtempSync(join(tmpdir(), 'tsclient-'));
  try {
    const table = JSON.parse(readFileSync(join(ROOT, 'protocol/plugin-hostd-protocol.json'), 'utf8'));
    table.commands.push({ ...table.commands.find((c) => c.verb === 'curve_points'), verb: 'curve_added_by_a_test' });
    writeFileSync(join(dir, 'table.json'), JSON.stringify(table));
    const r = gen(['--table', join(dir, 'table.json'), '--out', dir]);
    assert.equal(r.status, 0, r.stderr);
    assert.match(readFileSync(join(dir, 'vocabulary.mjs'), 'utf8'), /curve_added_by_a_test/);
    assert.match(readFileSync(join(dir, 'vocabulary.d.mts'), 'utf8'), /'curve_added_by_a_test'/);
    assert.doesNotMatch(readFileSync(join(ROOT, 'ts/client/vocabulary.mjs'), 'utf8'), /curve_added_by_a_test/);
  } finally { rmSync(dir, { recursive: true }); }
});

test('the committed vocabulary is what the generator writes, and a hand edit is caught', () => {
  assert.equal(gen(['--check']).status, 0, gen(['--check']).stderr);
  const dir = mkdtempSync(join(tmpdir(), 'tsclient-'));
  try {
    for (const f of ['vocabulary.mjs', 'vocabulary.d.mts']) copyFileSync(join(ROOT, 'ts/client', f), join(dir, f));
    const f = join(dir, 'vocabulary.mjs');
    writeFileSync(f, readFileSync(f, 'utf8').replace("'curve_points'", "'curve_pointz'"));
    const r = gen(['--check', '--out', dir]);
    assert.notEqual(r.status, 0);
    assert.match(r.stderr, /vocabulary\.mjs is not what the generator writes/);
  } finally { rmSync(dir, { recursive: true }); }
});

test('curvePoints sends the words the table declares and parses the reply into points', async () => {
  const p = await peer();
  const c = await connect({ port: p.port, timeout: 2000 });
  const pts = await c.curvePoints(3, 0, 3);
  assert.deepEqual(p.seen, ['curve_points 3 0 3']);
  assert.deepEqual(pts, [{ x: 20, y: 0 }, { x: 1000, y: 3.18 }, { x: 20000, y: -0.02 }]);
  await assert.rejects(c.curvePoints(7, 0, 3), (e) => e instanceof HostdError && e.kind === 'malformed');
  await assert.rejects(c.curvePoints(3, 0, 'x'), TypeError);
  await assert.rejects(c.call('curve_points', 3, 0), TypeError);
  c.close(); p.stop();
});

test('errors are typed: a refusal by its code and word, a parse refusal by its text', async () => {
  const p = await peer();
  const c = await connect({ port: p.port, timeout: 2000 });
  await assert.rejects(c.curvePoints(9, 0, 3), (e) => e instanceof HostdError && e.kind === 'refused' && e.code === -537
    && e.word === 'reply.plugin-hostd.curve-term-missing' && e.reply === 'resp -537');
  await assert.rejects(c.curvePoints(4, 0, 1), (e) => e.kind === 'refused' && e.code === -539);
  await assert.rejects(c.curvePoints(3, 1, 3), (e) => e.kind === 'refused' && e.code === -3 && e.word === 'reply.host.instance-non-exists');
  await assert.rejects(c.curvePoints(6, 0, 3), (e) => e.kind === 'refused' && e.code === -3 && e.word === 'reply.host.instance-non-exists');
  await assert.rejects(c.curvePoints(5, 0, 3), (e) => e.kind === 'protocol' && e.code === -1 && e.word === 'not found');
  c.close(); p.stop();
});

test('a word is bare, quoted, or refused as the grammar says', () => {
  assert.equal(encodeWord('gain'), 'gain');
  assert.equal(encodeWord('a b'), '"a b"');
  assert.equal(encodeWord('a"b'), '"a\\"b"');
  assert.equal(encodeWord(''), '""');
  assert.throws(() => encodeWord('a\nb'), TypeError);
  assert.throws(() => encodeWord('x'.repeat(256)), TypeError);
});
