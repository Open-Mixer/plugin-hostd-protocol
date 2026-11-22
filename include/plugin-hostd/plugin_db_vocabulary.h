// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
#ifndef PLUGIN_DB_VOCABULARY_H
#define PLUGIN_DB_VOCABULARY_H
/*
 * GENERATED: DO NOT EDIT BY HAND.
 * Produced by `omx-contract render --target c` from Open-Mixer/plugin-db's data/: the vocabulary
 * (docs/data-layout.md). A code is OMX_ and the code in upper snake case, and a reply code that carries a resp has it as that name and _RESP; a word is OMX_<ITEM>_<NAME>, each words
 * item with its _COUNT and an _INIT of its words in their order. A consumer writes and compares these, never a
 * literal of its own.
 */

/** @file
 * @brief The vocabulary of plugin-db: every code and every word as a string define, with its sentence
 * where it carries one. */

/* PLUGIN_DB_BUNDLE_RULES (contract.d/plugin-db/plugin-db.json) */
#define OMX_PLUGIN_DB_BUNDLE_RULES_NEWEST_VERSION "newest-version" ///< of the candidates, the one whose manifest gives the plugin the highest lv2:minorVersion, then the highest lv2:microVersion, a version not stated counting as 0; of equal versions the first in the search path, its directories in order and the bundles of one directory in byte order of their names. This is lilv's rule, so the host loads the bundle plugin-db pins.
/** The number of words of PLUGIN_DB_BUNDLE_RULES. */
#define OMX_PLUGIN_DB_BUNDLE_RULES_COUNT 1u
/** The words of PLUGIN_DB_BUNDLE_RULES in their declared order, as a C initializer list. */
#define OMX_PLUGIN_DB_BUNDLE_RULES_INIT { "newest-version" }
/** Expands `X(code, "word", NAME)` once per word of PLUGIN_DB_BUNDLE_RULES, in order, the code being its place in the order. Which LV2 bundle is the plugin, when several bundles of the search path name it: the one rule in force, and the only word of this item. A bundle realises the plugin when its manifest names the plugin with an lv2:binary; one that only names it (a MOD GUI, a preset bundle) complements it. The candidates are the realising bundles, or, when none realises it, the bundles that name it. */
#define OMX_PLUGIN_DB_BUNDLE_RULES(X) \
    X(0u, "newest-version", NEWEST_VERSION)

/* PLUGIN_DB_LAYERS (contract.d/plugin-db/plugin-db.json) */
#define OMX_PLUGIN_DB_LAYERS_DECLARED "declared" ///< meaning, units, travel and defaults, installed with the package by the plugin's owner, read only
#define OMX_PLUGIN_DB_LAYERS_SCANNED "scanned" ///< identity, controls, ports, presets and GUI evidence, written by plugin-qualify under its runner
#define OMX_PLUGIN_DB_LAYERS_PINNED "pinned" ///< the files of the plugin with their digests and the layout fingerprint, written through plugin-db
#define OMX_PLUGIN_DB_LAYERS_MEASURED "measured" ///< latency, cost and behaviour per rate and quantum with the conditions, written by the probes; may be wiped
#define OMX_PLUGIN_DB_LAYERS_JUDGED "judged" ///< the record: verdict, conditions and what it was measured under, written by plugin-qualify's judgement
#define OMX_PLUGIN_DB_LAYERS_REFERENCE "reference" ///< a record from a reference machine, installed by plugin-db-qualification, read only
#define OMX_PLUGIN_DB_LAYERS_CORRECTED "corrected" ///< renames, hidden controls and scale a person chose, written by the override module
#define OMX_PLUGIN_DB_LAYERS_WARNED_OVERRIDE "warned-override" ///< a person's choice to run a refused plugin anyway, alone; it binds the pin it was written for and does not survive an upgrade
#define OMX_PLUGIN_DB_LAYERS_OBSERVED "observed" ///< crash and xrun history, written by plugin-hostd and the engine
/** The number of words of PLUGIN_DB_LAYERS. */
#define OMX_PLUGIN_DB_LAYERS_COUNT 9u
/** The words of PLUGIN_DB_LAYERS in their declared order, as a C initializer list. */
#define OMX_PLUGIN_DB_LAYERS_INIT { "declared", "scanned", "pinned", "measured", "judged", "reference", "corrected", "warned-override", "observed" }
/** Expands `X(code, "word", NAME)` once per word of PLUGIN_DB_LAYERS, in order, the code being its place in the order. The layers of an entry, each with one writer. An entry is a directory of the store, and each layer is one file of it, the word and .json, in the envelope data/store.schema.json declares. The entry is the join of its layers at read time and is never stored. */
#define OMX_PLUGIN_DB_LAYERS(X) \
    X(0u, "declared", DECLARED) X(1u, "scanned", SCANNED) X(2u, "pinned", PINNED) X(3u, "measured", MEASURED) \
    X(4u, "judged", JUDGED) X(5u, "reference", REFERENCE) X(6u, "corrected", CORRECTED) X(7u, "warned-override", WARNED_OVERRIDE) \
    X(8u, "observed", OBSERVED)

