// SPDX-License-Identifier: GPL-3.0-or-later
// The protocol's transport from Node (doc/ts-client.md), the twin of python3-plugin-hostd-client: connect the command port,
// then the feedback port, send a line, read its reply; and curvePoints. Every line is written and every reply read by the
// codec (./codec, the package's `.` export), the one TypeScript encoder of the protocol; the command port and the reply cap
// are the table's constants. A line is ended by one NUL and split on the NUL alone.
import net from 'node:net';
import { Protocol, ProtocolError } from './codec/dist/index.js';

export { Protocol, ProtocolError } from './codec/dist/index.js';

/**
 * A reply that is not a success, or a transport that failed. kind: 'refused' (resp <negative>), 'protocol' (a parse refusal
 * such as `not found`), 'malformed' (not a reply the protocol declares); and, with `reply` null, the transport's: 'connect',
 * 'closed', 'timeout', 'oversize' (a reply past the cap, the socket closed).
 */
export class HostdError extends Error {
  constructor(kind, reply, { code = null, word = null, data = [], detail = null } = {}) {
    super(detail ? `${kind}: ${detail}` : `${kind}: ${reply}${word && !reply.includes(word) ? ` (${word})` : ''}`);
    this.name = 'HostdError';
    this.kind = kind;
    this.reply = reply;
    this.code = code;
    this.word = word;
    this.data = data;
  }
}

const transport = (kind, detail, e) => Object.assign(new HostdError(kind, null, { detail, word: e?.code ?? null }), { cause: e });

/** A word the host sent with a refusal in a session granted `errors`. */
const WORD = /^reply\.[a-z0-9-]+\.[a-z0-9-]+$/;

/** A control character: no argument holds one, whatever its type. */
const CONTROL = /[\u0000-\u001f\u007f]/;

function open(port, host) {
  return new Promise((resolve, reject) => {
    const s = net.connect({ port, host });
    const fail = (e) => reject(transport('connect', `${host}:${port}: ${e.message}`, e));
    s.once('connect', () => { s.removeListener('error', fail); resolve(s); });
    s.once('error', fail);
  });
}

/**
 * Connect the command port and then the feedback port (the next one unless `feedbackPort` is given; `null` opens none).
 * `protocol` is the codec the lines are written and read by: the installed table when none is given.
 */
export async function connect({ protocol = new Protocol(), port, host = '127.0.0.1', feedbackPort, timeout = 10000, onFeedback } = {}) {
  const command = port ?? protocol.constant('default_command_port');
  const sock = await open(command, host);
  let fb = null;
  if (feedbackPort !== null) {
    try { fb = await open(feedbackPort ?? command + 1, host); } catch (e) { sock.destroy(); throw e; }
  }
  return new Client(protocol, sock, fb, timeout, onFeedback);
}

export class Client {
  #protocol; #max; #sock; #fb; #timeout; #buf = Buffer.alloc(0); #waiter = null; #dead = null; #chain = Promise.resolve();

