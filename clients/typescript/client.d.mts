// SPDX-License-Identifier: GPL-3.0-or-later
import type { Point, Reply } from './lines.mjs';
import type { Verb } from './vocabulary.mjs';

export { HostdError, encode, encodeWord, parseReply, parseCurvePoints } from './lines.mjs';
export type { ErrorKind, Point, Reply } from './lines.mjs';

export interface ConnectOptions {
  /** the command port; 5555 by default */
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
  call(verb: Verb, ...args: ReadonlyArray<string | number>): Promise<Reply>;
  curvePoints(instance: number, display: number, n: number): Promise<Point[]>;
  close(): void;
}