/* PLUGIN_DB_FILES (contract.d/plugin-db/plugin-db.json) */
#define OMX_PLUGIN_DB_FILES_SAME "same" ///< every file the plugin loads is held by the pin and has the digest the pin holds
#define OMX_PLUGIN_DB_FILES_CHANGED "changed" ///< a file differs from its digest, is gone, is new, or another bundle is now the plugin
#define OMX_PLUGIN_DB_FILES_ABSENT "absent" ///< the plugin is in no file of the search path any more, or its pinned file is gone
#define OMX_PLUGIN_DB_FILES_COULD_NOT_TEST "could-not-test" ///< there is no pinned layer, or a file or the pin could not be read
/** The number of words of PLUGIN_DB_FILES. */
#define OMX_PLUGIN_DB_FILES_COUNT 4u
/** The words of PLUGIN_DB_FILES in their declared order, as a C initializer list. */
#define OMX_PLUGIN_DB_FILES_INIT { "same", "changed", "absent", "could-not-test" }
/** Expands `X(code, "word", NAME)` once per word of PLUGIN_DB_FILES, in order, the code being its place in the order. What the check found of the plugin's files against its pinned layer, as rpm -V reports an attribute. Could not test is never same. */
#define OMX_PLUGIN_DB_FILES(X) \
    X(0u, "same", SAME) X(1u, "changed", CHANGED) X(2u, "absent", ABSENT) X(3u, "could-not-test", COULD_NOT_TEST)

/* PLUGIN_DB_LAYOUT (contract.d/plugin-db/plugin-db.json) */
#define OMX_PLUGIN_DB_LAYOUT_SAME "same" ///< the layout has the pinned fingerprint: the files are the pinned bytes, or a host found the pinned fingerprint
#define OMX_PLUGIN_DB_LAYOUT_CHANGED "changed" ///< a host found another fingerprint than the pinned one
#define OMX_PLUGIN_DB_LAYOUT_NOT_YET_KNOWN "not-yet-known" ///< the pin holds no layout, or the files changed and no host has found their layout yet
/** The number of words of PLUGIN_DB_LAYOUT. */
#define OMX_PLUGIN_DB_LAYOUT_COUNT 3u
/** The words of PLUGIN_DB_LAYOUT in their declared order, as a C initializer list. */
#define OMX_PLUGIN_DB_LAYOUT_INIT { "same", "changed", "not-yet-known" }
/** Expands `X(code, "word", NAME)` once per word of PLUGIN_DB_LAYOUT, in order, the code being its place in the order. What the check found of the plugin's parameter layout against the layout fingerprint of its pinned layer. */
#define OMX_PLUGIN_DB_LAYOUT(X) \
    X(0u, "same", SAME) X(1u, "changed", CHANGED) X(2u, "not-yet-known", NOT_YET_KNOWN)

/* PLUGIN_DB_RECORD (contract.d/plugin-db/plugin-db.json) */
#define OMX_PLUGIN_DB_RECORD_CURRENT "current" ///< it was written for the pinned bytes and layout, and nothing of PLUGIN_DB_RECORD_CHANGES makes it stale
/** The source sentence (English) of OMX_PLUGIN_DB_RECORD_CURRENT. */
#define OMX_PLUGIN_DB_RECORD_CURRENT_TEXT_EN "Its record is current."
#define OMX_PLUGIN_DB_RECORD_STALE "stale" ///< it exists, and a change of PLUGIN_DB_RECORD_CHANGES, or files that could not be tested, keep it from answering
/** The source sentence (English) of OMX_PLUGIN_DB_RECORD_STALE. */
#define OMX_PLUGIN_DB_RECORD_STALE_TEXT_EN "Its record no longer applies."
#define OMX_PLUGIN_DB_RECORD_NONE "none" ///< there is no judged layer
/** The source sentence (English) of OMX_PLUGIN_DB_RECORD_NONE. */
#define OMX_PLUGIN_DB_RECORD_NONE_TEXT_EN "It has no record."
/** The number of words of PLUGIN_DB_RECORD. */
#define OMX_PLUGIN_DB_RECORD_COUNT 3u
/** The words of PLUGIN_DB_RECORD in their declared order, as a C initializer list. */
#define OMX_PLUGIN_DB_RECORD_INIT { "current", "stale", "none" }
/** Expands `X(code, "word", NAME)` once per word of PLUGIN_DB_RECORD, in order, the code being its place in the order. Whether the judged layer, the record, answers for these bytes on this machine now. */
#define OMX_PLUGIN_DB_RECORD(X) \
    X(0u, "current", CURRENT) X(1u, "stale", STALE) X(2u, "none", NONE)

