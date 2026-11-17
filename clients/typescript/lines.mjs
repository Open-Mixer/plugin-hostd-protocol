// SPDX-License-Identifier: GPL-3.0-or-later
// The protocol's lines without a socket (doc/ts-client.md): a command written from the vocabulary, a reply read into a
// result or a typed HostdError, curve_points' reply read into points. Every verb, argument type and code is the generated
// vocabulary's; nothing here names one.
import { ARG_TYPES, PROTOCOL_ERRORS, REPLY_CODES, STRING_MAX, VERBS } from './vocabulary.mjs';

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

const CONTROL = /[\u0000-\u001f\u007f]/;

/** A word as the daemon reads it: bare when it holds no space or double quote, else in quotes with each quote written \" and each backslash \\ (src/utils.c reads \\ as one backslash inside quotes; grammar/core.abnf's `quoted` has no such escape, so a value ending in a backslash cannot be written by it). */
export function encodeWord(value) {
  const v = String(value);
  if (CONTROL.test(v)) throw new TypeError(`a value with a control character has no spelling: ${JSON.stringify(v)}`);
  if (Buffer.byteLength(v) > STRING_MAX) throw new TypeError(`a value past ${STRING_MAX} bytes has no spelling`);
  return v !== '' && !/[ "]/.test(v) ? v : `"${v.replace(/["\\]/g, '\\$&')}"`;
}

function writeForm(verb, form, args) {
  const words = args.map((a, i) => {
    const type = form.args[i] ?? 'word';
    const s = String(a);
    if (type === 'word') return encodeWord(s);
    if (ARG_TYPES[type].rest) return s;
    if (!new RegExp(`^(?:${ARG_TYPES[type].pattern})$`).test(s)) throw new TypeError(`${verb}: argument ${i + 1}, ${JSON.stringify(s)}, is not a ${type}`);
    return s;
  });
  return [verb, ...words].join(' ');
}

/** The line for `verb` and its arguments: the first form with that many arguments whose types they all are, each written as its type says. A control character in any argument is refused. */
export function encode(verb, ...args) {
  const decl = Object.hasOwn(VERBS, verb) ? VERBS[verb] : undefined;
  if (!decl) throw new TypeError(`${verb} is not a verb of the protocol`);
  args.forEach((a, i) => {
    if (CONTROL.test(String(a))) throw new TypeError(`${verb}: argument ${i + 1} has a control character`);
  });
  const forms = decl.forms.filter((f) => args.length === f.args.length || (f.tail && args.length >= f.args.length));
  if (!forms.length) throw new TypeError(`${verb} takes ${decl.forms.map((f) => f.args.length + (f.tail ? '+' : '')).join(' or ')} arguments, not ${args.length}`);
  let first;
  for (const f of forms) {
    try { return writeForm(verb, f, args); } catch (e) { first ??= e; }
  }
  throw first;
}

const WORD = /^reply\.[a-z0-9-]+\.[a-z0-9-]+$/;

/** A reply line read: `{ code, data, reply }` for a success, a HostdError for anything else. */
export function parseReply(line) {
  const m = /^resp (-?[0-9]+)(?: (.*))?$/.exec(line);
  if (m) {
    const code = Number(m[1]);
    const data = m[2] === undefined ? [] : m[2].split(' ');
    if (code >= 0) return { code, data, reply: line };
    const word = data.length === 1 && WORD.test(data[0]) ? data[0] : (REPLY_CODES[String(code)] ?? null);
    throw new HostdError('refused', line, { code, word, data });
  }
  const e = PROTOCOL_ERRORS.find((p) => p.word === line);
  if (e) throw new HostdError('protocol', line, { code: e.code, word: e.word });
  throw new HostdError('malformed', line);
}

/** curve_points' success reply, `resp 0 <count> <x>,<y> ...`, as `[{ x, y }]`; its count must be the number of points. */
export function parseCurvePoints(result) {
  const [count, ...pairs] = result.data;
  const points = pairs.map((p) => p.split(',').map(Number));
  if (!/^[0-9]+$/.test(count ?? '') || Number(count) !== points.length || points.some((p) => p.length !== 2 || !p.every(Number.isFinite))) {
    throw new HostdError('malformed', result.reply);
  }
  return points.map(([x, y]) => ({ x, y }));
}
