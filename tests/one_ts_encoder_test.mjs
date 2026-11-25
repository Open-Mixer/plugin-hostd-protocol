// SPDX-License-Identifier: GPL-3.0-or-later
// One TypeScript encoder (#106): the package writes and reads the protocol's lines in the codec (clients/typescript/codec,
// the `.` export) and nowhere else. The thin client (./client) imports the codec and keeps no encoder, no reply parser and
// no generated vocabulary of its own; the package exports no ./lines and no ./vocabulary, and no generator writes one. Each
// rule is also run on a tree with the old second encoder planted, so a check that sees nothing is red.
//
//   node --test tests/one_ts_encoder_test.mjs        (make test-ts-client)
import test from 'node:test';
import assert from 'node:assert/strict';
import { existsSync, readdirSync, readFileSync } from 'node:fs';
import { join, dirname } from 'node:path';
import { fileURLToPath } from 'node:url';

const ROOT = join(dirname(fileURLToPath(import.meta.url)), '..');
const CLIENT_DIR = 'clients/typescript';
const CODEC = './codec/dist/index.js';
/** what a second encoder is made of: its exports, its files, its generator */
const SECOND = {
  exports: ['./lines', './vocabulary'],
  files: [`${CLIENT_DIR}/lines.mjs`, `${CLIENT_DIR}/lines.d.mts`, `${CLIENT_DIR}/vocabulary.mjs`, `${CLIENT_DIR}/vocabulary.d.mts`, 'tools/ts-client-gen.py'],
};
/** a line writer or reader of its own, or an import of the generated vocabulary */
const OWN = /\bfunction\s+(encode|encodeWord|parseReply|parseCurvePoints)\b|from\s+'\.\/(lines|vocabulary)\.mjs'/;

/** the tree as the rules read it: the package's exports, which files exist, the thin client's sources, the Makefile */
function tree() {
  const sources = {};
  for (const f of readdirSync(join(ROOT, CLIENT_DIR))) {
    if (/\.(mjs|d\.mts)$/.test(f)) sources[f] = readFileSync(join(ROOT, CLIENT_DIR, f), 'utf8');
  }
  return {
    exports: Object.keys(JSON.parse(readFileSync(join(ROOT, 'package.json'), 'utf8')).exports),
    files: SECOND.files.filter((f) => existsSync(join(ROOT, f))),
    sources,
    makefile: readFileSync(join(ROOT, 'Makefile'), 'utf8'),
  };
}

function problems(t) {
  const out = [];
  for (const e of SECOND.exports) if (t.exports.includes(e)) out.push(`package.json exports ${e}`);
  for (const f of t.files) out.push(`${f} exists`);
  if (!t.sources['client.mjs']) out.push(`${CLIENT_DIR}/client.mjs is missing`);
  for (const [f, src] of Object.entries(t.sources)) {
    const m = OWN.exec(src);
    if (m) out.push(`${CLIENT_DIR}/${f} writes or reads lines itself: ${m[0]}`);
  }
  for (const f of ['client.mjs', 'client.d.mts']) {
    if (t.sources[f] && !t.sources[f].includes(`from '${CODEC}'`)) out.push(`${CLIENT_DIR}/${f} does not import the codec, ${CODEC}`);
  }
  if (/ts-client-gen/.test(t.makefile)) out.push('the Makefile runs tools/ts-client-gen.py');
  return out;
}

test('the package has one TypeScript encoder, the codec', () => {
  assert.deepEqual(problems(tree()), []);
});

test('each rule finds the second encoder planted back', () => {
  const t = tree();
  const planted = {
    'an export ./lines': { ...t, exports: [...t.exports, './lines'] },
    'an export ./vocabulary': { ...t, exports: [...t.exports, './vocabulary'] },
    'lines.mjs back': { ...t, files: [`${CLIENT_DIR}/lines.mjs`] },
    'the generator back': { ...t, files: ['tools/ts-client-gen.py'] },
    'an encoder in the client': { ...t, sources: { ...t.sources, 'client.mjs': `import { Protocol } from '${CODEC}';\nexport function encode(verb) { return verb; }\n` } },
    'the vocabulary imported': { ...t, sources: { ...t.sources, 'client.mjs': `import { Protocol } from '${CODEC}';\nimport { VERBS } from './vocabulary.mjs';\n` } },
    'a client without the codec': { ...t, sources: { ...t.sources, 'client.mjs': 'export const connect = () => {};\n' } },
    'the generator in the Makefile': { ...t, makefile: `${t.makefile}\n\tpython3 tools/ts-client-gen.py\n` },
  };
  for (const [what, p] of Object.entries(planted)) assert.equal(problems(p).length, 1, `${what}: ${problems(p).join('; ')}`);
});