/* PLUGIN_DB_RECORD_ACTIONS (contract.d/plugin-db/plugin-db.json) */
#define OMX_PLUGIN_DB_RECORD_ACTIONS_FULL_PATH "full-path" ///< scan, pin, measure and judge again
/** The source sentence (English) of OMX_PLUGIN_DB_RECORD_ACTIONS_FULL_PATH. */
#define OMX_PLUGIN_DB_RECORD_ACTIONS_FULL_PATH_TEXT_EN "Scan, measure and judge it again."
#define OMX_PLUGIN_DB_RECORD_ACTIONS_MEASURE_HERE "measure-here" ///< measure and judge on this class of machine; the record stays valid for its own class
/** The source sentence (English) of OMX_PLUGIN_DB_RECORD_ACTIONS_MEASURE_HERE. */
#define OMX_PLUGIN_DB_RECORD_ACTIONS_MEASURE_HERE_TEXT_EN "Measure it on this machine."
#define OMX_PLUGIN_DB_RECORD_ACTIONS_CONVERT "convert" ///< convert the record to the schema in force, with no new measurement
/** The source sentence (English) of OMX_PLUGIN_DB_RECORD_ACTIONS_CONVERT. */
#define OMX_PLUGIN_DB_RECORD_ACTIONS_CONVERT_TEXT_EN "Convert the record; nothing is measured again."
#define OMX_PLUGIN_DB_RECORD_ACTIONS_REJUDGE "rejudge" ///< judge the measurements again, automatically, with no new measurement
/** The source sentence (English) of OMX_PLUGIN_DB_RECORD_ACTIONS_REJUDGE. */
#define OMX_PLUGIN_DB_RECORD_ACTIONS_REJUDGE_TEXT_EN "Judge the measurements again; nothing is measured again."
#define OMX_PLUGIN_DB_RECORD_ACTIONS_REMEASURE_BACKGROUND "remeasure-background" ///< the record stays current, shown as measured under the host it names, and is measured again in the background
/** The source sentence (English) of OMX_PLUGIN_DB_RECORD_ACTIONS_REMEASURE_BACKGROUND. */
#define OMX_PLUGIN_DB_RECORD_ACTIONS_REMEASURE_BACKGROUND_TEXT_EN "The record still applies; it is measured again in the background."
/** The number of words of PLUGIN_DB_RECORD_ACTIONS. */
#define OMX_PLUGIN_DB_RECORD_ACTIONS_COUNT 5u
/** The words of PLUGIN_DB_RECORD_ACTIONS in their declared order, as a C initializer list. */
#define OMX_PLUGIN_DB_RECORD_ACTIONS_INIT { "full-path", "measure-here", "convert", "rejudge", "remeasure-background" }
/** Expands `X(code, "word", NAME)` once per word of PLUGIN_DB_RECORD_ACTIONS, in order, the code being its place in the order. What a change since a record was written asks for, each in the group of PLUGIN_DB_RECORD it leaves the record in. */
#define OMX_PLUGIN_DB_RECORD_ACTIONS(X) \
    X(0u, "full-path", FULL_PATH) X(1u, "measure-here", MEASURE_HERE) X(2u, "convert", CONVERT) X(3u, "rejudge", REJUDGE) \
    X(4u, "remeasure-background", REMEASURE_BACKGROUND)

/* PLUGIN_DB_RECORD_CHANGES (contract.d/plugin-db/plugin-db.json) */
#define OMX_PLUGIN_DB_RECORD_CHANGES_FILES "files" ///< the plugin's files are not the bytes the record was written for, or could not be tested
/** The source sentence (English) of OMX_PLUGIN_DB_RECORD_CHANGES_FILES. */
#define OMX_PLUGIN_DB_RECORD_CHANGES_FILES_TEXT_EN "Its files changed."
#define OMX_PLUGIN_DB_RECORD_CHANGES_LAYOUT "layout" ///< the parameter layout is not the one the record was written for, or is not yet known
/** The source sentence (English) of OMX_PLUGIN_DB_RECORD_CHANGES_LAYOUT. */
#define OMX_PLUGIN_DB_RECORD_CHANGES_LAYOUT_TEXT_EN "Its parameters changed."
#define OMX_PLUGIN_DB_RECORD_CHANGES_HOSTING_CHANGE "hosting-change" ///< the host is a release that declares a hosting change: its hosting epoch is past the record's. The host's owner declares it in the contract; no reader guesses
/** The source sentence (English) of OMX_PLUGIN_DB_RECORD_CHANGES_HOSTING_CHANGE. */
#define OMX_PLUGIN_DB_RECORD_CHANGES_HOSTING_CHANGE_TEXT_EN "The host changed how it hosts plugins."
#define OMX_PLUGIN_DB_RECORD_CHANGES_MACHINE_CLASS "machine-class" ///< this machine is of another class than the record's
/** The source sentence (English) of OMX_PLUGIN_DB_RECORD_CHANGES_MACHINE_CLASS. */
#define OMX_PLUGIN_DB_RECORD_CHANGES_MACHINE_CLASS_TEXT_EN "This is another kind of machine."
#define OMX_PLUGIN_DB_RECORD_CHANGES_SCHEMA "schema" ///< the record schema in force is not the record's
/** The source sentence (English) of OMX_PLUGIN_DB_RECORD_CHANGES_SCHEMA. */
#define OMX_PLUGIN_DB_RECORD_CHANGES_SCHEMA_TEXT_EN "The record format changed."
#define OMX_PLUGIN_DB_RECORD_CHANGES_RULES "rules" ///< the judgement rules in force are not the ones the record was judged by
/** The source sentence (English) of OMX_PLUGIN_DB_RECORD_CHANGES_RULES. */
#define OMX_PLUGIN_DB_RECORD_CHANGES_RULES_TEXT_EN "The qualification rules changed."
#define OMX_PLUGIN_DB_RECORD_CHANGES_HOST_VERSION "host-version" ///< the host is a normal release other than the one the record was measured under
/** The source sentence (English) of OMX_PLUGIN_DB_RECORD_CHANGES_HOST_VERSION. */
#define OMX_PLUGIN_DB_RECORD_CHANGES_HOST_VERSION_TEXT_EN "The host is another release."
/** The number of words of PLUGIN_DB_RECORD_CHANGES. */
#define OMX_PLUGIN_DB_RECORD_CHANGES_COUNT 7u
/** The words of PLUGIN_DB_RECORD_CHANGES in their declared order, as a C initializer list. */
#define OMX_PLUGIN_DB_RECORD_CHANGES_INIT { "files", "layout", "hosting-change", "machine-class", "schema", "rules", "host-version" }
/** Expands `X(code, "word", NAME)` once per word of PLUGIN_DB_RECORD_CHANGES, in order, the code being its place in the order. What may have changed since a record was written, each the row of the staleness table with the action it asks for. When several changed, the first in this order answers. */
#define OMX_PLUGIN_DB_RECORD_CHANGES(X) \
    X(0u, "files", FILES) X(1u, "layout", LAYOUT) X(2u, "hosting-change", HOSTING_CHANGE) X(3u, "machine-class", MACHINE_CLASS) \
    X(4u, "schema", SCHEMA) X(5u, "rules", RULES) X(6u, "host-version", HOST_VERSION)

