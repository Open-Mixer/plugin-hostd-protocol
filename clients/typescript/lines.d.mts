// SPDX-License-Identifier: GPL-3.0-or-later
import type { Verb } from './vocabulary.mjs';

export type ErrorKind = 'refused' | 'protocol' | 'malformed';
export declare class HostdError extends Error {
  constructor(kind: ErrorKind, reply: string, extra?: { code?: number | null; word?: string | null; data?: readonly string[] });
  readonly kind: ErrorKind;
  /** the reply line as the host wrote it */
  readonly reply: string;
  /** a refusal's number (`resp -3`), or the table's number of a parse refusal (`not found` is -1) */
  readonly code: number | null;
  /** the stable word of a refusal (`reply.host.instance-non-exists`), or the text of a parse refusal */
  readonly word: string | null;
  readonly data: readonly string[];
}
export interface Reply { readonly code: number; readonly data: readonly string[]; readonly reply: string }
export interface Point { readonly x: number; readonly y: number }
export declare function encodeWord(value: string | number): string;
export declare function encode(verb: Verb, ...args: ReadonlyArray<string | number>): string;
export declare function parseReply(line: string): Reply;
export declare function parseCurvePoints(result: Reply): Point[];
