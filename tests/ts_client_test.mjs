// SPDX-License-Identifier: GPL-3.0-or-later
// The TypeScript client (doc/ts-client.md, clients/typescript/) against a scripted peer. Its lines are the codec's
// (clients/typescript/codec, built by make test-ts-client), read from the table this tree writes; the peer answers as the
// daemon does, NUL-ended, in the replies the golden transcript and doc/protocol.md hold for these lines; no daemon is
// built here, since the library needs omx-dsp's header.
//
//   node --test tests/ts_client_test.mjs        (make test-ts-client)
import test from 'node:test';
import assert from 'node:assert/strict';
import net from 'node:net';
import { mkdtempSync, readFileSync, writeFileSync, rmSync } from 'node:fs';
import { tmpdir } from 'node:os';
import { join, dirname } from 'node:path';
import { fileURLToPath } from 'node:url';

const ROOT = join(dirname(fileURLToPath(import.meta.url)), '..');
const { connect, HostdError, Protocol, ProtocolError } = await import(join(ROOT, 'clients/typescript/client.mjs'));
const TABLE_PATH = join(ROOT, 'protocol/plugin-hostd-protocol.json');
const TABLE = JSON.parse(readFileSync(TABLE_PATH, 'utf8'));
const PROTOCOL = new Protocol({ table: TABLE_PATH, contractDirs: [] });
const MESSAGE_MAX = PROTOCOL.constant('message_max');