/* PLUGIN_DB_STATES (contract.d/plugin-db/plugin-db.json) */
#define OMX_PLUGIN_DB_STATES_UNQUALIFIED "unqualified" ///< no current record, for any reason: runs alone in its own limited worker, never grouped; shown as not qualified, with the reason
/** The source sentence (English) of OMX_PLUGIN_DB_STATES_UNQUALIFIED. */
#define OMX_PLUGIN_DB_STATES_UNQUALIFIED_TEXT_EN "Not qualified: it runs alone, in a worker of its own."
#define OMX_PLUGIN_DB_STATES_QUALIFIED "qualified" ///< a current record whose verdict allows it and whose conditions are met here: runs as the verdict and the placement rules allow
/** The source sentence (English) of OMX_PLUGIN_DB_STATES_QUALIFIED. */
#define OMX_PLUGIN_DB_STATES_QUALIFIED_TEXT_EN "Qualified: it runs as its record allows."
#define OMX_PLUGIN_DB_STATES_ISOLATED_ONLY "isolated-only" ///< a current record that says it is safe only alone: runs alone
/** The source sentence (English) of OMX_PLUGIN_DB_STATES_ISOLATED_ONLY. */
#define OMX_PLUGIN_DB_STATES_ISOLATED_ONLY_TEXT_EN "Qualified to run alone only."
#define OMX_PLUGIN_DB_STATES_CONDITIONS_UNMET "conditions-unmet" ///< a current record with no cell for this rate or quantum, or a condition that fails here: runs alone, as unqualified
/** The source sentence (English) of OMX_PLUGIN_DB_STATES_CONDITIONS_UNMET. */
#define OMX_PLUGIN_DB_STATES_CONDITIONS_UNMET_TEXT_EN "Its record does not cover this machine's settings: it runs alone."
#define OMX_PLUGIN_DB_STATES_REFUSED "refused" ///< a current record or a shipped refusal says refused: not loaded, unless a warned override says run alone
/** The source sentence (English) of OMX_PLUGIN_DB_STATES_REFUSED. */
#define OMX_PLUGIN_DB_STATES_REFUSED_TEXT_EN "Refused: it is not loaded."
#define OMX_PLUGIN_DB_STATES_ABSENT "absent" ///< the files are gone: not loaded
/** The source sentence (English) of OMX_PLUGIN_DB_STATES_ABSENT. */
#define OMX_PLUGIN_DB_STATES_ABSENT_TEXT_EN "Its files are gone: it is not loaded."
/** The number of words of PLUGIN_DB_STATES. */
#define OMX_PLUGIN_DB_STATES_COUNT 6u
/** The words of PLUGIN_DB_STATES in their declared order, as a C initializer list. */
#define OMX_PLUGIN_DB_STATES_INIT { "unqualified", "qualified", "isolated-only", "conditions-unmet", "refused", "absent" }
/** Expands `X(code, "word", NAME)` once per word of PLUGIN_DB_STATES, in order, the code being its place in the order. What a plugin is, from its attributes, its record, a shipped record and a warned override, answered by one function and by no reader for itself. There is no upgraded and no crashed state: new bytes have no record, so they are unqualified. Each state is in the group of PLUGIN_DB_RUNS it runs as; a refusal a warned override covers runs alone instead. */
#define OMX_PLUGIN_DB_STATES(X) \
    X(0u, "unqualified", UNQUALIFIED) X(1u, "qualified", QUALIFIED) X(2u, "isolated-only", ISOLATED_ONLY) X(3u, "conditions-unmet", CONDITIONS_UNMET) \
    X(4u, "refused", REFUSED) X(5u, "absent", ABSENT)

