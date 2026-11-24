// SPDX-License-Identifier: GPL-3.0-or-later
import type { Verb } from './vocabulary.mjs';

export type ErrorKind = 'refused' | 'protocol' | 'malformed' | 'connect' | 'closed' | 'timeout' | 'oversize';
export declare class HostdError extends Error {
  constructor(kind: ErrorKind, reply: string | null, extra?: { code?: number | null; word?: string | null; data?: readonly string[]; detail?: string | null });
  readonly kind: ErrorKind;
  /** the reply line as the host wrote it; null for a transport kind (connect, closed, timeout, oversize) */
  readonly reply: string | null;
  /** a refusal's number (`resp -3`), or the table's number of a parse refusal (`not found` is -1) */
  readonly code: number | null;
  /** the stable word of a refusal (`reply.host.instance-non-exists`), or the text of a parse refusal */
  readonly word: string | null;
  readonly data: readonly string[];
}
export interface Reply { readonly code: number; readonly data: readonly string[]; readonly reply: string }
export interface Point { readonly x: number; readonly y: number }
/** a band a corrector planted: its frequency in Hz, depth in dB, Q and omx-dsp's origin word (CURVE_ORIGINS) */
export interface Mark { readonly freq: number; readonly depth: number; readonly q: number; readonly origin: string }
export interface Curve { readonly points: Point[]; readonly marks: Mark[] }
export declare function encodeWord(value: string | number): string;
export declare function encode(verb: Verb, ...args: ReadonlyArray<string | number>): string;
export declare function parseReply(line: string): Reply;
export declare function parseCurvePoints(result: Reply): Curve;
