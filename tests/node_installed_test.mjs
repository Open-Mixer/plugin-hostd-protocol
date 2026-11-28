#!/usr/bin/env node
// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
//
// nodejs-plugin-hostd-protocol as a Node consumer meets it installed: a scratch application whose
// node_modules/@openmixer/plugin-hostd-protocol is a link to the installed tree, as the console's package build links
// its installed @openmixer packages, and nothing of this source tree in reach. The application imports the bare name
// (the codec's Protocol), ./client (the transport, which must hand back the codec's own Protocol), ./codec-vectors and
// ./package.json by the package's exports, and meets every vector of clients/fixtures/codec.json through the codec's
// own dispatcher (dist/cli.js's answer, the one the codec's tests use): each encode, decode, parse and read gives what
// the vector expects, and each vector marked error throws ProtocolError. The codec's @openmixer/omx-contract must resolve
// through the installed tree's node_modules link, to a tree outside the package.
//
//   node tests/node_installed_test.mjs <nodedir> [--table F] [--contract-dirs D:D] [--datadir D]
//
// <nodedir> holds @openmixer/plugin-hostd-protocol (/usr/lib/node_modules once the package is installed). With no
// option the codec reads the installed files (its INSTALLED paths): the table plugin-hostd-protocol installs, the
// domains plugin-hostd-protocol-contract and omx-contract-data install; the options name a staged tree's instead.
// Exit 0 every check passed, 1 one failed (named), 2 usage.
import { spawnSync } from 'node:child_process';
import { existsSync, lstatSync, mkdirSync, mkdtempSync, readdirSync, realpathSync, rmSync, symlinkSync, writeFileSync } from 'node:fs';
import { tmpdir } from 'node:os';
import { join } from 'node:path';

const argv = process.argv.slice(2);
const nodedir = argv.shift();
const options = {};
while (argv.length) {
  const flag = argv.shift();
  const value = argv.shift();
  if (value === undefined) break;
  if (flag === '--table') options.table = value;
  else if (flag === '--contract-dirs') options.contractDirs = value.split(':').filter(Boolean);
  else if (flag === '--datadir') options.datadir = value;
  else {
    console.error(`node-installed: unknown option ${flag}`);
    process.exit(2);
  }
}
if (!nodedir) {
  console.error('usage: node_installed_test.mjs <nodedir> [--table F] [--contract-dirs D:D] [--datadir D]');
  process.exit(2);
}
const installed = join(nodedir, '@openmixer', 'plugin-hostd-protocol');
if (!existsSync(join(installed, 'package.json'))) {
  console.error(`node-installed: FAIL no package at ${installed}`);
  process.exit(1);
}
// the one node_modules of the tree is its own, holding the link to omx-contract alone: a node_modules packed under the
// codec (its build toolchain, the registry's @openmixer/omx-contract) would be read first by the codec, before the link
const nested = [];
const walk = (dir, rel) => {
  for (const e of readdirSync(dir, { withFileTypes: true })) {
    const r = rel ? `${rel}/${e.name}` : e.name;
    if (e.name === 'node_modules' && r !== 'node_modules') nested.push(r);
    else if (e.isDirectory() && r !== 'node_modules') walk(join(dir, e.name), r);
  }
};
walk(installed, '');
const top = join(installed, 'node_modules');
const own = existsSync(top) ? readdirSync(top).flatMap((s) => [s, ...(s === '@openmixer' ? readdirSync(join(top, s)).map((n) => `${s}/${n}`) : [])]).sort() : [];
const linked = own.includes('@openmixer/omx-contract') && lstatSync(join(top, '@openmixer/omx-contract')).isSymbolicLink();
if (nested.length || JSON.stringify(own) !== JSON.stringify(['@openmixer', '@openmixer/omx-contract']) || !linked) {
  console.error(`node-installed: FAIL the tree carries node_modules beyond its omx-contract link: ${[...nested, ...own.map((o) => `node_modules/${o}`)].slice(0, 6).join(', ')}`);
  process.exit(1);
}