/* PLUGIN_DB_RUNS (contract.d/plugin-db/plugin-db.json) */
#define OMX_PLUGIN_DB_RUNS_SHARED "shared" ///< as the verdict and the placement rules allow, in a worker other plugins may share
/** The source sentence (English) of OMX_PLUGIN_DB_RUNS_SHARED. */
#define OMX_PLUGIN_DB_RUNS_SHARED_TEXT_EN "Runs in a shared worker."
#define OMX_PLUGIN_DB_RUNS_ALONE "alone" ///< in its own limited worker, never grouped
/** The source sentence (English) of OMX_PLUGIN_DB_RUNS_ALONE. */
#define OMX_PLUGIN_DB_RUNS_ALONE_TEXT_EN "Runs alone, in a worker of its own."
#define OMX_PLUGIN_DB_RUNS_NOT_LOADED "not-loaded" ///< not loaded: shown with its reason and its saved values kept; an effect passes audio through, an instrument is silent
/** The source sentence (English) of OMX_PLUGIN_DB_RUNS_NOT_LOADED. */
#define OMX_PLUGIN_DB_RUNS_NOT_LOADED_TEXT_EN "Not loaded: an effect passes audio through, an instrument is silent."
/** The number of words of PLUGIN_DB_RUNS. */
#define OMX_PLUGIN_DB_RUNS_COUNT 3u
/** The words of PLUGIN_DB_RUNS in their declared order, as a C initializer list. */
#define OMX_PLUGIN_DB_RUNS_INIT { "shared", "alone", "not-loaded" }
/** Expands `X(code, "word", NAME)` once per word of PLUGIN_DB_RUNS, in order, the code being its place in the order. How a state runs at load. */
#define OMX_PLUGIN_DB_RUNS(X) \
    X(0u, "shared", SHARED) X(1u, "alone", ALONE) X(2u, "not-loaded", NOT_LOADED)

