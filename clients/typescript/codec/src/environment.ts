// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
/**
 * The options a command line or a test takes from its environment: PLUGIN_HOSTD_PROTOCOL_TABLE, OMX_CONTRACT_PATH and
 * pkg-config's variables of plugin-hostd-protocol and omx-contract. Not part of the library (index.ts reads neither the
 * environment nor a program), not exported by the package: cli.ts and the tests use it.
 */
import { execFileSync } from 'node:child_process';
import type { ProtocolOptions } from './index.js';

function pkgConfig(env: NodeJS.ProcessEnv, ...args: string[]): string | undefined {
  try {
    const out = execFileSync(env['PKG_CONFIG'] ?? 'pkg-config', args, { encoding: 'utf8', stdio: ['ignore', 'pipe', 'ignore'] }).trim();
    return out || undefined;
  } catch {
    return undefined;
  }
}

/** The options the environment names; a key it does not name is left out, so the library's installed default stands. */
export function optionsFromEnvironment(env: NodeJS.ProcessEnv = process.env): ProtocolOptions {
  const table = env['PLUGIN_HOSTD_PROTOCOL_TABLE'] ?? pkgConfig(env, '--variable=table', 'plugin-hostd-protocol');
  const path = env['OMX_CONTRACT_PATH'];
  const contractdir = pkgConfig(env, '--variable=contractdir', 'omx-contract');
  const contractDirs = path ? path.split(':').filter(Boolean) : contractdir ? [contractdir] : undefined;
  const datadir = env['PLUGIN_HOSTD_PROTOCOL_DATADIR'] ?? pkgConfig(env, '--variable=datadir', 'plugin-hostd-protocol');
  return {
    ...(table ? { table } : {}),
    ...(contractDirs ? { contractDirs } : {}),
    ...(datadir ? { datadir } : {}),
  };
}