const APP = String.raw`
import assert from 'node:assert/strict';
import { realpathSync } from 'node:fs';
import { fileURLToPath } from 'node:url';
const fail = (what) => { console.error('node-installed: FAIL ' + what); process.exit(1); };
const installed = realpathSync(process.env.NODE_INSTALLED_TREE);
const options = JSON.parse(process.env.NODE_INSTALLED_OPTIONS);

let codec, client, vectors, pkg;
try {
  codec = await import('@openmixer/plugin-hostd-protocol');
  client = await import('@openmixer/plugin-hostd-protocol/client');
  vectors = (await import('@openmixer/plugin-hostd-protocol/codec-vectors', { with: { type: 'json' } })).default;
  pkg = (await import('@openmixer/plugin-hostd-protocol/package.json', { with: { type: 'json' } })).default;
} catch (e) {
  fail('the installed package does not import: ' + (e.code ?? '') + ' ' + e.message.split('\n')[0]);
}
const main = fileURLToPath(import.meta.resolve('@openmixer/plugin-hostd-protocol'));
if (!main.startsWith(installed + '/')) fail('the bare name resolved to ' + main + ', outside the installed tree ' + installed);
console.log('ok   ' + pkg.name + ' ' + pkg.version + ' resolves to ' + main.slice(installed.length + 1) + ' in ' + installed);
if (typeof codec.Protocol !== 'function') fail('the bare name exports no Protocol');
if (client.Protocol !== codec.Protocol) fail('./client hands back a Protocol that is not the codec\'s');
if (typeof client.connect !== 'function' || typeof client.Client !== 'function') fail('./client exports no connect and Client');
console.log('ok   ./client resolves to ' + fileURLToPath(import.meta.resolve('@openmixer/plugin-hostd-protocol/client')).slice(installed.length + 1) + ', its Protocol the codec\'s');

// the codec imported @openmixer/omx-contract/catalog at load, from the package's node_modules: that must be a link out
let contract;
try { contract = realpathSync(installed + '/node_modules/@openmixer/omx-contract'); } catch (e) { fail('the package\'s node_modules/@openmixer/omx-contract does not resolve: ' + e.message); }
if (contract.startsWith(installed + '/')) fail('@openmixer/omx-contract is a copy inside the package: ' + contract);
console.log('ok   the codec\'s @openmixer/omx-contract is ' + contract + ' (omx-contract ' + JSON.parse((await import('node:fs')).readFileSync(contract + '/package.json', 'utf8')).version + ')');

const { answer } = await import(new URL('./cli.js', import.meta.resolve('@openmixer/plugin-hostd-protocol')));
const protocol = new codec.Protocol(options);
const clean = (v) => Array.isArray(v) ? v.map(clean)
  : v && typeof v === 'object' ? Object.fromEntries(Object.entries(v).filter(([, x]) => x !== null && x !== undefined).map(([k, x]) => [k, clean(x)]))
  : v;
const met = {};
for (const c of vectors.cases) {
  const what = c.op + ' ' + JSON.stringify(c.in);
  if (c.error) {
    try { answer(protocol, c.op, c.in); } catch (e) {
      if (!(e instanceof codec.ProtocolError)) fail(what + ' threw ' + e + ', not a ProtocolError');
      met[c.op] = (met[c.op] ?? 0) + 1;
      continue;
    }
    fail(what + ' is accepted; the vector says it is refused');
  }
  let got;
  try { got = answer(protocol, c.op, c.in); } catch (e) { fail(what + ' threw ' + e.message); }
  try { if ('expect' in c) assert.deepEqual(clean(got), clean(c.expect)); } catch { fail(what + ' gave ' + JSON.stringify(got) + ', expected ' + JSON.stringify(c.expect)); }
  met[c.op] = (met[c.op] ?? 0) + 1;
}
const total = Object.values(met).reduce((a, b) => a + b, 0);
if (total !== vectors.cases.length || !met.encode || !met.parse || !met.decode) fail('met ' + total + ' of ' + vectors.cases.length + ' vectors: ' + JSON.stringify(met));
console.log('ok   ' + total + ' of ' + vectors.cases.length + ' codec vectors met: ' + Object.entries(met).sort().map(([k, n]) => k + ' ' + n).join(', '));
`;

const work = mkdtempSync(join(tmpdir(), 'node-installed-'));
try {
  mkdirSync(join(work, 'node_modules', '@openmixer'), { recursive: true });
  symlinkSync(realpathSync(installed), join(work, 'node_modules', '@openmixer', 'plugin-hostd-protocol'));
  writeFileSync(join(work, 'app.mjs'), APP);
  const r = spawnSync(process.execPath, [join(work, 'app.mjs')], {
    cwd: work,
    stdio: 'inherit',
    env: { ...process.env, NODE_INSTALLED_TREE: installed, NODE_INSTALLED_OPTIONS: JSON.stringify(options), NODE_PATH: '' },
  });
  process.exitCode = r.status ?? 1;
} finally {
  rmSync(work, { recursive: true, force: true });
}