/* PLUGIN_DB_REASONS (contract.d/plugin-db/plugin-db.json) */
#define OMX_PLUGIN_DB_REASONS_NEVER_PINNED "never-pinned" ///< there is no pinned layer: never scanned or never pinned
/** The source sentence (English) of OMX_PLUGIN_DB_REASONS_NEVER_PINNED. */
#define OMX_PLUGIN_DB_REASONS_NEVER_PINNED_TEXT_EN "It has never been scanned."
#define OMX_PLUGIN_DB_REASONS_MEASUREMENT_PENDING "measurement-pending" ///< pinned, and there is no record yet
/** The source sentence (English) of OMX_PLUGIN_DB_REASONS_MEASUREMENT_PENDING. */
#define OMX_PLUGIN_DB_REASONS_MEASUREMENT_PENDING_TEXT_EN "It has not been measured yet."
#define OMX_PLUGIN_DB_REASONS_COULD_NOT_TEST "could-not-test" ///< the files or the pin could not be read
/** The source sentence (English) of OMX_PLUGIN_DB_REASONS_COULD_NOT_TEST. */
#define OMX_PLUGIN_DB_REASONS_COULD_NOT_TEST_TEXT_EN "Its files could not be read."
#define OMX_PLUGIN_DB_REASONS_FILES_CHANGED "files-changed" ///< the files are not the pinned bytes
/** The source sentence (English) of OMX_PLUGIN_DB_REASONS_FILES_CHANGED. */
#define OMX_PLUGIN_DB_REASONS_FILES_CHANGED_TEXT_EN "Its files have changed since they were scanned."
#define OMX_PLUGIN_DB_REASONS_LAYOUT_CHANGED "layout-changed" ///< the layout is not the pinned one
/** The source sentence (English) of OMX_PLUGIN_DB_REASONS_LAYOUT_CHANGED. */
#define OMX_PLUGIN_DB_REASONS_LAYOUT_CHANGED_TEXT_EN "Its parameters have changed since they were scanned."
#define OMX_PLUGIN_DB_REASONS_LAYOUT_NOT_YET_KNOWN "layout-not-yet-known" ///< the pin holds no layout, so a record cannot bind one
/** The source sentence (English) of OMX_PLUGIN_DB_REASONS_LAYOUT_NOT_YET_KNOWN. */
#define OMX_PLUGIN_DB_REASONS_LAYOUT_NOT_YET_KNOWN_TEXT_EN "Its parameters are not known yet."
#define OMX_PLUGIN_DB_REASONS_RECORD_STALE "record-stale" ///< the record is stale for a change of PLUGIN_DB_RECORD_CHANGES, which show names with its action
/** The source sentence (English) of OMX_PLUGIN_DB_REASONS_RECORD_STALE. */
#define OMX_PLUGIN_DB_REASONS_RECORD_STALE_TEXT_EN "Its record no longer applies and it needs measuring again."
#define OMX_PLUGIN_DB_REASONS_NOT_MEASURED_UNDER_HOST "not-measured-under-host" ///< the verdict says alone, and the record was not measured under plugin-hostd, which isolated only needs
/** The source sentence (English) of OMX_PLUGIN_DB_REASONS_NOT_MEASURED_UNDER_HOST. */
#define OMX_PLUGIN_DB_REASONS_NOT_MEASURED_UNDER_HOST_TEXT_EN "Its record says alone, but it was not measured under plugin-hostd."
#define OMX_PLUGIN_DB_REASONS_VERDICT_ALLOWS "verdict-allows" ///< the record's verdict allows a shared worker and its conditions are met here
/** The source sentence (English) of OMX_PLUGIN_DB_REASONS_VERDICT_ALLOWS. */
#define OMX_PLUGIN_DB_REASONS_VERDICT_ALLOWS_TEXT_EN "Its record allows a shared worker."
#define OMX_PLUGIN_DB_REASONS_VERDICT_ISOLATED "verdict-isolated" ///< the record's verdict says it is safe only alone
/** The source sentence (English) of OMX_PLUGIN_DB_REASONS_VERDICT_ISOLATED. */
#define OMX_PLUGIN_DB_REASONS_VERDICT_ISOLATED_TEXT_EN "Its record says it is safe only alone."
#define OMX_PLUGIN_DB_REASONS_REFERENCE_ONLY "reference-only" ///< a shipped record matches the pin and was measured on another class of machine: reference figures only, until measured here
/** The source sentence (English) of OMX_PLUGIN_DB_REASONS_REFERENCE_ONLY. */
#define OMX_PLUGIN_DB_REASONS_REFERENCE_ONLY_TEXT_EN "Measured on another kind of machine: reference figures only."
#define OMX_PLUGIN_DB_REASONS_NO_CELL "no-cell" ///< the record has no cell for this rate and quantum
/** The source sentence (English) of OMX_PLUGIN_DB_REASONS_NO_CELL. */
#define OMX_PLUGIN_DB_REASONS_NO_CELL_TEXT_EN "Its record has no measurement at this rate and quantum."
#define OMX_PLUGIN_DB_REASONS_CONDITION_FAILS "condition-fails" ///< a condition of the record does not hold here, or is not known here
/** The source sentence (English) of OMX_PLUGIN_DB_REASONS_CONDITION_FAILS. */
#define OMX_PLUGIN_DB_REASONS_CONDITION_FAILS_TEXT_EN "A condition of its record does not hold on this machine."
#define OMX_PLUGIN_DB_REASONS_RECORD_REFUSED "record-refused" ///< the record says refused; the way back is a rescan, a requalification or a warned override that runs it alone
/** The source sentence (English) of OMX_PLUGIN_DB_REASONS_RECORD_REFUSED. */
#define OMX_PLUGIN_DB_REASONS_RECORD_REFUSED_TEXT_EN "Its record says refused."
#define OMX_PLUGIN_DB_REASONS_SHIPPED_REFUSAL "shipped-refusal" ///< a shipped record of these bytes says refused, which wins over everything and no warned override overrides
/** The source sentence (English) of OMX_PLUGIN_DB_REASONS_SHIPPED_REFUSAL. */
#define OMX_PLUGIN_DB_REASONS_SHIPPED_REFUSAL_TEXT_EN "These files are refused by the record shipped with them."
#define OMX_PLUGIN_DB_REASONS_FILES_GONE "files-gone" ///< the plugin's files are gone; show names the path that was pinned
/** The source sentence (English) of OMX_PLUGIN_DB_REASONS_FILES_GONE. */
#define OMX_PLUGIN_DB_REASONS_FILES_GONE_TEXT_EN "Its files are gone."
/** The number of words of PLUGIN_DB_REASONS. */
#define OMX_PLUGIN_DB_REASONS_COUNT 16u
/** The words of PLUGIN_DB_REASONS in their declared order, as a C initializer list. */
#define OMX_PLUGIN_DB_REASONS_INIT { "never-pinned", "measurement-pending", "could-not-test", "files-changed", "layout-changed", "layout-not-yet-known", "record-stale", "not-measured-under-host", "verdict-allows", "verdict-isolated", "reference-only", "no-cell", "condition-fails", "record-refused", "shipped-refusal", "files-gone" }
/** Expands `X(code, "word", NAME)` once per word of PLUGIN_DB_REASONS, in order, the code being its place in the order. Why a plugin is in its state, each in the group of PLUGIN_DB_STATES it explains: what show prints beside the state. */
#define OMX_PLUGIN_DB_REASONS(X) \
    X(0u, "never-pinned", NEVER_PINNED) X(1u, "measurement-pending", MEASUREMENT_PENDING) X(2u, "could-not-test", COULD_NOT_TEST) X(3u, "files-changed", FILES_CHANGED) \
    X(4u, "layout-changed", LAYOUT_CHANGED) X(5u, "layout-not-yet-known", LAYOUT_NOT_YET_KNOWN) X(6u, "record-stale", RECORD_STALE) X(7u, "not-measured-under-host", NOT_MEASURED_UNDER_HOST) \
    X(8u, "verdict-allows", VERDICT_ALLOWS) X(9u, "verdict-isolated", VERDICT_ISOLATED) X(10u, "reference-only", REFERENCE_ONLY) X(11u, "no-cell", NO_CELL) \
    X(12u, "condition-fails", CONDITION_FAILS) X(13u, "record-refused", RECORD_REFUSED) X(14u, "shipped-refusal", SHIPPED_REFUSAL) X(15u, "files-gone", FILES_GONE)

