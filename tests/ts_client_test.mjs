// SPDX-License-Identifier: GPL-3.0-or-later
// The TypeScript client (doc/ts-client.md, clients/typescript/) against a scripted peer, and its generator against a table with a
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
const { connect, HostdError } = await import(join(ROOT, 'clients/typescript/client.mjs'));
const { encode, encodeWord } = await import(join(ROOT, 'clients/typescript/lines.mjs'));
const { VERBS, REPLY_CODES, MESSAGE_MAX } = await import(join(ROOT, 'clients/typescript/vocabulary.mjs'));

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

const peers = [];
test.after(() => { for (const p of peers) p.stop(); });

/** a peer: the feedback port first or no command is read, as plugin-hostd does; `seen` collects the lines it was sent */
async function peer(opts = {}) {
  const seen = [];
  const p = { closed: false };
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
        if (opts.big) { s.write(Buffer.alloc(MESSAGE_MAX + 1, 97)); continue; }
        if (opts.close) { s.destroy(); continue; }
        const reply = Buffer.from((SCRIPT.get(line) ?? 'not found') + '\0');
        s.write(reply.subarray(0, 4));
        setTimeout(() => s.write(reply.subarray(4)), 5);
      }
    });
    s.on('close', () => { p.closed = true; });
    s.on('error', () => {});
  });
  const fb = net.createServer((s) => { feedback = s; sockets.push(s); s.on('error', () => {}); });
  for (let attempt = 0; attempt < 50; attempt++) {
    await new Promise((r) => cmd.listen(0, '127.0.0.1', r));
    const port = cmd.address().port;
    try {
      await new Promise((res, rej) => { fb.once('error', rej); fb.listen(port + 1, '127.0.0.1', res); });
      Object.assign(p, { port, seen, push: (l) => feedback.write(l + '\0'),
        stop: () => { for (const s of sockets) s.destroy(); cmd.close(); fb.close(); } });
      peers.push(p);
      return p;
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
    assert.doesNotMatch(readFileSync(join(ROOT, 'clients/typescript/vocabulary.mjs'), 'utf8'), /curve_added_by_a_test/);
  } finally { rmSync(dir, { recursive: true }); }
});