// line -> reply: the daemon's own answers, from tests/golden/plugin-hostd-protocol.txt and doc/protocol.md (Curves)
const SCRIPT = new Map([
  ['curve_points 3 0 3', 'resp 0 3 - 20.00,0.00 1000,3.18 20000,-0.02'],
  ['curve_points 4 4 2', 'resp 0 2 1000,-6.50,2.00,fbs-fixed;250,-3.00,4.50,hrp 20.00,0.00 1000,-6.50'],
  ['curve_points 8 0 2', 'resp 0 2 1000,-6.50,2.00 20.00,0.00 1000,-6.50'],
  ['curve_points 3 1 3', 'resp -3'],
  ['curve_points 9 0 3', 'resp -537'],
  ['curve_points 4 0 1', 'resp -539'],
  ['curve_points 5 0 3', 'not found'],
  ['curve_points 6 0 3', 'resp -3 reply.host.instance-non-exists'],
  ['curve_points 7 0 3', 'resp 0 2 - 20.00,0.00'],
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
        if (opts.big) { s.write(Buffer.alloc(opts.big, 97)); continue; }
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

/** the codec over a copy of the table, changed by `edit` */
function protocolWith(edit) {
  const dir = mkdtempSync(join(tmpdir(), 'tsclient-'));
  try {
    const t = structuredClone(TABLE);
    edit(t);
    writeFileSync(join(dir, 'table.json'), JSON.stringify(t));
    return new Protocol({ table: join(dir, 'table.json'), contractDirs: [] });
  } finally { rmSync(dir, { recursive: true }); }
}

const setConstant = (t, name, value) => { t.constants.find((c) => c.name === name).value = value; };

test('the transport sends a line and reads its reply, the feedback port opened first', async () => {
  const p = await peer();
  const c = await connect({ protocol: PROTOCOL, port: p.port, timeout: 2000 });
  assert.equal(await c.send('ping'), 'resp 0');
  assert.equal(await c.send('curve_points 3 0 3'), 'resp 0 3 - 20.00,0.00 1000,3.18 20000,-0.02');
  assert.deepEqual(p.seen, ['ping', 'curve_points 3 0 3']);
  c.close(); p.stop();
});

test('feedback lines go to onFeedback and never into a reply', async () => {
  const p = await peer();
  const got = [];
  const c = await connect({ protocol: PROTOCOL, port: p.port, timeout: 2000, onFeedback: (l) => got.push(l) });
  p.push('#1 output_set 3 level 0.75');
  assert.equal(await c.send('ping'), 'resp 0');
  assert.deepEqual(got, ['#1 output_set 3 level 0.75']);
  c.close(); p.stop();
});

test('a peer that closes, or does not answer, is an error and not a wait', async () => {
  let p = await peer({ close: true });
  let c = await connect({ protocol: PROTOCOL, port: p.port, timeout: 2000 });
  await assert.rejects(c.send('ping'), /closed/);
  p.stop();
  p = await peer({ mute: true });
  c = await connect({ protocol: PROTOCOL, port: p.port, timeout: 100 });
  await assert.rejects(c.send('ping'), /no reply/);
  p.stop();
});

// #106: one TypeScript encoder. The client writes a line by the codec's table and keeps no vocabulary of its own: a verb
// added to the table it is given is sent, and one the table does not declare is refused by the codec before it is sent
test("the client's lines are the codec's: a verb added to the table is written, an undeclared one is refused", async () => {
  const added = protocolWith((t) => t.commands.push({ ...t.commands.find((c) => c.verb === 'curve_points'), verb: 'curve_added_by_a_test' }));
  const p = await peer();
  let c = await connect({ protocol: added, port: p.port, timeout: 2000 });
  await assert.rejects(c.call('curve_added_by_a_test', 3, 0, 3), (e) => e instanceof HostdError && e.kind === 'protocol');
  assert.deepEqual(p.seen, ['curve_added_by_a_test 3 0 3']);
  c.close();
  c = await connect({ protocol: PROTOCOL, port: p.port, timeout: 2000 });
  await assert.rejects(c.call('curve_added_by_a_test', 3, 0, 3), ProtocolError);
  assert.deepEqual(p.seen, ['curve_added_by_a_test 3 0 3'], 'nothing was sent for the undeclared verb');
  c.close(); p.stop();
});

test("the command port and the reply cap are the table's constants", async () => {
  assert.equal(PROTOCOL.constant('default_command_port'), TABLE.constants.find((c) => c.name === 'default_command_port').value);
  const p = await peer();
  const c = await connect({ protocol: protocolWith((t) => setConstant(t, 'default_command_port', p.port)), timeout: 2000 });
  assert.equal(await c.send('ping'), 'resp 0');
  assert.deepEqual(p.seen, ['ping']);
  c.close(); p.stop();
  const q = await peer({ big: 65 });
  const d = await connect({ protocol: protocolWith((t) => setConstant(t, 'message_max', 64)), port: q.port, timeout: 2000 });
  await assert.rejects(d.send('ping'), (e) => e instanceof HostdError && e.kind === 'oversize' && /past 64 bytes/.test(e.message));
  d.close(); q.stop();
});

test('curvePoints sends the words the table declares and reads the reply into points and marks', async () => {
  const p = await peer();
  const c = await connect({ protocol: PROTOCOL, port: p.port, timeout: 2000 });
  const pts = await c.curvePoints(3, 0, 3);
  assert.deepEqual(p.seen, ['curve_points 3 0 3']);
  assert.deepEqual(pts, { points: [{ x: 20, y: 0 }, { x: 1000, y: 3.18 }, { x: 20000, y: -0.02 }], marks: [] });
  assert.deepEqual(await c.curvePoints(4, 4, 2), { points: [{ x: 20, y: 0 }, { x: 1000, y: -6.5 }],
    marks: [{ freq: 1000, depth: -6.5, q: 2, origin: 'fbs-fixed' }, { freq: 250, depth: -3, q: 4.5, origin: 'hrp' }] });
  await assert.rejects(c.curvePoints(8, 0, 2), (e) => e instanceof HostdError && e.kind === 'malformed');
  await assert.rejects(c.curvePoints(7, 0, 3), (e) => e instanceof HostdError && e.kind === 'malformed');
  await assert.rejects(c.curvePoints(3, 0, 'x'), ProtocolError);
  await assert.rejects(c.call('curve_points', 3, 0), ProtocolError);
  c.close(); p.stop();
});

test('errors are typed: a refusal by its code and word, a parse refusal by its text', async () => {
  const p = await peer();
  const c = await connect({ protocol: PROTOCOL, port: p.port, timeout: 2000 });
  await assert.rejects(c.curvePoints(9, 0, 3), (e) => e instanceof HostdError && e.kind === 'refused' && e.code === -537
    && e.word === 'reply.plugin-hostd.curve-term-missing' && e.reply === 'resp -537');
  await assert.rejects(c.curvePoints(4, 0, 1), (e) => e.kind === 'refused' && e.code === -539);
  await assert.rejects(c.curvePoints(3, 1, 3), (e) => e.kind === 'refused' && e.code === -3 && e.word === 'reply.host.instance-non-exists');
  await assert.rejects(c.curvePoints(6, 0, 3), (e) => e.kind === 'refused' && e.code === -3 && e.word === 'reply.host.instance-non-exists');
  await assert.rejects(c.curvePoints(5, 0, 3), (e) => e.kind === 'protocol' && e.code === -1 && e.word === 'not found');
  c.close(); p.stop();
});

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

/** a value valid for a form's argument type: a number for int and float, the table's own example (a quoted type's
 * example as the value it quotes, which the codec quotes), or a literal word */
function sample(type) {
  if (type === 'int') return 3;
  if (type === 'float') return 0.5;
  const ex = TABLE.arg_types[type]?.example;
  if (ex !== undefined) return /^quoted/.test(type) ? cSplit(ex)[0] : ex;
  return type.split('|')[0];
}

test('every verb of the table encodes with a valid value per argument type and the daemon reads it back as it was given', () => {
  let n = 0;
  for (const c of TABLE.commands) {
    for (const f of c.forms) {
      const args = f.args.map(sample);
      if (f.tail) args.push('tail');
      const line = PROTOCOL.encode(c.verb, args);
      assert.deepEqual(cSplit(line), [c.verb, ...args.map(String)], `${c.verb} ${f.args.join(' ')}`);
      n++;
    }
  }
  assert.ok(n >= 97);
});

test('a word with a backslash, a quote or a space is read back by the daemon as it was written', () => {
  for (const v of ['gain', 'a b', 'a"b', 'a\\b', 'a b\\', 'a\\', '\\', 'x\\"y', 'a \\" b', '\\\\', '']) {
    assert.deepEqual(cSplit(PROTOCOL.encode('param_get', [3, v])), ['param_get', '3', v], JSON.stringify(v));
  }
});

test('a control character is refused in every argument of every verb, before anything is sent', async () => {
  const p = await peer();
  const c = await connect({ protocol: PROTOCOL, port: p.port, timeout: 2000 });
  for (const v of TABLE.commands) {
    for (const f of v.forms) {
      for (let k = 0; k < f.args.length; k++) {
        for (const bad of ['a\nb', 'a b\r', '\t', 'a\u007f']) {
          const args = f.args.map(sample);
          args[k] = bad;
          await assert.rejects(c.call(v.verb, ...args), (e) => e instanceof ProtocolError && /control character/.test(e.message), `${v.verb} argument ${k + 1} ${JSON.stringify(bad)}`);
        }
      }
    }
  }
  assert.deepEqual(p.seen, []);
  c.close(); p.stop();
});

test('a reply past the cap closes the socket with a typed error', async () => {
  const p = await peer({ big: MESSAGE_MAX + 1 });
  const c = await connect({ protocol: PROTOCOL, port: p.port, timeout: 5000 });
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
  await assert.rejects(connect({ protocol: PROTOCOL, port: dead }), (e) => e instanceof HostdError && e.kind === 'connect');
  let p = await peer({ close: true });
  let c = await connect({ protocol: PROTOCOL, port: p.port, timeout: 2000 });
  await assert.rejects(c.send('ping'), (e) => e instanceof HostdError && e.kind === 'closed');
  p.stop();
  p = await peer({ mute: true });
  c = await connect({ protocol: PROTOCOL, port: p.port, timeout: 100 });
  await assert.rejects(c.send('ping'), (e) => e instanceof HostdError && e.kind === 'timeout');
  p.stop();
});