/* PLUGIN_DB_ISOLATION_HOSTS (contract.d/plugin-db/plugin-db.json) */
#define OMX_PLUGIN_DB_ISOLATION_HOSTS_PLUGIN_HOSTD "plugin-hostd" ///< the daemon that hosts plugins for use, in workers of their own
/** The number of words of PLUGIN_DB_ISOLATION_HOSTS. */
#define OMX_PLUGIN_DB_ISOLATION_HOSTS_COUNT 1u
/** The words of PLUGIN_DB_ISOLATION_HOSTS in their declared order, as a C initializer list. */
#define OMX_PLUGIN_DB_ISOLATION_HOSTS_INIT { "plugin-hostd" }
/** Expands `X(code, "word", NAME)` once per word of PLUGIN_DB_ISOLATION_HOSTS, in order, the code being its place in the order. The hosts a verdict of isolated must have been measured under to make a plugin isolated only: a record that says alone and names another host leaves the plugin unqualified. */
#define OMX_PLUGIN_DB_ISOLATION_HOSTS(X) \
    X(0u, "plugin-hostd", PLUGIN_HOSTD)

/* PLUGIN_DB_SOURCES (contract.d/plugin-db/plugin-db.json) */
#define OMX_PLUGIN_DB_SOURCES_NONE "none" ///< no record answers
/** The source sentence (English) of OMX_PLUGIN_DB_SOURCES_NONE. */
#define OMX_PLUGIN_DB_SOURCES_NONE_TEXT_EN "No record."
#define OMX_PLUGIN_DB_SOURCES_LOCAL "local" ///< the judged layer: measured here
/** The source sentence (English) of OMX_PLUGIN_DB_SOURCES_LOCAL. */
#define OMX_PLUGIN_DB_SOURCES_LOCAL_TEXT_EN "Measured on this machine."
#define OMX_PLUGIN_DB_SOURCES_REFERENCE "reference" ///< the reference layer: a shipped record, measured on a reference machine
/** The source sentence (English) of OMX_PLUGIN_DB_SOURCES_REFERENCE. */
#define OMX_PLUGIN_DB_SOURCES_REFERENCE_TEXT_EN "Measured on a reference machine."
/** The number of words of PLUGIN_DB_SOURCES. */
#define OMX_PLUGIN_DB_SOURCES_COUNT 3u
/** The words of PLUGIN_DB_SOURCES in their declared order, as a C initializer list. */
#define OMX_PLUGIN_DB_SOURCES_INIT { "none", "local", "reference" }
/** Expands `X(code, "word", NAME)` once per word of PLUGIN_DB_SOURCES, in order, the code being its place in the order. Where the record that answers comes from, as show prints it. */
#define OMX_PLUGIN_DB_SOURCES(X) \
    X(0u, "none", NONE) X(1u, "local", LOCAL) X(2u, "reference", REFERENCE)

/* PLUGIN_DB_BAR (contract.d/plugin-db/plugin-db.json) */
#define OMX_PLUGIN_DB_BAR_QUALIFIED_ONLY "qualified-only" ///< only plugins whose state is qualified are offered by default
/** The source sentence (English) of OMX_PLUGIN_DB_BAR_QUALIFIED_ONLY. */
#define OMX_PLUGIN_DB_BAR_QUALIFIED_ONLY_TEXT_EN "Offer qualified plugins only."
#define OMX_PLUGIN_DB_BAR_WARN_ON_CRASHES "warn-on-crashes" ///< every plugin that is not refused or absent is offered, and only a crash is asked about
/** The source sentence (English) of OMX_PLUGIN_DB_BAR_WARN_ON_CRASHES. */
#define OMX_PLUGIN_DB_BAR_WARN_ON_CRASHES_TEXT_EN "Offer every plugin that is not refused; ask only after a crash."
#define OMX_PLUGIN_DB_BAR_LOAD_ALL "load-all" ///< every plugin is offered, the way back from a refusal included
/** The source sentence (English) of OMX_PLUGIN_DB_BAR_LOAD_ALL. */
#define OMX_PLUGIN_DB_BAR_LOAD_ALL_TEXT_EN "Offer every plugin, refused ones included."
/** The number of words of PLUGIN_DB_BAR. */
#define OMX_PLUGIN_DB_BAR_COUNT 3u
/** The words of PLUGIN_DB_BAR in their declared order, as a C initializer list. */
#define OMX_PLUGIN_DB_BAR_INIT { "qualified-only", "warn-on-crashes", "load-all" }
/** Expands `X(code, "word", NAME)` once per word of PLUGIN_DB_BAR, in order, the code being its place in the order. The bar a person sets for what may load, a user setting on both surfaces whose three words are declared here and read by plugin-hostd and both surfaces: the picker's and the menu's default view follow it. It filters a view and never changes what the reader answers; a refused plugin stays listed with its reason. */
#define OMX_PLUGIN_DB_BAR(X) \
    X(0u, "qualified-only", QUALIFIED_ONLY) X(1u, "warn-on-crashes", WARN_ON_CRASHES) X(2u, "load-all", LOAD_ALL)

