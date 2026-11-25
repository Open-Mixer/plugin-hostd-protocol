// SPDX-License-Identifier: GPL-3.0-or-later
import type { Arg, Protocol } from './codec/dist/index.js';

export { Protocol, ProtocolError } from './codec/dist/index.js';

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

export interface ConnectOptions {
  /** the codec the lines are written and read by; the installed table when absent */
  protocol?: Protocol;
  /** the command port; the table's default_command_port when absent */
  port?: number;
  host?: string;
  /** the feedback port: the command port plus one by default; `null` opens none (a host started with -n) */
  feedbackPort?: number | null;
  /** how long a reply may take, in milliseconds; the connection is closed when it does not come */
  timeout?: number;
  /** each feedback line of the host, as it is written */
  onFeedback?: (line: string) => void;
}
export declare function connect(options?: ConnectOptions): Promise<Client>;
export declare class Client {
  send(line: string): Promise<string>;
  call(verb: string, ...args: Arg[]): Promise<Reply>;
  curvePoints(instance: number, display: number, n: number): Promise<Curve>;
  close(): void;
}
