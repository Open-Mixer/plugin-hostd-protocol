// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
//
// hosting-verb-hosts-check.mjs: the hosts a verb's implementedBy names (V29), declared as HOSTING_VERB_HOSTS by this
// package's domain hosting-verb (contract.d/hosting-verb/hosting-verb.json), the item omx-contract's verbs validator reads
// (contract.verb.host-unknown). The check moved here with the item (ruling R5): the guard goes with the words it holds.
//
//   node tests/hosting-verb-hosts-check.mjs [<hosting-verb.json>]
//   node tests/hosting-verb-hosts-check.mjs --self-test [<hosting-verb.json>]
//
// The item is a words item of exactly the three golden hosts, each with its title, its word and its text; a missing word,
// an extra one, a changed spelling or an empty text is a failure (omx-contract's validator refuses a host a verb names
// that is not declared, not an extra host or an empty text). The self test applies each of those to a copy and each must
// fail. Exit 0 held; 1 a difference.
import { readFileSync } from 'node:fs';

const args = process.argv.slice(2);
const selfTest = args.includes('--self-test');
const path = args.find((a) => !a.startsWith('--'))
  ?? new URL('../contract.d/hosting-verb/hosting-verb.json', import.meta.url).pathname;
const ITEM = 'HOSTING_VERB_HOSTS';
// title -> the word a verb's implementedBy writes
const GOLDEN = { pluginHostd: 'plugin-hostd', omxClapHost: 'omx-clap-host', modHost: 'mod-host' };

function problems(doc) {
  const out = [];
  const item = doc?.components?.schemas?.[ITEM];
  if (!item) return [`${ITEM} is not declared`];
  if (item['x-kind'] !== 'words') out.push('x-kind is not words');
  const words = {};
  for (const w of item.oneOf ?? []) {
    if (w.title in words) out.push(`word ${w.title} is declared twice`);
    words[w.title] = w;
  }
  for (const [title, word] of Object.entries(GOLDEN)) {
    const w = words[title];
    if (!w) out.push(`word ${title} is missing`);
    else {
      if (w.const !== word) out.push(`word ${title} is ${JSON.stringify(w.const)}, not ${word}`);
      if (!(typeof w['x-text'] === 'string' && w['x-text'].trim())) out.push(`word ${title} has no text (x-text)`);
    }
  }
  for (const title of Object.keys(words)) if (!(title in GOLDEN)) out.push(`word ${title} is not a golden host`);
  return out;
}

let doc;
try {
  doc = JSON.parse(readFileSync(path, 'utf8'));
} catch (e) {
  console.log(`FAIL ${path}: ${e.message}`);
  process.exit(1);
}
const bad = problems(doc);
for (const b of bad) console.log(`FAIL ${path}: ${b}`);
if (bad.length) process.exit(1);
console.log(`ok   ${ITEM} holds the three golden hosts`);

if (selfTest) {
  const words = (d) => d.components.schemas[ITEM].oneOf;
  const word = (d, title) => words(d).find((w) => w.title === title);
  const arms = {
    'a word dropped': (d) => { d.components.schemas[ITEM].oneOf = words(d).filter((w) => w.title !== 'modHost'); },
    'a word added': (d) => { words(d).push({ const: 'jalv', title: 'jalv', description: 'jalv', 'x-code': 3, 'x-text': 'jalv' }); },
    'a spelling changed': (d) => { word(d, 'pluginHostd').const = 'plugin_hostd'; },
    'a text emptied': (d) => { word(d, 'omxClapHost')['x-text'] = ''; },
    'a word given twice': (d) => { words(d).push(structuredClone(word(d, 'modHost'))); },
    'the item dropped': (d) => { delete d.components.schemas[ITEM]; },
  };
  let failed = 0;
  for (const [arm, apply] of Object.entries(arms)) {
    const copy = structuredClone(doc);
    apply(copy);
    const red = problems(copy).length > 0;
    console.log(`${red ? 'ok  ' : 'FAIL'} sabotage: ${arm} is refused`);
    if (!red) failed++;
  }
  process.exit(failed ? 1 : 0);
}