/* PLUGIN_DB_LATENCY (contract.d/plugin-db/plugin-db.json) */
#define OMX_PLUGIN_DB_LATENCY_WITHIN "within" ///< the figure is at or under the line
/** The source sentence (English) of OMX_PLUGIN_DB_LATENCY_WITHIN. */
#define OMX_PLUGIN_DB_LATENCY_WITHIN_TEXT_EN "Latency within the line."
#define OMX_PLUGIN_DB_LATENCY_OVER "over" ///< the figure is over the line: shown by the owner's text, and the plugin loads
/** The source sentence (English) of OMX_PLUGIN_DB_LATENCY_OVER. */
#define OMX_PLUGIN_DB_LATENCY_OVER_TEXT_EN "Latency over the line; it still loads."
#define OMX_PLUGIN_DB_LATENCY_UNMEASURED "unmeasured" ///< the record has no line, or the cell has no figure
/** The source sentence (English) of OMX_PLUGIN_DB_LATENCY_UNMEASURED. */
#define OMX_PLUGIN_DB_LATENCY_UNMEASURED_TEXT_EN "Latency not measured."
/** The number of words of PLUGIN_DB_LATENCY. */
#define OMX_PLUGIN_DB_LATENCY_COUNT 3u
/** The words of PLUGIN_DB_LATENCY in their declared order, as a C initializer list. */
#define OMX_PLUGIN_DB_LATENCY_INIT { "within", "over", "unmeasured" }
/** Expands `X(code, "word", NAME)` once per word of PLUGIN_DB_LATENCY, in order, the code being its place in the order. Where a measured latency figure stands against the latency line, which plugin-qualify's judgement declares once and writes into the record as latencyLineMs beside the figure of each cell as latencyMs. Over the line is information: the plugin loads. A record without the line, or a cell without the figure, is unmeasured and never a guess. */
#define OMX_PLUGIN_DB_LATENCY(X) \
    X(0u, "within", WITHIN) X(1u, "over", OVER) X(2u, "unmeasured", UNMEASURED)

/* PLUGIN_DB_PIN_KEYS (contract.d/plugin-db/plugin-db.json) */
#define OMX_PLUGIN_DB_PIN_KEYS_VERDICT "verdict" ///< the qualification verdict of the pinned bytes, a word of PIN_VERDICTS of plugin-hostd-protocol's domain pin
#define OMX_PLUGIN_DB_PIN_KEYS_POOL "pool" ///< the thread pool word of the pinned bytes, a word of PIN_POOLS of plugin-hostd-protocol's domain pin
/** The number of words of PLUGIN_DB_PIN_KEYS. */
#define OMX_PLUGIN_DB_PIN_KEYS_COUNT 2u
/** The words of PLUGIN_DB_PIN_KEYS in their declared order, as a C initializer list. */
#define OMX_PLUGIN_DB_PIN_KEYS_INIT { "verdict", "pool" }
/** Expands `X(code, "word", NAME)` once per word of PLUGIN_DB_PIN_KEYS, in order, the code being its place in the order. The optional words of a pin, each this key, an equals sign and its value. Each appears at most once, in either order; a word with any other key is not a pin's. */
#define OMX_PLUGIN_DB_PIN_KEYS(X) \
    X(0u, "verdict", VERDICT) X(1u, "pool", POOL)

/* PLUGIN_DB_PIN_CHANGES (contract.d/plugin-db/plugin-db.json) */
#define OMX_PLUGIN_DB_PIN_CHANGES_FILES "files" ///< a pinned file is gone, or its bytes no longer have the digest the pin holds
#define OMX_PLUGIN_DB_PIN_CHANGES_LAYOUT "layout" ///< the plugin's parameter layout no longer has the fingerprint the pin holds
/** The number of words of PLUGIN_DB_PIN_CHANGES. */
#define OMX_PLUGIN_DB_PIN_CHANGES_COUNT 2u
/** The words of PLUGIN_DB_PIN_CHANGES in their declared order, as a C initializer list. */
#define OMX_PLUGIN_DB_PIN_CHANGES_INIT { "files", "layout" }
/** Expands `X(code, "word", NAME)` once per word of PLUGIN_DB_PIN_CHANGES, in order, the code being its place in the order. What counts as changed: a pin holds the digests of the plugin's files and the fingerprint of its parameter layout, and only these two differ from it. Anything else about a plugin, its name or its version included, changes no pin. */
#define OMX_PLUGIN_DB_PIN_CHANGES(X) \
    X(0u, "files", FILES) X(1u, "layout", LAYOUT)

/* PLUGIN_DB_PIN_LAYOUT_FORMATS (contract.d/plugin-db/plugin-db.json) */
#define OMX_PLUGIN_DB_PIN_LAYOUT_FORMATS_CLAP "clap" ///< every parameter clap.params gives, sorted by id
#define OMX_PLUGIN_DB_PIN_LAYOUT_FORMATS_LV2 "lv2" ///< every control port, sorted by index
/** The number of words of PLUGIN_DB_PIN_LAYOUT_FORMATS. */
#define OMX_PLUGIN_DB_PIN_LAYOUT_FORMATS_COUNT 2u
/** The words of PLUGIN_DB_PIN_LAYOUT_FORMATS in their declared order, as a C initializer list. */
#define OMX_PLUGIN_DB_PIN_LAYOUT_FORMATS_INIT { "clap", "lv2" }
/** Expands `X(code, "word", NAME)` once per word of PLUGIN_DB_PIN_LAYOUT_FORMATS, in order, the code being its place in the order. The formats the layout serialisation is written for, as the first line of its bytes names them. */
#define OMX_PLUGIN_DB_PIN_LAYOUT_FORMATS(X) \
    X(0u, "clap", CLAP) X(1u, "lv2", LV2)

#endif /* PLUGIN_DB_VOCABULARY_H */