  constructor(protocol, sock, fb, timeout, onFeedback) {
    this.#protocol = protocol;
    this.#max = protocol.constant('message_max');
    this.#sock = sock;
    this.#fb = fb;
    this.#timeout = timeout;
    sock.on('data', (d) => {
      this.#buf = Buffer.concat([this.#buf, d]);
      const nul = this.#buf.indexOf(0);
      if (nul > this.#max || (nul < 0 && this.#buf.length > this.#max)) {
        this.#fail(transport('oversize', `a reply past ${this.#max} bytes: the connection is closed`));
        this.#sock.destroy();
      } else this.#take();
    });
    sock.on('close', () => this.#fail(transport('closed', 'the peer closed')));
    sock.on('error', (e) => this.#fail(transport('closed', e.message, e)));
    if (fb) {
      let fbuf = Buffer.alloc(0);
      fb.on('data', (d) => {
        fbuf = Buffer.concat([fbuf, d]);
        let i;
        while ((i = fbuf.indexOf(0)) >= 0) {
          const line = fbuf.subarray(0, i).toString();
          fbuf = fbuf.subarray(i + 1);
          if (onFeedback) onFeedback(line);
        }
        if (fbuf.length > this.#max) fb.destroy();
      });
      fb.on('error', () => {});
    }
  }

  #take() {
    const i = this.#buf.indexOf(0);
    if (i < 0 || !this.#waiter) return;
    const w = this.#waiter;
    this.#waiter = null;
    const reply = this.#buf.subarray(0, i).toString();
    this.#buf = this.#buf.subarray(i + 1);
    w.resolve(reply);
  }

  #fail(err) {
    this.#dead ??= err;
    const w = this.#waiter;
    this.#waiter = null;
    if (w) w.reject(err);
  }

  /** One command line out and its one reply line back, as the host wrote it. Lines are sent in order, one at a time. */
  send(line) {
    const run = () => new Promise((resolve, reject) => {
      if (line.includes('\0')) return reject(new ProtocolError('a line holds no NUL'));
      if (this.#dead) return reject(this.#dead.kind === 'closed' ? transport('closed', `the peer closed on ${line}`, this.#dead.cause) : this.#dead);
      const timer = setTimeout(() => {
        this.#waiter = null;
        const e = transport('timeout', `no reply to ${line} in ${this.#timeout} ms`);
        this.#dead = e;
        this.close();
        reject(e);
      }, this.#timeout);
      this.#waiter = {
        resolve: (r) => { clearTimeout(timer); resolve(r); },
        reject: (e) => { clearTimeout(timer); reject(e.kind === 'closed' ? transport('closed', `the peer closed on ${line}`, e.cause) : e); },
      };
      this.#sock.write(Buffer.concat([Buffer.from(line), Buffer.alloc(1)]));
      this.#take();
    });
    const p = this.#chain.then(run, run);
    this.#chain = p.catch(() => {});
    return p;
  }

  /**
   * `verb` and its arguments written by the codec as the table declares them; the success `{ code, data, reply }`, or a
   * HostdError. A verb, an argument count or type the table does not declare, or a control character in an argument, is
   * the codec's ProtocolError, thrown before anything is sent.
   */
  async call(verb, ...args) {
    args.forEach((a, i) => {
      if (CONTROL.test(String(a))) throw new ProtocolError(`${verb}: argument ${i + 1} has a control character`);
    });
    return this.#read(await this.send(this.#protocol.encode(verb, args)));
  }

  /** A reply line read by the codec: `{ code, data, reply }` for a success, a HostdError for anything else. */
  #read(line) {
    let r;
    try { r = this.#protocol.parse(line); } catch (e) {
      if (e instanceof ProtocolError) throw new HostdError('malformed', line);
      throw e;
    }
    if (r.kind === 'resp' && r.code >= 0) return { code: r.code, data: r.data, reply: line };
    if (r.kind === 'resp') {
      const word = r.data.length === 1 && WORD.test(r.data[0]) ? r.data[0] : (r.name ?? null);
      throw new HostdError('refused', line, { code: r.code, word, data: r.data });
    }
    if (r.kind === 'protocol_error') throw new HostdError('protocol', line, { code: r.code, word: line });
    throw new HostdError('malformed', line);
  }

  /**
   * The points plugin-hostd draws for an instance's display, `n` grid points asked for, and the marks a corrector planted:
   * `{ points: [{ x, y }], marks: [{ freq, depth, q, origin }] }`, read by the table's reply form of curve_points; its count
   * must be the number of points.
   */
  async curvePoints(instance, display, n) {
    const result = await this.call('curve_points', instance, display, n);
    let r;
    try { r = this.#protocol.readReply('curve_points', result.data, result.code); } catch (e) {
      if (e instanceof ProtocolError) throw new HostdError('malformed', result.reply);
      throw e;
    }
    if (r.count !== r.points.length) throw new HostdError('malformed', result.reply);
    return { points: r.points, marks: r.marks };
  }

  close() {
    this.#sock.destroy();
    if (this.#fb) this.#fb.destroy();
  }
}
