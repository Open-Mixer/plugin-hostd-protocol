// SPDX-License-Identifier: GPL-3.0-or-later
// The protocol's transport from Node (doc/ts-client.md), the twin of python3-plugin-hostd-client: connect the command port,
// then the feedback port, send a line, read its reply; and curvePoints. Lines are written and read by ./lines.mjs from the
// generated vocabulary. A line is ended by one NUL and split on the NUL alone.
import net from 'node:net';
import { DEFAULT_COMMAND_PORT, MESSAGE_MAX } from './vocabulary.mjs';
import { HostdError, encode, parseCurvePoints, parseReply } from './lines.mjs';

export { HostdError, encode, encodeWord, parseReply, parseCurvePoints } from './lines.mjs';

const transport = (kind, detail, e) => Object.assign(new HostdError(kind, null, { detail, word: e?.code ?? null }), { cause: e });

function open(port, host) {
  return new Promise((resolve, reject) => {
    const s = net.connect({ port, host });
    const fail = (e) => reject(transport('connect', `${host}:${port}: ${e.message}`, e));
    s.once('connect', () => { s.removeListener('error', fail); resolve(s); });
    s.once('error', fail);
  });
}

/** Connect the command port and then the feedback port (the next one unless `feedbackPort` is given; `null` opens none). */
export async function connect({ port = DEFAULT_COMMAND_PORT, host = '127.0.0.1', feedbackPort, timeout = 10000, onFeedback } = {}) {
  const sock = await open(port, host);
  let fb = null;
  if (feedbackPort !== null) {
    try { fb = await open(feedbackPort ?? port + 1, host); } catch (e) { sock.destroy(); throw e; }
  }
  return new Client(sock, fb, timeout, onFeedback);
}

export class Client {
  #sock; #fb; #timeout; #buf = Buffer.alloc(0); #waiter = null; #dead = null; #chain = Promise.resolve();

  constructor(sock, fb, timeout, onFeedback) {
    this.#sock = sock;
    this.#fb = fb;
    this.#timeout = timeout;
    sock.on('data', (d) => {
      this.#buf = Buffer.concat([this.#buf, d]);
      const nul = this.#buf.indexOf(0);
      if (nul > MESSAGE_MAX || (nul < 0 && this.#buf.length > MESSAGE_MAX)) {
        this.#fail(transport('oversize', `a reply past ${MESSAGE_MAX} bytes: the connection is closed`));
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
        if (fbuf.length > MESSAGE_MAX) fb.destroy();
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
      if (line.includes('\0')) return reject(new TypeError('a line holds no NUL'));
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

  /** `verb` and its arguments as the vocabulary declares them; the success `{ code, data, reply }`, or a HostdError. */
  async call(verb, ...args) {
    return parseReply(await this.send(encode(verb, ...args)));
  }

  /** The points plugin-hostd draws for an instance's display, `n` grid points asked for: `[{ x, y }]`. */
  async curvePoints(instance, display, n) {
    return parseCurvePoints(await this.call('curve_points', instance, display, n));
  }

  close() {
    this.#sock.destroy();
    if (this.#fb) this.#fb.destroy();
  }
}