test('the committed vocabulary is what the generator writes, and a hand edit is caught', () => {
  assert.equal(gen(['--check']).status, 0, gen(['--check']).stderr);
  const dir = mkdtempSync(join(tmpdir(), 'tsclient-'));
  try {
    for (const f of ['vocabulary.mjs', 'vocabulary.d.mts']) copyFileSync(join(ROOT, 'clients/typescript', f), join(dir, f));
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

// ---- review fixes

const TABLE = JSON.parse(readFileSync(join(ROOT, 'protocol/plugin-hostd-protocol.json'), 'utf8'));

/** The daemon's tokenizer (src/utils.c, strarr_split with ENABLE_QUOTATION_MARKS, and parse_quote), read back in a test:
 * words split at spaces outside quotes; \" is a quote anywhere; inside quotes \\ is a backslash and "" a quote; the
 * quotes are not part of the value. */
function cSplit(line) {
  const out = [''];
  let quote = false;
  for (let i = 0; i < line.length; i++) {
    const c = line[i];
    const n = line[i + 1];
    if (c === ' ' && !quote) { while (line[i + 1] === ' ') i++; out.push(''); }
    else if (c === '\\' && n === '"') { out[out.length - 1] += '"'; i++; }
    else if (quote && c === '\\' && n === '\\') { out[out.length - 1] += '\\'; i++; }
    else if (c === '"') {
      if (!quote) quote = true;
      else if (n === '"') { out[out.length - 1] += '"'; i++; }
      else quote = false;
    } else out[out.length - 1] += c;
  }
  return out;
}

/** a value valid for a form's argument type: the table's own example, a literal word, or a float */
function sample(type) {
  if (TABLE.arg_types[type]) return TABLE.arg_types[type].example;
  if (type === 'float') return '0.5';
  return type.split('|')[0];
}

test('every verb of the table encodes with a valid value per argument type and reads back as it was written', () => {
  let n = 0;
  for (const c of TABLE.commands) {
    for (const f of c.forms) {
      const args = f.args.map(sample);
      if (f.tail) args.push('tail');
      const rest = TABLE.arg_types[f.args.at(-1)]?.rest;
      const line = encode(c.verb, ...args);
      const want = [c.verb, ...args.flatMap((a, i) => (rest && i === f.args.length - 1 ? a.split(' ') : /^quoted/.test(f.args[i]) ? cSplit(a) : [a]))];
      assert.deepEqual(cSplit(line), want, `${c.verb} ${f.args.join(' ')}`);
      n++;
    }
  }
  assert.ok(n >= 97);
});

test('the generator refuses a form type that is declared nowhere, by its name', () => {
  for (const bad of ['undeclared_type', 'flaot']) refused(bad);
});

function refused(bad) {
  const dir = mkdtempSync(join(tmpdir(), 'tsclient-'));
  try {
    const t = structuredClone(TABLE);
    t.commands.find((c) => c.verb === 'curve_points').forms[0].args[0] = bad;
    writeFileSync(join(dir, 'table.json'), JSON.stringify(t));
    const r = gen(['--table', join(dir, 'table.json'), '--out', dir]);
    assert.notEqual(r.status, 0);
    assert.match(r.stderr, new RegExp(bad));
  } finally { rmSync(dir, { recursive: true }); }
}

test('a word with a backslash, a quote or a space is read back by the daemon as it was written', () => {
  for (const v of ['gain', 'a b', 'a"b', 'a\\b', 'a b\\', 'a\\', '\\', 'x\\"y', 'a \\" b', '\\\\', '']) {
    assert.deepEqual(cSplit(encode('param_get', 3, v)), ['param_get', '3', v], JSON.stringify(v));
  }
});

test('a control character is refused in every argument of every verb, quoted or not', () => {
  for (const c of TABLE.commands) {
    for (const f of c.forms) {
      f.args.forEach((_, k) => {
        for (const bad of ['a\nb', 'a b\r', '\t', 'a\u007f']) {
          const args = f.args.map(sample);
          args[k] = bad;
          assert.throws(() => encode(c.verb, ...args), (e) => e instanceof TypeError && /control character/.test(e.message), `${c.verb} argument ${k + 1} ${JSON.stringify(bad)}`);
        }
      });
    }
  }
});

test('a reply past the cap closes the socket with a typed error', async () => {
  const p = await peer({ big: true });
  const c = await connect({ port: p.port, timeout: 5000 });
  await assert.rejects(c.send('ping'), (e) => e instanceof HostdError && e.kind === 'oversize');
  await new Promise((r) => setTimeout(r, 100));
  assert.ok(p.closed, 'the peer saw the connection close');
  await assert.rejects(c.send('ping'), (e) => e instanceof HostdError && e.kind === 'oversize');
  c.close(); p.stop();
});

test('transport errors are HostdError kinds: connect, closed, timeout', async () => {
  const srv = net.createServer();
  await new Promise((r) => srv.listen(0, '127.0.0.1', r));
  const dead = srv.address().port;
  await new Promise((r) => srv.close(r));
  await assert.rejects(connect({ port: dead }), (e) => e instanceof HostdError && e.kind === 'connect');
  let p = await peer({ close: true });
  let c = await connect({ port: p.port, timeout: 2000 });
  await assert.rejects(c.send('ping'), (e) => e instanceof HostdError && e.kind === 'closed');
  p.stop();
  p = await peer({ mute: true });
  c = await connect({ port: p.port, timeout: 100 });
  await assert.rejects(c.send('ping'), (e) => e instanceof HostdError && e.kind === 'timeout');
  p.stop();
});
