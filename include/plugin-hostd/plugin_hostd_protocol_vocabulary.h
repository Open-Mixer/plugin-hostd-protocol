// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
#ifndef PLUGIN_HOSTD_PROTOCOL_VOCABULARY_H
#define PLUGIN_HOSTD_PROTOCOL_VOCABULARY_H
/*
 * GENERATED: DO NOT EDIT BY HAND.
 * Produced by `omx-contract render --target c` from FreeMixer/plugin-hostd-protocol's data/: the vocabulary
 * (docs/data-layout.md). A code is OMX_ and the code in upper snake case, and a reply code that carries a resp has it as that name and _RESP; a word is OMX_<ITEM>_<NAME>, each words
 * item with its _COUNT and an _INIT of its words in their order. A consumer writes and compares these, never a
 * literal of its own.
 */

/* PLUGIN_HOSTD_REPLY_CODES (data/verbs.json) */
#define OMX_REPLY_PLUGIN_HOSTD_PLACEMENT_INVALID "reply.plugin-hostd.placement-invalid" ///< not a placement, or not valid here
#define OMX_REPLY_PLUGIN_HOSTD_PLACEMENT_INVALID_RESP (-501)
#define OMX_REPLY_PLUGIN_HOSTD_PLACEMENT_INVALID_TEXT_EN "The plugin cannot be placed there; choose another place for it."
#define OMX_REPLY_PLUGIN_HOSTD_PLACEMENT_INVALID_TEXT_CA "El plugin no es pot col\302\267locar all\303\240; tria-hi un altre lloc."
#define OMX_REPLY_PLUGIN_HOSTD_NO_BACKEND "reply.plugin-hostd.no-backend" ///< no worker program for the scheme
#define OMX_REPLY_PLUGIN_HOSTD_NO_BACKEND_RESP (-502)
#define OMX_REPLY_PLUGIN_HOSTD_NO_BACKEND_TEXT_EN "No installed program can run this kind of plugin; install one, or choose a plugin of another kind."
#define OMX_REPLY_PLUGIN_HOSTD_NO_BACKEND_TEXT_CA "Cap programa instal\302\267lat pot executar aquest tipus de plugin; instal\302\267la'n un o tria un plugin d'un altre tipus."
#define OMX_REPLY_PLUGIN_HOSTD_WORKER_SPAWN "reply.plugin-hostd.worker-spawn" ///< the worker is not up
#define OMX_REPLY_PLUGIN_HOSTD_WORKER_SPAWN_RESP (-503)
#define OMX_REPLY_PLUGIN_HOSTD_WORKER_SPAWN_TEXT_EN "The program that runs the plugin is not running; wait a moment and try again."
#define OMX_REPLY_PLUGIN_HOSTD_WORKER_SPAWN_TEXT_CA "El programa que executa el plugin no est\303\240 en marxa; espera un moment i torna-ho a provar."
#define OMX_REPLY_PLUGIN_HOSTD_REPLAY "reply.plugin-hostd.replay" ///< the name of a refused replay on stderr, never a reply
#define OMX_REPLY_PLUGIN_HOSTD_REPLAY_RESP (-504)
#define OMX_REPLY_PLUGIN_HOSTD_REPLAY_TEXT_EN "A plugin could not be put back after a restart and was left out; add it again."
#define OMX_REPLY_PLUGIN_HOSTD_REPLAY_TEXT_CA "Un plugin no s'ha pogut tornar a posar despr\303\251s d'un reinici i s'ha deixat fora; afegeix-lo de nou."
#define OMX_REPLY_PLUGIN_HOSTD_GAVE_UP "reply.plugin-hostd.gave-up" ///< the storm bound is spent for that placement
#define OMX_REPLY_PLUGIN_HOSTD_GAVE_UP_RESP (-505)
#define OMX_REPLY_PLUGIN_HOSTD_GAVE_UP_TEXT_EN "The plugin kept failing and was given up on; check it, then add it again."
#define OMX_REPLY_PLUGIN_HOSTD_GAVE_UP_TEXT_CA "El plugin ha fallat repetidament i s'ha deixat de provar; revisa'l i afegeix-lo de nou."
#define OMX_REPLY_PLUGIN_HOSTD_NO_SUCH_WORKER "reply.plugin-hostd.no-such-worker" ///< no such worker
#define OMX_REPLY_PLUGIN_HOSTD_NO_SUCH_WORKER_RESP (-506)
#define OMX_REPLY_PLUGIN_HOSTD_NO_SUCH_WORKER_TEXT_EN "No plugin host with that name is running; refresh the list and choose another."
#define OMX_REPLY_PLUGIN_HOSTD_NO_SUCH_WORKER_TEXT_CA "No hi ha cap amfitri\303\263 de plugins amb aquest nom en marxa; actualitza la llista i tria'n un altre."
#define OMX_REPLY_PLUGIN_HOSTD_VERB_DROPPED "reply.plugin-hostd.verb-dropped" ///< the worker died on this very command and the daemon dropped it
#define OMX_REPLY_PLUGIN_HOSTD_VERB_DROPPED_RESP (-507)
#define OMX_REPLY_PLUGIN_HOSTD_VERB_DROPPED_TEXT_EN "The plugin stopped just as it was asked to do this, so the request was lost; try again."
#define OMX_REPLY_PLUGIN_HOSTD_VERB_DROPPED_TEXT_CA "El plugin s'ha aturat just quan se li demanava aix\303\262 i la petici\303\263 s'ha perdut; torna-ho a provar."
#define OMX_REPLY_PLUGIN_HOSTD_PIN_BINARY_MISMATCH "reply.plugin-hostd.pin-binary-mismatch" ///< a pinned file is missing or its SHA-256 differs
#define OMX_REPLY_PLUGIN_HOSTD_PIN_BINARY_MISMATCH_RESP (-509)
#define OMX_REPLY_PLUGIN_HOSTD_PIN_BINARY_MISMATCH_TEXT_EN "The plugin's files are not the ones that were approved; install the approved version, or approve this one."
#define OMX_REPLY_PLUGIN_HOSTD_PIN_BINARY_MISMATCH_TEXT_CA "Els fitxers del plugin no s\303\263n els que es van aprovar; instal\302\267la la versi\303\263 aprovada o aprova aquesta."
#define OMX_REPLY_PLUGIN_HOSTD_LV2_ADAPTER_REFUSED "reply.plugin-hostd.lv2-adapter-refused" ///< the lv2-clap worker's LV2 adapter cannot run this plugin
#define OMX_REPLY_PLUGIN_HOSTD_LV2_ADAPTER_REFUSED_RESP (-512)
#define OMX_REPLY_PLUGIN_HOSTD_LV2_ADAPTER_REFUSED_TEXT_EN "This LV2 plugin cannot run here because the adapter does not support something it needs; choose another plugin."
#define OMX_REPLY_PLUGIN_HOSTD_LV2_ADAPTER_REFUSED_TEXT_CA "Aquest plugin LV2 no es pot executar aqu\303\255 perqu\303\250 l'adaptador no admet alguna cosa que necessita; tria un altre plugin."
#define OMX_REPLY_PLUGIN_HOSTD_NO_PRESET_LIST "reply.plugin-hostd.no-preset-list" ///< the instance's worker cannot list the plugin's presets
#define OMX_REPLY_PLUGIN_HOSTD_NO_PRESET_LIST_RESP (-515)
#define OMX_REPLY_PLUGIN_HOSTD_NO_PRESET_LIST_TEXT_EN "This plugin cannot list its presets; load one by name, or choose one of your own."
#define OMX_REPLY_PLUGIN_HOSTD_NO_PRESET_LIST_TEXT_CA "Aquest plugin no pot llistar els seus presets; carrega'n un pel nom o tria'n un de propi."
#define OMX_REPLY_PLUGIN_HOSTD_PIN_REFUSED "reply.plugin-hostd.pin-refused" ///< the plugin's pin carries verdict=refused
#define OMX_REPLY_PLUGIN_HOSTD_PIN_REFUSED_RESP (-516)
#define OMX_REPLY_PLUGIN_HOSTD_PIN_REFUSED_TEXT_EN "This plugin failed its checks and is blocked; only you can allow it, by approving the plugin."
#define OMX_REPLY_PLUGIN_HOSTD_PIN_REFUSED_TEXT_CA "Aquest plugin no ha superat les comprovacions i est\303\240 bloquejat; nom\303\251s tu el pots permetre, aprovant-lo."
#define OMX_REPLY_PLUGIN_HOSTD_CHAIN_UNKNOWN_INSTANCE "reply.plugin-hostd.chain-unknown-instance" ///< a member is an instance no add made
#define OMX_REPLY_PLUGIN_HOSTD_CHAIN_UNKNOWN_INSTANCE_RESP (-518)
#define OMX_REPLY_PLUGIN_HOSTD_CHAIN_UNKNOWN_INSTANCE_TEXT_EN "The chain includes a plugin that was never added; add it first, or take it out of the chain."
#define OMX_REPLY_PLUGIN_HOSTD_CHAIN_UNKNOWN_INSTANCE_TEXT_CA "La cadena inclou un plugin que no s'ha afegit mai; afegeix-lo primer o treu-lo de la cadena."
#define OMX_REPLY_PLUGIN_HOSTD_CHAIN_DUPLICATE "reply.plugin-hostd.chain-duplicate" ///< an instance is named twice in one chain
#define OMX_REPLY_PLUGIN_HOSTD_CHAIN_DUPLICATE_RESP (-519)
#define OMX_REPLY_PLUGIN_HOSTD_CHAIN_DUPLICATE_TEXT_EN "The chain includes the same plugin twice; keep it only once."
#define OMX_REPLY_PLUGIN_HOSTD_CHAIN_DUPLICATE_TEXT_CA "La cadena inclou el mateix plugin dues vegades; deixa'l nom\303\251s una vegada."
#define OMX_REPLY_PLUGIN_HOSTD_CHAIN_INSTANCE_BUSY "reply.plugin-hostd.chain-instance-busy" ///< a member is already in another chain
#define OMX_REPLY_PLUGIN_HOSTD_CHAIN_INSTANCE_BUSY_RESP (-520)
#define OMX_REPLY_PLUGIN_HOSTD_CHAIN_INSTANCE_BUSY_TEXT_EN "A plugin in this chain is already in another chain; clear that chain first, or use another plugin."
#define OMX_REPLY_PLUGIN_HOSTD_CHAIN_INSTANCE_BUSY_TEXT_CA "Un plugin d'aquesta cadena ja \303\251s en una altra cadena; buida l'altra cadena primer o fes servir un altre plugin."
#define OMX_REPLY_PLUGIN_HOSTD_CHAIN_MIXED_FORMAT "reply.plugin-hostd.chain-mixed-format" ///< no one worker program runs every member
#define OMX_REPLY_PLUGIN_HOSTD_CHAIN_MIXED_FORMAT_RESP (-521)
#define OMX_REPLY_PLUGIN_HOSTD_CHAIN_MIXED_FORMAT_TEXT_EN "These plugins cannot run together as one chain because no single program can host them all; split the chain."
#define OMX_REPLY_PLUGIN_HOSTD_CHAIN_MIXED_FORMAT_TEXT_CA "Aquests plugins no poden funcionar junts en una sola cadena perqu\303\250 cap programa no els pot allotjar tots; divideix la cadena."
#define OMX_REPLY_PLUGIN_HOSTD_NO_SUCH_CHAIN "reply.plugin-hostd.no-such-chain" ///< a chain no chain line declared
#define OMX_REPLY_PLUGIN_HOSTD_NO_SUCH_CHAIN_RESP (-522)
#define OMX_REPLY_PLUGIN_HOSTD_NO_SUCH_CHAIN_TEXT_EN "There is no such chain; declare it first, or choose one that exists."
#define OMX_REPLY_PLUGIN_HOSTD_NO_SUCH_CHAIN_TEXT_CA "No existeix aquesta cadena; declara-la primer o tria'n una que existeixi."
#define OMX_REPLY_PLUGIN_HOSTD_PRESET_NOT_OFFERED "reply.plugin-hostd.preset-not-offered" ///< a stable name the plugin does not offer
#define OMX_REPLY_PLUGIN_HOSTD_PRESET_NOT_OFFERED_RESP (-523)
#define OMX_REPLY_PLUGIN_HOSTD_PRESET_NOT_OFFERED_TEXT_EN "This plugin has no preset with that name; choose one from its list."
#define OMX_REPLY_PLUGIN_HOSTD_PRESET_NOT_OFFERED_TEXT_CA "Aquest plugin no t\303\251 cap preset amb aquest nom; tria'n un de la llista."
#define OMX_REPLY_PLUGIN_HOSTD_PRESET_INVALID "reply.plugin-hostd.preset-invalid" ///< a preset file or path that is not the preset format
#define OMX_REPLY_PLUGIN_HOSTD_PRESET_INVALID_RESP (-524)
#define OMX_REPLY_PLUGIN_HOSTD_PRESET_INVALID_TEXT_EN "The preset file cannot be used because it is unreadable or not in the preset format; choose another file, or save the preset again."
#define OMX_REPLY_PLUGIN_HOSTD_PRESET_INVALID_TEXT_CA "El fitxer del preset no es pot fer servir perqu\303\250 no es pot llegir o no t\303\251 el format de preset; tria un altre fitxer o desa el preset de nou."
#define OMX_REPLY_PLUGIN_HOSTD_PRESET_NEEDS_STATE "reply.plugin-hostd.preset-needs-state" ///< a preset file that carries state for a worker that cannot take state
#define OMX_REPLY_PLUGIN_HOSTD_PRESET_NEEDS_STATE_RESP (-525)
#define OMX_REPLY_PLUGIN_HOSTD_PRESET_NEEDS_STATE_TEXT_EN "This preset holds the plugin's saved state, which this plugin host cannot load yet; use a preset made by the plugin itself."
#define OMX_REPLY_PLUGIN_HOSTD_PRESET_NEEDS_STATE_TEXT_CA "Aquest preset cont\303\251 l'estat desat del plugin, que aquest amfitri\303\263 encara no pot carregar; fes servir un preset fet pel mateix plugin."
#define OMX_REPLY_PLUGIN_HOSTD_PRESET_PLUGIN_CHANGED "reply.plugin-hostd.preset-plugin-changed" ///< the preset file's plugin id, version or fingerprint is not the instance's
#define OMX_REPLY_PLUGIN_HOSTD_PRESET_PLUGIN_CHANGED_RESP (-526)
#define OMX_REPLY_PLUGIN_HOSTD_PRESET_PLUGIN_CHANGED_TEXT_EN "The preset was made for another plugin, or another version of it, so nothing was applied; choose a preset made for this one."
#define OMX_REPLY_PLUGIN_HOSTD_PRESET_PLUGIN_CHANGED_TEXT_CA "El preset es va fer per a un altre plugin, o per a una altra versi\303\263, i no s'ha aplicat res; tria un preset fet per a aquest."
#define OMX_REPLY_PLUGIN_HOSTD_LOAD_REFUSED "reply.plugin-hostd.load-refused" ///< the plugin refused the preset or the state its file carries
#define OMX_REPLY_PLUGIN_HOSTD_LOAD_REFUSED_RESP (-527)
#define OMX_REPLY_PLUGIN_HOSTD_LOAD_REFUSED_TEXT_EN "The plugin did not accept the contents of the preset; try another preset."
#define OMX_REPLY_PLUGIN_HOSTD_LOAD_REFUSED_TEXT_CA "El plugin no ha acceptat el contingut del preset; prova'n un altre."
#define OMX_REPLY_PLUGIN_HOSTD_DIR_NOT_WRITABLE "reply.plugin-hostd.dir-not-writable" ///< descriptor_dir or qualification_dir cannot be made or written
#define OMX_REPLY_PLUGIN_HOSTD_DIR_NOT_WRITABLE_RESP (-528)
#define OMX_REPLY_PLUGIN_HOSTD_DIR_NOT_WRITABLE_TEXT_EN "A folder the plugins need cannot be created or written to; check its location and permissions."
#define OMX_REPLY_PLUGIN_HOSTD_DIR_NOT_WRITABLE_TEXT_CA "No es pot crear ni escriure una carpeta que els plugins necessiten; revisa'n la ubicaci\303\263 i els permisos."
#define OMX_REPLY_PLUGIN_HOSTD_DESCRIPTOR_ABSENT "reply.plugin-hostd.descriptor-absent" ///< no descriptor (panel) for the plugin's binary as it is now
#define OMX_REPLY_PLUGIN_HOSTD_DESCRIPTOR_ABSENT_RESP (-529)
#define OMX_REPLY_PLUGIN_HOSTD_DESCRIPTOR_ABSENT_TEXT_EN "There is no description of this plugin, so its panel cannot be shown; add the plugin again so it is scanned."
#define OMX_REPLY_PLUGIN_HOSTD_DESCRIPTOR_ABSENT_TEXT_CA "No hi ha cap descripci\303\263 d'aquest plugin, de manera que no se'n pot mostrar el tauler; torna a afegir el plugin perqu\303\250 s'examini."
#define OMX_REPLY_PLUGIN_HOSTD_DESCRIPTOR_PENDING "reply.plugin-hostd.descriptor-pending" ///< the scan its add started has not written the descriptor (panel) yet
#define OMX_REPLY_PLUGIN_HOSTD_DESCRIPTOR_PENDING_RESP (-530)
#define OMX_REPLY_PLUGIN_HOSTD_DESCRIPTOR_PENDING_TEXT_EN "The plugin is still being scanned; try again in a moment."
#define OMX_REPLY_PLUGIN_HOSTD_DESCRIPTOR_PENDING_TEXT_CA "El plugin encara s'est\303\240 examinant; torna-ho a provar d'aqu\303\255 a un moment."
#define OMX_REPLY_PLUGIN_HOSTD_DESCRIPTOR_SCAN_FAILED "reply.plugin-hostd.descriptor-scan-failed" ///< the plugin's last scan wrote no descriptor (panel)
#define OMX_REPLY_PLUGIN_HOSTD_DESCRIPTOR_SCAN_FAILED_RESP (-531)
#define OMX_REPLY_PLUGIN_HOSTD_DESCRIPTOR_SCAN_FAILED_TEXT_EN "The plugin could not be scanned, so its panel is not available; check that it works, then add it again."
#define OMX_REPLY_PLUGIN_HOSTD_DESCRIPTOR_SCAN_FAILED_TEXT_CA "No s'ha pogut examinar el plugin, de manera que el seu tauler no est\303\240 disponible; comprova que funciona i afegeix-lo de nou."
#define OMX_REPLY_PLUGIN_HOSTD_NO_SUCH_PLAN "reply.plugin-hostd.no-such-plan" ///< a chain no plan verb planned
#define OMX_REPLY_PLUGIN_HOSTD_NO_SUCH_PLAN_RESP (-532)
#define OMX_REPLY_PLUGIN_HOSTD_NO_SUCH_PLAN_TEXT_EN "There is no plan for this chain; make one first."
#define OMX_REPLY_PLUGIN_HOSTD_NO_SUCH_PLAN_TEXT_CA "No hi ha cap pla per a aquesta cadena; fes-ne un primer."
#define OMX_REPLY_PLUGIN_HOSTD_NO_RECORD "reply.plugin-hostd.no-record" ///< no qualification record names the plugin
#define OMX_REPLY_PLUGIN_HOSTD_NO_RECORD_RESP (-533)
#define OMX_REPLY_PLUGIN_HOSTD_NO_RECORD_TEXT_EN "This plugin has not been measured yet, so its cost is not known; run its qualification first."
#define OMX_REPLY_PLUGIN_HOSTD_NO_RECORD_TEXT_CA "Aquest plugin encara no s'ha mesurat, de manera que no se'n coneix el cost; fes-ne primer la qualificaci\303\263."
#define OMX_REPLY_PLUGIN_HOSTD_NO_CELL "reply.plugin-hostd.no-cell" ///< the plugin's qualification records hold no cost and latency at that rate and quantum
#define OMX_REPLY_PLUGIN_HOSTD_NO_CELL_RESP (-534)
#define OMX_REPLY_PLUGIN_HOSTD_NO_CELL_TEXT_EN "This plugin has no measurement at this sample rate and block size; measure it with these settings, or change them."
#define OMX_REPLY_PLUGIN_HOSTD_NO_CELL_TEXT_CA "Aquest plugin no t\303\251 cap mesura amb aquesta freq\303\274\303\250ncia de mostreig i mida de bloc; mesura'l amb aquests ajustos o canvia'ls."
#define OMX_REPLY_PLUGIN_HOSTD_UNKNOWN_PLUGIN "reply.plugin-hostd.unknown-plugin" ///< the uri names no plugin installed here
#define OMX_REPLY_PLUGIN_HOSTD_UNKNOWN_PLUGIN_RESP (-535)
#define OMX_REPLY_PLUGIN_HOSTD_UNKNOWN_PLUGIN_TEXT_EN "No plugin with that address is installed here; install it, or choose another."
#define OMX_REPLY_PLUGIN_HOSTD_UNKNOWN_PLUGIN_TEXT_CA "Aqu\303\255 no hi ha instal\302\267lat cap plugin amb aquesta adre\303\247a; instal\302\267la'l o tria'n un altre."

/* PLUGIN_HOSTD_PROTOCOL_EXTENSIONS (data/verbs.json) */
#define OMX_PLUGIN_HOSTD_PROTOCOL_EXTENSIONS_TAGS "tags" ///< request tags: a line may start with @<tag>, and its answer then starts with the same @<tag>
#define OMX_PLUGIN_HOSTD_PROTOCOL_EXTENSIONS_ERRORS "errors" ///< error words: a refusal resp <n> is followed by the stable word of its code, reply.<group>.<reason>, when the protocol declares one
#define OMX_PLUGIN_HOSTD_PROTOCOL_EXTENSIONS_SEQ "seq" ///< event sequence numbers: every feedback line starts with #<n>, 1 for the first after this hello and one more for each, so a gap is a missed line
#define OMX_PLUGIN_HOSTD_PROTOCOL_EXTENSIONS_COUNT 3u
#define OMX_PLUGIN_HOSTD_PROTOCOL_EXTENSIONS_INIT { "tags", "errors", "seq" }

#endif /* PLUGIN_HOSTD_PROTOCOL_VOCABULARY_H */
