// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
#ifndef PLUGIN_HOSTD_PROTOCOL_HOST_VERBS_H
#define PLUGIN_HOSTD_PROTOCOL_HOST_VERBS_H
/*
 * GENERATED: DO NOT EDIT BY HAND.
 * Produced by `omx-contract render --target c` from plugin-hostd-protocol's files.
 * The host verbs (docs/contract-dir.md): a verb is OMX_VERB_<VERB>, its word, OMX_VERB_<VERB>_FMT, the line
 * protocol_add_command takes, and OMX_VERB_<VERB>_REPLY; a verb that answers refusals carries their resp numbers
 * in OMX_VERB_<VERB>_CODES_INIT. plugin_hostd_protocol_host_verbs() is the table of what each host owes each verb.
 */

/** @file
 * @brief The host verbs of plugin-hostd-protocol: each verb's word, line format and reply, and
 * the table of what each host owes it. */

#include <string.h>

#ifndef OMX_HOST_VERB_TYPES_H
#define OMX_HOST_VERB_TYPES_H
/* What a host owes a verb: required, answered as its grammar says; optional, answered or refused, never misread;
 * unsupported, refused on purpose. */
enum omx_host_verb_level {
  OMX_HOST_VERB_UNSUPPORTED = 0,
  OMX_HOST_VERB_OPTIONAL = 1,
  OMX_HOST_VERB_REQUIRED = 2,
};
/* The hosts of HOSTING_VERB_HOSTS, a column each of a verb's level, in this order. */
#define OMX_HOST_VERB_HOST_PLUGIN_HOSTD 0u
#define OMX_HOST_VERB_HOST_OMX_CLAP_HOST 1u
#define OMX_HOST_VERB_HOST_MOD_HOST 2u
#define OMX_HOST_VERB_HOST_COUNT 3u
#define OMX_HOST_VERB_HOSTS_INIT { "plugin-hostd", "omx-clap-host", "mod-host" }
struct omx_host_verb {
  const char *verb;
  const char *format;
  const char *reply;
  enum omx_host_verb_level level[OMX_HOST_VERB_HOST_COUNT];
};
#endif /* OMX_HOST_VERB_TYPES_H */

/* PLUGIN_HOSTD_PROTOCOL_DAEMON_VERBS.workerList (contract.d/plugin-hostd-protocol/plugin-hostd-protocol.json): The workers, one record each: <core> is always -: the setting placement pins a worker to the whole set of audio cores, never to one core of its own, so no worker has a core to report; <pool> is on for a worker started with the CLAP thread pool (the pin of its plugin carries pool=cores, and no other plugin shares the worker), qualified for every other. */
#define OMX_VERB_WORKER_LIST "worker_list"
#define OMX_VERB_WORKER_LIST_FMT "worker_list"
#define OMX_VERB_WORKER_LIST_REPLY "resp <n> <worker>:<pid>:<format>:<state>:<core>:<pool>:<place>:<i>,<i>,..."
#define OMX_VERB_WORKER_LIST_CODES_COUNT 2u
#define OMX_VERB_WORKER_LIST_CODES_INIT { -901, -902 } /* reply.host.memory-allocation, reply.host.invalid-operation */

/* PLUGIN_HOSTD_PROTOCOL_DAEMON_VERBS.instanceInfo (contract.d/plugin-hostd-protocol/plugin-hostd-protocol.json): Where an instance lives and what it has cost. */
#define OMX_VERB_INSTANCE_INFO "instance_info"
#define OMX_VERB_INSTANCE_INFO_FMT "instance_info %i"
#define OMX_VERB_INSTANCE_INFO_REPLY "resp 0 <worker> <pid> <state> <crashes> <quarantined>"
#define OMX_VERB_INSTANCE_INFO_CODES_COUNT 3u
#define OMX_VERB_INSTANCE_INFO_CODES_INIT { -3, -901, -902 } /* reply.host.instance-non-exists, reply.host.memory-allocation, reply.host.invalid-operation */

/* PLUGIN_HOSTD_PROTOCOL_DAEMON_VERBS.supervisorReset (contract.d/plugin-hostd-protocol/plugin-hostd-protocol.json): Re-arm the storm bound and the backoff of a given-up placement: [<worker> | all]. */
#define OMX_VERB_SUPERVISOR_RESET "supervisor_reset"
#define OMX_VERB_SUPERVISOR_RESET_FMT "supervisor_reset ..."
#define OMX_VERB_SUPERVISOR_RESET_REPLY "resp 0"
#define OMX_VERB_SUPERVISOR_RESET_CODES_COUNT 3u
#define OMX_VERB_SUPERVISOR_RESET_CODES_INIT { -506, -901, -902 } /* reply.plugin-hostd.no-such-worker, reply.host.memory-allocation, reply.host.invalid-operation */

/* PLUGIN_HOSTD_PROTOCOL_DAEMON_VERBS.quarantineClear (contract.d/plugin-hostd-protocol/plugin-hostd-protocol.json): Let a quarantined instance go back where its placement says: <instance> | all. */
#define OMX_VERB_QUARANTINE_CLEAR "quarantine_clear"
#define OMX_VERB_QUARANTINE_CLEAR_FMT "quarantine_clear %s"
#define OMX_VERB_QUARANTINE_CLEAR_REPLY "resp 0"
#define OMX_VERB_QUARANTINE_CLEAR_CODES_COUNT 3u
#define OMX_VERB_QUARANTINE_CLEAR_CODES_INIT { -3, -901, -902 } /* reply.host.instance-non-exists, reply.host.memory-allocation, reply.host.invalid-operation */

/* PLUGIN_HOSTD_PROTOCOL_DAEMON_VERBS.policySet (contract.d/plugin-hostd-protocol/plugin-hostd-protocol.json): The placement of an add that says default: <format | *> own | pool:<name>. */
#define OMX_VERB_POLICY_SET "policy_set"
#define OMX_VERB_POLICY_SET_FMT "policy_set %s %s"
#define OMX_VERB_POLICY_SET_REPLY "resp 0"
#define OMX_VERB_POLICY_SET_CODES_COUNT 3u
#define OMX_VERB_POLICY_SET_CODES_INIT { -501, -901, -902 } /* reply.plugin-hostd.placement-invalid, reply.host.memory-allocation, reply.host.invalid-operation */

/* PLUGIN_HOSTD_PROTOCOL_DAEMON_VERBS.poolConfig (contract.d/plugin-hostd-protocol/plugin-hostd-protocol.json): The instances a pool holds before a sibling opens: <name> <max_instances>. */
#define OMX_VERB_POOL_CONFIG "pool_config"
#define OMX_VERB_POOL_CONFIG_FMT "pool_config %s %i"
#define OMX_VERB_POOL_CONFIG_REPLY "resp 0"
#define OMX_VERB_POOL_CONFIG_CODES_COUNT 3u
#define OMX_VERB_POOL_CONFIG_CODES_INIT { -501, -901, -902 } /* reply.plugin-hostd.placement-invalid, reply.host.memory-allocation, reply.host.invalid-operation */

/* PLUGIN_HOSTD_PROTOCOL_DAEMON_VERBS.workerEnv (contract.d/plugin-hostd-protocol/plugin-hostd-protocol.json): The cpu list and the nice value of the workers of a format, at spawn and on the ones running: <format | *> <cpu-list | -> <nice | ->. */
#define OMX_VERB_WORKER_ENV "worker_env"
#define OMX_VERB_WORKER_ENV_FMT "worker_env %s %s %s"
#define OMX_VERB_WORKER_ENV_REPLY "resp 0"
#define OMX_VERB_WORKER_ENV_CODES_COUNT 2u
#define OMX_VERB_WORKER_ENV_CODES_INIT { -901, -902 } /* reply.host.memory-allocation, reply.host.invalid-operation */

/* PLUGIN_HOSTD_PROTOCOL_DAEMON_VERBS.pinSet (contract.d/plugin-hostd-protocol/plugin-hostd-protocol.json): The pin of one plugin: <uri> <path>=<sha256>[,<path>=<sha256>...] <scheme>:<sha256> [verdict=<verdict>]. */
#define OMX_VERB_PIN_SET "pin_set"
#define OMX_VERB_PIN_SET_FMT "pin_set %s %s %s ..."
#define OMX_VERB_PIN_SET_REPLY "resp 0"
#define OMX_VERB_PIN_SET_CODES_COUNT 3u
#define OMX_VERB_PIN_SET_CODES_INIT { -516, -901, -902 } /* reply.plugin-hostd.pin-refused, reply.host.memory-allocation, reply.host.invalid-operation */

/* PLUGIN_HOSTD_PROTOCOL_DAEMON_VERBS.pinClear (contract.d/plugin-hostd-protocol/plugin-hostd-protocol.json): Forget the pin of one plugin, or of every one: <uri> | all. */
#define OMX_VERB_PIN_CLEAR "pin_clear"
#define OMX_VERB_PIN_CLEAR_FMT "pin_clear %s"
#define OMX_VERB_PIN_CLEAR_REPLY "resp 0"
#define OMX_VERB_PIN_CLEAR_CODES_COUNT 4u
#define OMX_VERB_PIN_CLEAR_CODES_INIT { -901, -902, -540, -541 } /* reply.host.memory-allocation, reply.host.invalid-operation, reply.plugin-hostd.durable-write-failed, reply.plugin-hostd.writer-queue-full */

/* PLUGIN_HOSTD_PROTOCOL_DAEMON_VERBS.pinReload (contract.d/plugin-hostd-protocol/plugin-hostd-protocol.json): Read the qualification store and the pins_dir files again and put the pins in at once; running instances keep running. */
#define OMX_VERB_PIN_RELOAD "pin_reload"
#define OMX_VERB_PIN_RELOAD_FMT "pin_reload"
#define OMX_VERB_PIN_RELOAD_REPLY "resp 0"
#define OMX_VERB_PIN_RELOAD_CODES_COUNT 1u
#define OMX_VERB_PIN_RELOAD_CODES_INIT { -902 } /* reply.host.invalid-operation */

/* PLUGIN_HOSTD_PROTOCOL_DAEMON_VERBS.pinChanged (contract.d/plugin-hostd-protocol/plugin-hostd-protocol.json): The sync's notice that the pin file was rewritten: the same as pin_reload, with or without the <uri> [files|layout] of the pin_changed event. */
#define OMX_VERB_PIN_CHANGED "pin_changed"
#define OMX_VERB_PIN_CHANGED_FMT "pin_changed ..."
#define OMX_VERB_PIN_CHANGED_REPLY "resp 0"
#define OMX_VERB_PIN_CHANGED_CODES_COUNT 1u
#define OMX_VERB_PIN_CHANGED_CODES_INIT { -902 } /* reply.host.invalid-operation */

/* PLUGIN_HOSTD_PROTOCOL_DAEMON_VERBS.pinAccept (contract.d/plugin-hostd-protocol/plugin-hostd-protocol.json): Under pin_mode first-use, take the plugin's files as they are now as its pin: <uri>. */
#define OMX_VERB_PIN_ACCEPT "pin_accept"
#define OMX_VERB_PIN_ACCEPT_FMT "pin_accept %s"
#define OMX_VERB_PIN_ACCEPT_REPLY "resp 0"
#define OMX_VERB_PIN_ACCEPT_CODES_COUNT 7u
#define OMX_VERB_PIN_ACCEPT_CODES_INIT { -508, -509, -516, -901, -902, -540, -541 } /* reply.host.pin-absent, reply.plugin-hostd.pin-binary-mismatch, reply.plugin-hostd.pin-refused, reply.host.memory-allocation, reply.host.invalid-operation, reply.plugin-hostd.durable-write-failed, reply.plugin-hostd.writer-queue-full */

/* PLUGIN_HOSTD_PROTOCOL_DAEMON_VERBS.pinLayout (contract.d/plugin-hostd-protocol/plugin-hostd-protocol.json): Sent by the daemon to a worker that checks layouts, after it accepted an add whose layout pin is unknown: the layout the worker found after init. */
#define OMX_VERB_PIN_LAYOUT "pin_layout"
#define OMX_VERB_PIN_LAYOUT_FMT "pin_layout %i"
#define OMX_VERB_PIN_LAYOUT_REPLY "resp 0 <scheme>:<sha256>"

/* PLUGIN_HOSTD_PROTOCOL_DAEMON_VERBS.lv2WorkerSet (contract.d/plugin-hostd-protocol/plugin-hostd-protocol.json): The worker of one LV2 plugin, for its adds from now on: <uri> <mod-host | omx-clap-host | ->. */
#define OMX_VERB_LV2_WORKER_SET "lv2_worker_set"
#define OMX_VERB_LV2_WORKER_SET_FMT "lv2_worker_set %s %s"
#define OMX_VERB_LV2_WORKER_SET_REPLY "resp 0"
#define OMX_VERB_LV2_WORKER_SET_CODES_COUNT 2u
#define OMX_VERB_LV2_WORKER_SET_CODES_INIT { -901, -902 } /* reply.host.memory-allocation, reply.host.invalid-operation */

/* PLUGIN_HOSTD_PROTOCOL_DAEMON_VERBS.chain (contract.d/plugin-hostd-protocol/plugin-hostd-protocol.json): The instances, in this order, are one chain run in one worker: <chain> <instance> [<instance> ...]; a worker that runs chains in one client is sent the line with the members it holds. */
#define OMX_VERB_CHAIN "chain"
#define OMX_VERB_CHAIN_FMT "chain %i %i ..."
#define OMX_VERB_CHAIN_REPLY "resp 0"
#define OMX_VERB_CHAIN_CODES_COUNT 7u
#define OMX_VERB_CHAIN_CODES_INIT { -518, -519, -520, -521, -501, -901, -902 } /* reply.plugin-hostd.chain-unknown-instance, reply.plugin-hostd.chain-duplicate, reply.plugin-hostd.chain-instance-busy, reply.plugin-hostd.chain-mixed-format, reply.plugin-hostd.placement-invalid, reply.host.memory-allocation, reply.host.invalid-operation */

/* PLUGIN_HOSTD_PROTOCOL_DAEMON_VERBS.chainClear (contract.d/plugin-hostd-protocol/plugin-hostd-protocol.json): Forget the chain: each member goes to a worker of its own. */
#define OMX_VERB_CHAIN_CLEAR "chain_clear"
#define OMX_VERB_CHAIN_CLEAR_FMT "chain_clear %i"
#define OMX_VERB_CHAIN_CLEAR_REPLY "resp 0"
#define OMX_VERB_CHAIN_CLEAR_CODES_COUNT 3u
#define OMX_VERB_CHAIN_CLEAR_CODES_INIT { -522, -901, -902 } /* reply.plugin-hostd.no-such-chain, reply.host.memory-allocation, reply.host.invalid-operation */

/* PLUGIN_HOSTD_PROTOCOL_DAEMON_VERBS.chainGet (contract.d/plugin-hostd-protocol/plugin-hostd-protocol.json): Read a chain back: its worker, its members in the declared order, and the members its worker runs now. */
#define OMX_VERB_CHAIN_GET "chain_get"
#define OMX_VERB_CHAIN_GET_FMT "chain_get %i"
#define OMX_VERB_CHAIN_GET_REPLY "resp 0 <worker> <instance>,... <instance>,..."
#define OMX_VERB_CHAIN_GET_CODES_COUNT 3u
#define OMX_VERB_CHAIN_GET_CODES_INIT { -522, -901, -902 } /* reply.plugin-hostd.no-such-chain, reply.host.memory-allocation, reply.host.invalid-operation */

/* PLUGIN_HOSTD_PROTOCOL_DAEMON_VERBS.descriptorGet (contract.d/plugin-hostd-protocol/plugin-hostd-protocol.json): The scanned descriptor of the plugin, answered from descriptor_dir: <uri>. */
#define OMX_VERB_DESCRIPTOR_GET "descriptor_get"
#define OMX_VERB_DESCRIPTOR_GET_FMT "descriptor_get %s"
#define OMX_VERB_DESCRIPTOR_GET_REPLY "resp 0 \"<path>\""
#define OMX_VERB_DESCRIPTOR_GET_CODES_COUNT 6u
#define OMX_VERB_DESCRIPTOR_GET_CODES_INIT { -528, -529, -530, -531, -901, -902 } /* reply.plugin-hostd.dir-not-writable, reply.plugin-hostd.descriptor-absent, reply.plugin-hostd.descriptor-pending, reply.plugin-hostd.descriptor-scan-failed, reply.host.memory-allocation, reply.host.invalid-operation */

/* PLUGIN_HOSTD_PROTOCOL_DAEMON_VERBS.panelGet (contract.d/plugin-hostd-protocol/plugin-hostd-protocol.json): The panel the plugin publishes, answered from descriptor_dir: <uri>. */
#define OMX_VERB_PANEL_GET "panel_get"
#define OMX_VERB_PANEL_GET_FMT "panel_get %s"
#define OMX_VERB_PANEL_GET_REPLY "resp 0 \"<path>\""
#define OMX_VERB_PANEL_GET_CODES_COUNT 6u
#define OMX_VERB_PANEL_GET_CODES_INIT { -528, -529, -530, -531, -901, -902 } /* reply.plugin-hostd.dir-not-writable, reply.plugin-hostd.descriptor-absent, reply.plugin-hostd.descriptor-pending, reply.plugin-hostd.descriptor-scan-failed, reply.host.memory-allocation, reply.host.invalid-operation */

/* PLUGIN_HOSTD_PROTOCOL_DAEMON_VERBS.presetList (contract.d/plugin-hostd-protocol/plugin-hostd-protocol.json): The plugin's presets, the ones its preset discovery or its bundle lists. */
#define OMX_VERB_PRESET_LIST "preset_list"
#define OMX_VERB_PRESET_LIST_FMT "preset_list %i"
#define OMX_VERB_PRESET_LIST_REPLY "resp <count>"
#define OMX_VERB_PRESET_LIST_CODES_COUNT 4u
#define OMX_VERB_PRESET_LIST_CODES_INIT { -515, -3, -901, -902 } /* reply.plugin-hostd.no-preset-list, reply.host.instance-non-exists, reply.host.memory-allocation, reply.host.invalid-operation */

/* PLUGIN_HOSTD_PROTOCOL_DAEMON_VERBS.presetGet (contract.d/plugin-hostd-protocol/plugin-hostd-protocol.json): One preset, 0 to count - 1: the key preset_load takes for it, and its name. */
#define OMX_VERB_PRESET_GET "preset_get"
#define OMX_VERB_PRESET_GET_FMT "preset_get %i %i"
#define OMX_VERB_PRESET_GET_REPLY "resp 0 <load_key> <name>"
#define OMX_VERB_PRESET_GET_CODES_COUNT 4u
#define OMX_VERB_PRESET_GET_CODES_INIT { -515, -3, -901, -902 } /* reply.plugin-hostd.no-preset-list, reply.host.instance-non-exists, reply.host.memory-allocation, reply.host.invalid-operation */

/* PLUGIN_HOSTD_PROTOCOL_DAEMON_VERBS.presetShow (contract.d/plugin-hostd-protocol/plugin-hostd-protocol.json): What a load of the preset would apply, on one line of JSON: the preset file's document with each state body's bytes replaced by its decoded size, or for a listed preset its name, loadKey and location. The instance form; mod-host's preset_show <uri> is its own verb. */
#define OMX_VERB_PRESET_SHOW "preset_show"
#define OMX_VERB_PRESET_SHOW_FMT "preset_show %i %s"
#define OMX_VERB_PRESET_SHOW_REPLY "resp 0 <json>"
#define OMX_VERB_PRESET_SHOW_CODES_COUNT 5u
#define OMX_VERB_PRESET_SHOW_CODES_INIT { -523, -524, -3, -901, -902 } /* reply.plugin-hostd.preset-not-offered, reply.plugin-hostd.preset-invalid, reply.host.instance-non-exists, reply.host.memory-allocation, reply.host.invalid-operation */

/* PLUGIN_HOSTD_PROTOCOL_DAEMON_VERBS.historyGet (contract.d/plugin-hostd-protocol/plugin-hostd-protocol.json): What the daemon kept of one plugin on this host class, for the binary its pin holds now: <uri>. The history is one word, digest=<sha256>,class=<host class>,crashed=<0|1>,raises_xruns=<0|1> and then every key=value of PLUGIN_HOSTD_HISTORY_KEYS, joined by commas; - when the plugin has no record or no pin, unavailable while the history file is blocked. */
#define OMX_VERB_HISTORY_GET "history_get"
#define OMX_VERB_HISTORY_GET_FMT "history_get %s"
#define OMX_VERB_HISTORY_GET_REPLY "resp 0 <history>"
#define OMX_VERB_HISTORY_GET_CODES_COUNT 2u
#define OMX_VERB_HISTORY_GET_CODES_INIT { -901, -902 } /* reply.host.memory-allocation, reply.host.invalid-operation */

/* PLUGIN_HOSTD_PROTOCOL_DAEMON_VERBS.historyClear (contract.d/plugin-hostd-protocol/plugin-hostd-protocol.json): Delete the history of one plugin, every binary and host class, or all of it: <uri> | all. Refused while the history file is blocked. */
#define OMX_VERB_HISTORY_CLEAR "history_clear"
#define OMX_VERB_HISTORY_CLEAR_FMT "history_clear %s"
#define OMX_VERB_HISTORY_CLEAR_REPLY "resp 0"
#define OMX_VERB_HISTORY_CLEAR_CODES_COUNT 4u
#define OMX_VERB_HISTORY_CLEAR_CODES_INIT { -901, -902, -540, -541 } /* reply.host.memory-allocation, reply.host.invalid-operation, reply.plugin-hostd.durable-write-failed, reply.plugin-hostd.writer-queue-full */

/* PLUGIN_HOSTD_PROTOCOL_DAEMON_VERBS.plan (contract.d/plugin-hostd-protocol/plugin-hostd-protocol.json): The instances, in signal order, are planned into the fewest workers their pins and costs allow: <chain> <instance> [<instance> ...]. Each run of consecutive members that may share a worker becomes a chain of place chain:<chain>.<k>, the others stay alone in a worker of their own; the reply names the segments in order, the members of a segment joined by commas and the segments by semicolons, then the same with each member replaced by -, the core field, which the setting placement leaves empty because it pins a worker to the whole set of audio cores and not to one core. With the setting grouping off the plan is answered and kept, and nothing moves. */
#define OMX_VERB_PLAN "plan"
#define OMX_VERB_PLAN_FMT "plan %i %i ..."
#define OMX_VERB_PLAN_REPLY "resp 0 <instance>,...;<instance>,...;... <core>,...;<core>,...;..."
#define OMX_VERB_PLAN_CODES_COUNT 6u
#define OMX_VERB_PLAN_CODES_INIT { -518, -519, -520, -501, -901, -902 } /* reply.plugin-hostd.chain-unknown-instance, reply.plugin-hostd.chain-duplicate, reply.plugin-hostd.chain-instance-busy, reply.plugin-hostd.placement-invalid, reply.host.memory-allocation, reply.host.invalid-operation */

/* PLUGIN_HOSTD_PROTOCOL_DAEMON_VERBS.planGet (contract.d/plugin-hostd-protocol/plugin-hostd-protocol.json): Read the last plan of a chain back: its segments as plan answered them, then the reason of each member that has one (a word of PLUGIN_HOSTD_GROUPING_REASONS, plan-refused followed by the refusal's number), instance:reason joined by commas, - for none, then the cores of the members as plan answers them. */
#define OMX_VERB_PLAN_GET "plan_get"
#define OMX_VERB_PLAN_GET_FMT "plan_get %i"
#define OMX_VERB_PLAN_GET_REPLY "resp 0 <instance>,...;<instance>,...;... <instance>:<reason>,... <core>,...;<core>,...;..."
#define OMX_VERB_PLAN_GET_CODES_COUNT 3u
#define OMX_VERB_PLAN_GET_CODES_INIT { -532, -901, -902 } /* reply.plugin-hostd.no-such-plan, reply.host.memory-allocation, reply.host.invalid-operation */

/* PLUGIN_HOSTD_PROTOCOL_DAEMON_VERBS.planClear (contract.d/plugin-hostd-protocol/plugin-hostd-protocol.json): Forget the plan: each member goes to a worker of its own, as chain_clear does. */
#define OMX_VERB_PLAN_CLEAR "plan_clear"
#define OMX_VERB_PLAN_CLEAR_FMT "plan_clear %i"
#define OMX_VERB_PLAN_CLEAR_REPLY "resp 0"
#define OMX_VERB_PLAN_CLEAR_CODES_COUNT 3u
#define OMX_VERB_PLAN_CLEAR_CODES_INIT { -532, -901, -902 } /* reply.plugin-hostd.no-such-plan, reply.host.memory-allocation, reply.host.invalid-operation */

/* PLUGIN_HOSTD_PROTOCOL_DAEMON_VERBS.estimate (contract.d/plugin-hostd-protocol/plugin-hostd-protocol.json): What a chain of these plugins would cost at a rate and a quantum, read from the qualification records with no plugin loaded: <rate> <quantum> <uri> [<uri> ...], the uris as add takes them, in signal order. The reply is the latency in frames (each member's measured latency, else its declared one, added up), the CPU as percent of one core's cycle (each member's median nanoseconds per sample times the rate, added up), the RAM in bytes (the Pss each member's worker had running alone on this host class, added up; - unless every member has one), the workers plan would place the members in, and how many members have no figure at that rate and quantum; then each of those as its position in the line, from 0, and its code (noRecord, noCell or unknownPlugin), joined by commas, - for none. The latency and the CPU add up only the members that have a figure. Then the memory: what the workers hold now in bytes (- when one cannot be read), what the members would add in bytes from the qualification records, what instances of one plugin share counted once in a segment (- when no member has a figure), the host's ram_budget_mb in bytes (- when none is set), and how many members have no such figure. Last the host's insert verdict on the last uri, the candidate, added to the chain the others make: a word of ESTIMATE_VERDICTS and the word of ESTIMATE_REASONS that gave it (the domain estimate), the first that holds in this order: the candidate's unknownPlugin refuses, its noRecord or noCell is not measured; the CPU over the real-time admission ceiling of one core, the memory used and added over the budget, another member's code, a memory figure missing each warns; else recommend, within. A consumer shows the two words and computes no verdict. */
#define OMX_VERB_ESTIMATE "estimate"
#define OMX_VERB_ESTIMATE_FMT "estimate %i %i %s ..."
#define OMX_VERB_ESTIMATE_REPLY "resp 0 <latency_frames> <cpu_percent> <ram_bytes> <workers> <unknown_count> <member>:<code>,... <ram_used_bytes> <ram_added_bytes> <ram_budget_bytes> <ram_unknown_count> <verdict> <reason>"
#define OMX_VERB_ESTIMATE_CODES_COUNT 5u
#define OMX_VERB_ESTIMATE_CODES_INIT { -533, -534, -535, -901, -902 } /* reply.plugin-hostd.no-record, reply.plugin-hostd.no-cell, reply.plugin-hostd.unknown-plugin, reply.host.memory-allocation, reply.host.invalid-operation */

/* PLUGIN_HOSTD_PROTOCOL_DAEMON_VERBS.nodeLabel (contract.d/plugin-hostd-protocol/plugin-hostd-protocol.json): Labels a hosted node for graph clients: <instance> [<label word> ...], answered resp 0. The label is the rest of the line, its words joined by one space, and the node's pretty-name becomes the labelled form of node_metadata (<label>: <plugin name> (<instance>)); no words clears the label, and the pretty-name is the plain form again. The node's client name stays effect_<instance>. The label lasts as long as the instance: plugin-hostd keeps it and sets it again on a respawned worker, and remove ends it. A label over PHD_NODE_LABEL_MAX bytes or not UTF-8 is refused (-545) and the node keeps its label; an instance with no node is refused (-3); a host that does not serve the verb answers invalid operation (plugin-hostd#35). */
#define OMX_VERB_NODE_LABEL "node_label"
#define OMX_VERB_NODE_LABEL_FMT "node_label %i ..."
#define OMX_VERB_NODE_LABEL_REPLY "resp 0"
#define OMX_VERB_NODE_LABEL_CODES_COUNT 3u
#define OMX_VERB_NODE_LABEL_CODES_INIT { -545, -3, -902 } /* reply.plugin-hostd.label-invalid, reply.host.instance-non-exists, reply.host.invalid-operation */

/* PLUGIN_HOSTD_PROTOCOL_DAEMON_VERBS.descriptionGet (contract.d/plugin-hostd-protocol/plugin-hostd-protocol.json): The key a client reads the plugin's description by in the descriptions store (omx-panel docs/descriptions-store.md): <uri>, answered <sha256> <format> <uri> <layer>, the layer that holds it; -529 when no layer holds it, -530 while its ask runs. */
#define OMX_VERB_DESCRIPTION_GET "description_get"
#define OMX_VERB_DESCRIPTION_GET_FMT "description_get %s"
#define OMX_VERB_DESCRIPTION_GET_REPLY "resp 0 <sha256> <format> <uri> <layer>"
#define OMX_VERB_DESCRIPTION_GET_CODES_COUNT 4u
#define OMX_VERB_DESCRIPTION_GET_CODES_INIT { -529, -530, -901, -902 } /* reply.plugin-hostd.descriptor-absent, reply.plugin-hostd.descriptor-pending, reply.host.memory-allocation, reply.host.invalid-operation */

/* PLUGIN_HOSTD_PROTOCOL_LIBRARY_VERBS.hello (contract.d/plugin-hostd-protocol/plugin-hostd-protocol.json): Open a session: <version> [<extension> ...], the protocol version the client speaks and the extensions it asks for. The reply is the daemon's protocol version and, of the extensions asked for, the ones it grants, in the order of PLUGIN_HOSTD_PROTOCOL_EXTENSIONS; an extension the daemon does not know is not granted and is no error. Each hello replaces what the one before it granted, and a new connection starts with none. -902 for a version that is not a whole number of 1 or more, and nothing changes. */
#define OMX_VERB_HELLO "hello"
#define OMX_VERB_HELLO_FMT "hello %i ..."
#define OMX_VERB_HELLO_REPLY "resp 0 <version> <extension> ..."
#define OMX_VERB_HELLO_CODES_COUNT 1u
#define OMX_VERB_HELLO_CODES_INIT { -902 } /* reply.host.invalid-operation */

/* PLUGIN_HOSTD_PROTOCOL_LIBRARY_VERBS.paramGetMany (contract.d/plugin-hostd-protocol/plugin-hostd-protocol.json): The values of several parameters of one instance in one line: <instance> <symbol> [<symbol> ...], each value as param_get writes it, in the order asked. A symbol the instance does not have refuses the whole line with param_get's code (-103), an instance that does not exist with -3, and no value is answered then. */
#define OMX_VERB_PARAM_GET_MANY "param_get_many"
#define OMX_VERB_PARAM_GET_MANY_FMT "param_get_many %i %s ..."
#define OMX_VERB_PARAM_GET_MANY_REPLY "resp 0 <value> ..."
#define OMX_VERB_PARAM_GET_MANY_CODES_COUNT 4u
#define OMX_VERB_PARAM_GET_MANY_CODES_INIT { -3, -103, -901, -902 } /* reply.host.instance-non-exists, reply.host.param-symbol-invalid, reply.host.memory-allocation, reply.host.invalid-operation */

/* PLUGIN_HOSTD_PROTOCOL_LIBRARY_VERBS.paramSetMany (contract.d/plugin-hostd-protocol/plugin-hostd-protocol.json): Several parameters of one instance in one line, all or nothing: <instance> <symbol> <value> [<symbol> <value> ...]. Every pair is checked before any is set: a symbol the instance does not have (-103), an instance that does not exist (-3), a value that is not a number or a pair with no value (-902). The first pair refused refuses the line, answered resp <code> <position> with the pair's position from 1, and no value is set. Should the host refuse a value as it sets it, the values set before it are set back, and the line is refused the same way. */
#define OMX_VERB_PARAM_SET_MANY "param_set_many"
#define OMX_VERB_PARAM_SET_MANY_FMT "param_set_many %i %s %f ..."
#define OMX_VERB_PARAM_SET_MANY_REPLY "resp 0"
#define OMX_VERB_PARAM_SET_MANY_CODES_COUNT 4u
#define OMX_VERB_PARAM_SET_MANY_CODES_INIT { -3, -103, -901, -902 } /* reply.host.instance-non-exists, reply.host.param-symbol-invalid, reply.host.memory-allocation, reply.host.invalid-operation */

/* PLUGIN_HOSTD_PROTOCOL_LIBRARY_VERBS.curveBind (contract.d/plugin-hostd-protocol/plugin-hostd-protocol.json): Bind a display of an instance's panel description to the instance's parameters, for curve_points: <instance> <display> clear, or <instance> <display> <kind> <family> <term> [<term> ...], the display's index in the description, its kind (eq_response, transfer_curve or time_response) and family (eq for an eq_response; delay or reverb for a time_response; any other for a transfer_curve), and each term <role>[.<index>]=<symbol>[:<unit>][@<shape>|<shape>...] as the description gives the role: the parameter that plays it, its unit, for a band_type the band shape of each choice value from 0 (a band_freq takes one shape, the band's fixed shape), and for a head filter's slope the dB per octave of each choice value from 0 (@12|24), the role's declared enumeration. The terms are added to the display's binding, a term of a role and index already bound replacing it; a line of another kind or family starts the binding again, and clear drops it. Every term is checked before any is kept: an instance that does not exist (-3), a symbol it does not have (-103), a family the kind is not drawn for (-536), a unit the role does not take (-538), a term the grammar does not take (-902). The binding lives with the instance and goes with its remove; it is never saved. */
#define OMX_VERB_CURVE_BIND "curve_bind"
#define OMX_VERB_CURVE_BIND_FMT "curve_bind %i %i %s ..."
#define OMX_VERB_CURVE_BIND_REPLY "resp 0"
#define OMX_VERB_CURVE_BIND_CODES_COUNT 6u
#define OMX_VERB_CURVE_BIND_CODES_INIT { -3, -103, -536, -538, -901, -902 } /* reply.host.instance-non-exists, reply.host.param-symbol-invalid, reply.plugin-hostd.curve-kind-unknown, reply.plugin-hostd.curve-unit-refused, reply.host.memory-allocation, reply.host.invalid-operation */

/* PLUGIN_HOSTD_PROTOCOL_LIBRARY_VERBS.curvePoints (contract.d/plugin-hostd-protocol/plugin-hostd-protocol.json): The points of a bound display: <instance> <display> <points>, the grid points asked for. Each bound symbol's value is read from the instance as it is now and the instance's rate with it, and omx-dsp's omx_curve_points_marked draws them: an eq_response x in Hz and y in dB, the bands' anchors added (three a drawn band), a transfer_curve input dB to output dB, a time_response of a reverb its decay envelope, time in ms to level in dB (no point at mix 0), and of a delay its taps. The reply is the count, the marks (one word) and each point x,y, x to four significant digits and y to 0.01 dB, ascending in x; at most curve_points_max (omx-dsp's CURVE_POINTS_MAX, 256) points. A delay has no point but taps, which follow the count as `stems <count> <t>,<dB>,<shade>,<leg> ...`: each tap's time in ms (four significant digits), its level in dB relative to the impulse (0.01), its brightness 0..1 (0.001) and the leg it lands on (both, left or right, omx-dsp's CURVE_LEGS), in time order, the dry tap first. The marks are each drawn eq_response band whose origin is not user, `<freq>,<depth>,<q>,<origin>`, its frequency and depth at the same precision as a point, its Q to 0.01 and its origin, the word omx-dsp's OMX_CURVE_ORIGIN_WORDS spells for it (CURVE_ORIGINS, omx-dsp's curve domain), joined by `;`, in band order; `-` when there is none, as for a host that gives every bound term the origin user, a transfer_curve and a time_response. The origin comes from the host's backend alone. A delay whose sync is on needs the host's tempo, which this library is not given: it is -537, never a guessed tempo. A display with no binding, or a binding without a term the model needs (-537); a band shape or slope, a reverb algorithm or a delay division with no closed form (-536); points outside 2 to curve_points_max, the anchors counted (-539); a value that is not finite or an instance not running at a rate (-542). */
#define OMX_VERB_CURVE_POINTS "curve_points"
#define OMX_VERB_CURVE_POINTS_FMT "curve_points %i %i %i"
#define OMX_VERB_CURVE_POINTS_REPLY "resp 0 <count> <marks> <x>,<y> ..."
#define OMX_VERB_CURVE_POINTS_CODES_COUNT 8u
#define OMX_VERB_CURVE_POINTS_CODES_INIT { -3, -103, -536, -537, -539, -542, -901, -902 } /* reply.host.instance-non-exists, reply.host.param-symbol-invalid, reply.plugin-hostd.curve-kind-unknown, reply.plugin-hostd.curve-term-missing, reply.plugin-hostd.curve-points-range, reply.plugin-hostd.curve-value-not-finite, reply.host.memory-allocation, reply.host.invalid-operation */

/* PLUGIN_HOSTD_PROTOCOL_LIBRARY_VERBS.monitorOutputOff (contract.d/plugin-hostd-protocol/plugin-hostd-protocol.json): Ends the monitor monitor_output set on an output: <instance> <symbol>, answered resp 0, and the output's feedback lines stop. A monitor ends with the connection that set it, so a controller that disconnects leaves none behind, and only the connection that set a monitor ends it earlier: an output this connection did not monitor (never monitored, monitored by an earlier connection, already ended, or its instance removed) is refused (-543), and a line that is not an instance and a symbol, or a host that does not serve the verb, is refused (-902). The output's ledger line goes with the monitor, so a respawn of the instance does not replay it. */
#define OMX_VERB_MONITOR_OUTPUT_OFF "monitor_output_off"
#define OMX_VERB_MONITOR_OUTPUT_OFF_FMT "monitor_output_off %i %s"
#define OMX_VERB_MONITOR_OUTPUT_OFF_REPLY "resp 0"
#define OMX_VERB_MONITOR_OUTPUT_OFF_CODES_COUNT 2u
#define OMX_VERB_MONITOR_OUTPUT_OFF_CODES_INIT { -543, -902 } /* reply.plugin-hostd.output-not-monitored, reply.host.invalid-operation */

/* PLUGIN_HOSTD_PROTOCOL_SHARED_VERBS.trackInfo (contract.d/plugin-hostd-protocol/plugin-hostd-protocol.json): The strip's name, colour and kind: <instance> <name> <#RRGGBB | -> [bus | return | master]; a string word is at most 255 bytes of UTF-8 with no control character, and any other word is -902. A CLAP plugin reads them through clap.track-info. */
#define OMX_VERB_TRACK_INFO "track_info"
#define OMX_VERB_TRACK_INFO_FMT "track_info %i %s %s ..."
#define OMX_VERB_TRACK_INFO_REPLY "resp 0"
#define OMX_VERB_TRACK_INFO_CODES_COUNT 3u
#define OMX_VERB_TRACK_INFO_CODES_INIT { -902, -3, -901 } /* reply.host.invalid-operation, reply.host.instance-non-exists, reply.host.memory-allocation */

/* PLUGIN_HOSTD_PROTOCOL_SHARED_VERBS.remotePages (contract.d/plugin-hostd-protocol/plugin-hostd-protocol.json): The plugin's remote-control pages, from clap.remote-controls; 0 for a plugin that has none. */
#define OMX_VERB_REMOTE_PAGES "remote_pages"
#define OMX_VERB_REMOTE_PAGES_FMT "remote_pages %i"
#define OMX_VERB_REMOTE_PAGES_REPLY "resp <count>"
#define OMX_VERB_REMOTE_PAGES_CODES_COUNT 3u
#define OMX_VERB_REMOTE_PAGES_CODES_INIT { -902, -3, -901 } /* reply.host.invalid-operation, reply.host.instance-non-exists, reply.host.memory-allocation */

/* PLUGIN_HOSTD_PROTOCOL_SHARED_VERBS.remotePageGet (contract.d/plugin-hostd-protocol/plugin-hostd-protocol.json): One page, 0 to count - 1: its section and name as quoted words, and each slot the symbol param_set takes, - for an empty one; -902 for a page that is not one. */
#define OMX_VERB_REMOTE_PAGE_GET "remote_page_get"
#define OMX_VERB_REMOTE_PAGE_GET_FMT "remote_page_get %i %i"
#define OMX_VERB_REMOTE_PAGE_GET_REPLY "resp 0 <page_id> <section> <page_name> <s1> <s2> <s3> <s4> <s5> <s6> <s7> <s8>"
#define OMX_VERB_REMOTE_PAGE_GET_CODES_COUNT 3u
#define OMX_VERB_REMOTE_PAGE_GET_CODES_INIT { -902, -3, -901 } /* reply.host.invalid-operation, reply.host.instance-non-exists, reply.host.memory-allocation */

/* PLUGIN_HOSTD_PROTOCOL_SHARED_VERBS.paramInfo (contract.d/plugin-hostd-protocol/plugin-hostd-protocol.json): What a parameter's value means: its unit, its scale (linear, log or stepped), its range, default and step, and the symbol it keeps across hosts and versions. */
#define OMX_VERB_PARAM_INFO "param_info"
#define OMX_VERB_PARAM_INFO_FMT "param_info %i %s"
#define OMX_VERB_PARAM_INFO_REPLY "resp 0 <unit> <scale> <min> <max> <default> <step> <stable_symbol>"
#define OMX_VERB_PARAM_INFO_CODES_COUNT 5u
#define OMX_VERB_PARAM_INFO_CODES_INIT { -103, -511, -3, -902, -901 } /* reply.host.param-symbol-invalid, reply.host.no-param-contract, reply.host.instance-non-exists, reply.host.invalid-operation, reply.host.memory-allocation */

/* PLUGIN_HOSTD_PROTOCOL_SHARED_VERBS.pinExpect (contract.d/plugin-hostd-protocol/plugin-hostd-protocol.json): The layout pin <scheme>:<sha256> the next add of the instance checks after init and before activate; plugin-hostd sends it to a worker of a format that checks layouts, and refuses the add with any other answer than resp 0. */
#define OMX_VERB_PIN_EXPECT "pin_expect"
#define OMX_VERB_PIN_EXPECT_FMT "pin_expect %i %s"
#define OMX_VERB_PIN_EXPECT_REPLY "resp 0"
#define OMX_VERB_PIN_EXPECT_CODES_COUNT 3u
#define OMX_VERB_PIN_EXPECT_CODES_INIT { -1, -902, -508 } /* reply.host.instance-invalid, reply.host.invalid-operation, reply.host.pin-absent */

/* PLUGIN_HOSTD_PROTOCOL_SHARED_VERBS.chainExpect (contract.d/plugin-hostd-protocol/plugin-hostd-protocol.json): The chain the next add of the instance joins, at its end, in the head's client: no client of its own is opened or activated. That add answers invalidOperation, and nothing of the plugin stays, when the host runs no such chain or the plugin is not as wide as the head's output; plugin-hostd sends it for each member after a chain's first, and a worker that refuses the verb gets the add and the chain line instead. */
#define OMX_VERB_CHAIN_EXPECT "chain_expect"
#define OMX_VERB_CHAIN_EXPECT_FMT "chain_expect %i %i"
#define OMX_VERB_CHAIN_EXPECT_REPLY "resp 0"
#define OMX_VERB_CHAIN_EXPECT_CODES_COUNT 2u
#define OMX_VERB_CHAIN_EXPECT_CODES_INIT { -1, -902 } /* reply.host.instance-invalid, reply.host.invalid-operation */

/* PLUGIN_HOSTD_PROTOCOL_SHARED_VERBS.noteInput (contract.d/plugin-hostd-protocol/plugin-hostd-protocol.json): Whether the instance takes notes: the name of its note input on its worker's jack client, - for a plugin that takes none. */
#define OMX_VERB_NOTE_INPUT "note_input"
#define OMX_VERB_NOTE_INPUT_FMT "note_input %i"
#define OMX_VERB_NOTE_INPUT_REPLY "resp 0 <port>"
#define OMX_VERB_NOTE_INPUT_CODES_COUNT 4u
#define OMX_VERB_NOTE_INPUT_CODES_INIT { -3, -902, -514, -901 } /* reply.host.instance-non-exists, reply.host.invalid-operation, reply.host.notes-by-jack, reply.host.memory-allocation */

/* PLUGIN_HOSTD_PROTOCOL_SHARED_VERBS.noteOn (contract.d/plugin-hostd-protocol/plugin-hostd-protocol.json): Play a key: <instance> <channel> <key> <velocity>, channel 0 to 15, key 0 to 127, velocity 1 to 127, a MIDI note on at the start of the worker's next cycle; -902 for a word out of its range. */
#define OMX_VERB_NOTE_ON "note_on"
#define OMX_VERB_NOTE_ON_FMT "note_on %i %i %i %i"
#define OMX_VERB_NOTE_ON_REPLY "resp 0"
#define OMX_VERB_NOTE_ON_CODES_COUNT 6u
#define OMX_VERB_NOTE_ON_CODES_INIT { -513, -514, -517, -3, -902, -901 } /* reply.host.no-note-input, reply.host.notes-by-jack, reply.host.note-queue-full, reply.host.instance-non-exists, reply.host.invalid-operation, reply.host.memory-allocation */

/* PLUGIN_HOSTD_PROTOCOL_SHARED_VERBS.noteOff (contract.d/plugin-hostd-protocol/plugin-hostd-protocol.json): Release a key: <instance> <channel> <key> <velocity>, velocity 0 to 127, a MIDI note off; answered as note_on is. */
#define OMX_VERB_NOTE_OFF "note_off"
#define OMX_VERB_NOTE_OFF_FMT "note_off %i %i %i %i"
#define OMX_VERB_NOTE_OFF_REPLY "resp 0"
#define OMX_VERB_NOTE_OFF_CODES_COUNT 6u
#define OMX_VERB_NOTE_OFF_CODES_INIT { -513, -514, -517, -3, -902, -901 } /* reply.host.no-note-input, reply.host.notes-by-jack, reply.host.note-queue-full, reply.host.instance-non-exists, reply.host.invalid-operation, reply.host.memory-allocation */

/* PLUGIN_HOSTD_PROTOCOL_SHARED_VERBS.allNotesOff (contract.d/plugin-hostd-protocol/plugin-hostd-protocol.json): Release every key of every channel, MIDI's all notes off (controller 123) on each; answered as note_on is. */
#define OMX_VERB_ALL_NOTES_OFF "all_notes_off"
#define OMX_VERB_ALL_NOTES_OFF_FMT "all_notes_off %i"
#define OMX_VERB_ALL_NOTES_OFF_REPLY "resp 0"
#define OMX_VERB_ALL_NOTES_OFF_CODES_COUNT 6u
#define OMX_VERB_ALL_NOTES_OFF_CODES_INIT { -513, -514, -517, -3, -902, -901 } /* reply.host.no-note-input, reply.host.notes-by-jack, reply.host.note-queue-full, reply.host.instance-non-exists, reply.host.invalid-operation, reply.host.memory-allocation */

/* PLUGIN_HOSTD_PROTOCOL_SHARED_VERBS.propertyList (contract.d/plugin-hostd-protocol/plugin-hostd-protocol.json): The instance's file and text properties (org.openmixer.properties/1, #15): how many; 0 for a plugin that offers none or whose table broke a rule at load. */
#define OMX_VERB_PROPERTY_LIST "property_list"
#define OMX_VERB_PROPERTY_LIST_FMT "property_list %i"
#define OMX_VERB_PROPERTY_LIST_REPLY "resp <count>"
#define OMX_VERB_PROPERTY_LIST_CODES_COUNT 2u
#define OMX_VERB_PROPERTY_LIST_CODES_INIT { -902, -3 } /* reply.host.invalid-operation, reply.host.instance-non-exists */

/* PLUGIN_HOSTD_PROTOCOL_SHARED_VERBS.propertyInfo (contract.d/plugin-hostd-protocol/plugin-hostd-protocol.json): One property, 0 to count - 1: its symbol, its kind (path or string), its access (rw, w or r) and its name as a quoted-escaped word; -106 for an index that is not one. */
#define OMX_VERB_PROPERTY_INFO "property_info"
#define OMX_VERB_PROPERTY_INFO_FMT "property_info %i %i"
#define OMX_VERB_PROPERTY_INFO_REPLY "resp 0 <symbol> <kind> <access> <name>"
#define OMX_VERB_PROPERTY_INFO_CODES_COUNT 3u
#define OMX_VERB_PROPERTY_INFO_CODES_INIT { -902, -3, -106 } /* reply.host.invalid-operation, reply.host.instance-non-exists, reply.host.property-symbol-invalid */

/* PLUGIN_HOSTD_PROTOCOL_SHARED_VERBS.propertyGet (contract.d/plugin-hostd-protocol/plugin-hostd-protocol.json): The value the plugin holds now for the property of that symbol, as a quoted-escaped word ("" when unset); -107 for one that is not readable. */
#define OMX_VERB_PROPERTY_GET "property_get"
#define OMX_VERB_PROPERTY_GET_FMT "property_get %i %s"
#define OMX_VERB_PROPERTY_GET_REPLY "resp 0 <value>"
#define OMX_VERB_PROPERTY_GET_CODES_COUNT 4u
#define OMX_VERB_PROPERTY_GET_CODES_INIT { -902, -3, -106, -107 } /* reply.host.invalid-operation, reply.host.instance-non-exists, reply.host.property-symbol-invalid, reply.host.property-access */

/* PLUGIN_HOSTD_PROTOCOL_SHARED_VERBS.propertySet (contract.d/plugin-hostd-protocol/plugin-hostd-protocol.json): Set the property of that symbol to the quoted-escaped value: it reaches the plugin at its next block; -107 for one that is not writable, -108 for a value too large, not UTF-8 or a path that is not absolute, -109 when the instance's ring is full (send it again). */
#define OMX_VERB_PROPERTY_SET "property_set"
#define OMX_VERB_PROPERTY_SET_FMT "property_set %i %s %s"
#define OMX_VERB_PROPERTY_SET_REPLY "resp 0"
#define OMX_VERB_PROPERTY_SET_CODES_COUNT 6u
#define OMX_VERB_PROPERTY_SET_CODES_INIT { -902, -3, -106, -107, -108, -109 } /* reply.host.invalid-operation, reply.host.instance-non-exists, reply.host.property-symbol-invalid, reply.host.property-access, reply.host.property-value-invalid, reply.host.property-queue-full */

/* PLUGIN_HOSTD_PROTOCOL_CLAP_HOST_VERBS.audioPorts (contract.d/plugin-hostd-protocol/plugin-hostd-protocol.json): The instance's audio ports, inputs first and each side's main port first, each with the JACK ports that carry it; dry while the instance passes its input through. */
#define OMX_VERB_AUDIO_PORTS "audio_ports"
#define OMX_VERB_AUDIO_PORTS_FMT "audio_ports %i"
#define OMX_VERB_AUDIO_PORTS_REPLY "resp 0 [dry] <port> ..."
#define OMX_VERB_AUDIO_PORTS_CODES_COUNT 2u
#define OMX_VERB_AUDIO_PORTS_CODES_INIT { -3, -901 } /* reply.host.instance-non-exists, reply.host.memory-allocation */

/* PLUGIN_HOSTD_PROTOCOL_CLAP_HOST_VERBS.hideExternalUi (contract.d/plugin-hostd-protocol/plugin-hostd-protocol.json): Hide the plugin's own window that show_external_ui showed. */
#define OMX_VERB_HIDE_EXTERNAL_UI "hide_external_ui"
#define OMX_VERB_HIDE_EXTERNAL_UI_FMT "hide_external_ui %i"
#define OMX_VERB_HIDE_EXTERNAL_UI_REPLY "resp 0"
#define OMX_VERB_HIDE_EXTERNAL_UI_CODES_COUNT 2u
#define OMX_VERB_HIDE_EXTERNAL_UI_CODES_INIT { -3, -902 } /* reply.host.instance-non-exists, reply.host.invalid-operation */

/* PLUGIN_HOSTD_PROTOCOL_CLAP_HOST_VERBS.idleHold (contract.d/plugin-hostd-protocol/plugin-hostd-protocol.json): The CPU idle hold as it stands; idle_hold <off | microseconds> changes its target at once and answers resp 0. */
#define OMX_VERB_IDLE_HOLD "idle_hold"
#define OMX_VERB_IDLE_HOLD_FMT "idle_hold ..."
#define OMX_VERB_IDLE_HOLD_REPLY "resp 0 <target> <held> <aggregate> <active> <code>"
#define OMX_VERB_IDLE_HOLD_CODES_COUNT 1u
#define OMX_VERB_IDLE_HOLD_CODES_INIT { -902 } /* reply.host.invalid-operation */

/* PLUGIN_HOSTD_PROTOCOL_CLAP_HOST_VERBS.noteDrops (contract.d/plugin-hostd-protocol/plugin-hostd-protocol.json): The note events the instance's blocks had no room for, of them the releases, the sysex past a block's pool, and the messages no dialect of the plugin carries. */
#define OMX_VERB_NOTE_DROPS "note_drops"
#define OMX_VERB_NOTE_DROPS_FMT "note_drops %i"
#define OMX_VERB_NOTE_DROPS_REPLY "resp 0 <notes> <releases> <sysex> <unmappable>"
#define OMX_VERB_NOTE_DROPS_CODES_COUNT 1u
#define OMX_VERB_NOTE_DROPS_CODES_INIT { -3 } /* reply.host.instance-non-exists */

/* PLUGIN_HOSTD_PROTOCOL_CLAP_HOST_VERBS.noteOutDrops (contract.d/plugin-hostd-protocol/plugin-hostd-protocol.json): The plugin's note output counters: the events the ring had no room for, those with no MIDI form, and the messages midi_out had no room for. */
#define OMX_VERB_NOTE_OUT_DROPS "note_out_drops"
#define OMX_VERB_NOTE_OUT_DROPS_FMT "note_out_drops %i"
#define OMX_VERB_NOTE_OUT_DROPS_REPLY "resp 0 <dropped> <unmapped> <midi_out_full>"
#define OMX_VERB_NOTE_OUT_DROPS_CODES_COUNT 1u
#define OMX_VERB_NOTE_OUT_DROPS_CODES_INIT { -3 } /* reply.host.instance-non-exists */

#define PLUGIN_HOSTD_PROTOCOL_HOST_VERB_COUNT 55u

/* Every verb, in the order of its items, with what each host owes it. */
static inline const struct omx_host_verb *plugin_hostd_protocol_host_verbs(void) {
  static const struct omx_host_verb table[PLUGIN_HOSTD_PROTOCOL_HOST_VERB_COUNT] = {
    { OMX_VERB_WORKER_LIST, OMX_VERB_WORKER_LIST_FMT, OMX_VERB_WORKER_LIST_REPLY, { OMX_HOST_VERB_REQUIRED, OMX_HOST_VERB_UNSUPPORTED, OMX_HOST_VERB_UNSUPPORTED } },
    { OMX_VERB_INSTANCE_INFO, OMX_VERB_INSTANCE_INFO_FMT, OMX_VERB_INSTANCE_INFO_REPLY, { OMX_HOST_VERB_REQUIRED, OMX_HOST_VERB_UNSUPPORTED, OMX_HOST_VERB_UNSUPPORTED } },
    { OMX_VERB_SUPERVISOR_RESET, OMX_VERB_SUPERVISOR_RESET_FMT, OMX_VERB_SUPERVISOR_RESET_REPLY, { OMX_HOST_VERB_REQUIRED, OMX_HOST_VERB_UNSUPPORTED, OMX_HOST_VERB_UNSUPPORTED } },
    { OMX_VERB_QUARANTINE_CLEAR, OMX_VERB_QUARANTINE_CLEAR_FMT, OMX_VERB_QUARANTINE_CLEAR_REPLY, { OMX_HOST_VERB_REQUIRED, OMX_HOST_VERB_UNSUPPORTED, OMX_HOST_VERB_UNSUPPORTED } },
    { OMX_VERB_POLICY_SET, OMX_VERB_POLICY_SET_FMT, OMX_VERB_POLICY_SET_REPLY, { OMX_HOST_VERB_REQUIRED, OMX_HOST_VERB_UNSUPPORTED, OMX_HOST_VERB_UNSUPPORTED } },
    { OMX_VERB_POOL_CONFIG, OMX_VERB_POOL_CONFIG_FMT, OMX_VERB_POOL_CONFIG_REPLY, { OMX_HOST_VERB_REQUIRED, OMX_HOST_VERB_UNSUPPORTED, OMX_HOST_VERB_UNSUPPORTED } },
    { OMX_VERB_WORKER_ENV, OMX_VERB_WORKER_ENV_FMT, OMX_VERB_WORKER_ENV_REPLY, { OMX_HOST_VERB_REQUIRED, OMX_HOST_VERB_UNSUPPORTED, OMX_HOST_VERB_UNSUPPORTED } },
    { OMX_VERB_PIN_SET, OMX_VERB_PIN_SET_FMT, OMX_VERB_PIN_SET_REPLY, { OMX_HOST_VERB_REQUIRED, OMX_HOST_VERB_UNSUPPORTED, OMX_HOST_VERB_UNSUPPORTED } },
    { OMX_VERB_PIN_CLEAR, OMX_VERB_PIN_CLEAR_FMT, OMX_VERB_PIN_CLEAR_REPLY, { OMX_HOST_VERB_REQUIRED, OMX_HOST_VERB_UNSUPPORTED, OMX_HOST_VERB_UNSUPPORTED } },
    { OMX_VERB_PIN_RELOAD, OMX_VERB_PIN_RELOAD_FMT, OMX_VERB_PIN_RELOAD_REPLY, { OMX_HOST_VERB_REQUIRED, OMX_HOST_VERB_UNSUPPORTED, OMX_HOST_VERB_UNSUPPORTED } },
    { OMX_VERB_PIN_CHANGED, OMX_VERB_PIN_CHANGED_FMT, OMX_VERB_PIN_CHANGED_REPLY, { OMX_HOST_VERB_REQUIRED, OMX_HOST_VERB_UNSUPPORTED, OMX_HOST_VERB_UNSUPPORTED } },
    { OMX_VERB_PIN_ACCEPT, OMX_VERB_PIN_ACCEPT_FMT, OMX_VERB_PIN_ACCEPT_REPLY, { OMX_HOST_VERB_REQUIRED, OMX_HOST_VERB_UNSUPPORTED, OMX_HOST_VERB_UNSUPPORTED } },
    { OMX_VERB_PIN_LAYOUT, OMX_VERB_PIN_LAYOUT_FMT, OMX_VERB_PIN_LAYOUT_REPLY, { OMX_HOST_VERB_UNSUPPORTED, OMX_HOST_VERB_OPTIONAL, OMX_HOST_VERB_UNSUPPORTED } },
    { OMX_VERB_LV2_WORKER_SET, OMX_VERB_LV2_WORKER_SET_FMT, OMX_VERB_LV2_WORKER_SET_REPLY, { OMX_HOST_VERB_REQUIRED, OMX_HOST_VERB_UNSUPPORTED, OMX_HOST_VERB_UNSUPPORTED } },
    { OMX_VERB_CHAIN, OMX_VERB_CHAIN_FMT, OMX_VERB_CHAIN_REPLY, { OMX_HOST_VERB_REQUIRED, OMX_HOST_VERB_REQUIRED, OMX_HOST_VERB_UNSUPPORTED } },
    { OMX_VERB_CHAIN_CLEAR, OMX_VERB_CHAIN_CLEAR_FMT, OMX_VERB_CHAIN_CLEAR_REPLY, { OMX_HOST_VERB_REQUIRED, OMX_HOST_VERB_UNSUPPORTED, OMX_HOST_VERB_UNSUPPORTED } },
    { OMX_VERB_CHAIN_GET, OMX_VERB_CHAIN_GET_FMT, OMX_VERB_CHAIN_GET_REPLY, { OMX_HOST_VERB_REQUIRED, OMX_HOST_VERB_UNSUPPORTED, OMX_HOST_VERB_UNSUPPORTED } },
    { OMX_VERB_DESCRIPTOR_GET, OMX_VERB_DESCRIPTOR_GET_FMT, OMX_VERB_DESCRIPTOR_GET_REPLY, { OMX_HOST_VERB_REQUIRED, OMX_HOST_VERB_UNSUPPORTED, OMX_HOST_VERB_UNSUPPORTED } },
    { OMX_VERB_PANEL_GET, OMX_VERB_PANEL_GET_FMT, OMX_VERB_PANEL_GET_REPLY, { OMX_HOST_VERB_REQUIRED, OMX_HOST_VERB_UNSUPPORTED, OMX_HOST_VERB_UNSUPPORTED } },
    { OMX_VERB_PRESET_LIST, OMX_VERB_PRESET_LIST_FMT, OMX_VERB_PRESET_LIST_REPLY, { OMX_HOST_VERB_OPTIONAL, OMX_HOST_VERB_OPTIONAL, OMX_HOST_VERB_UNSUPPORTED } },
    { OMX_VERB_PRESET_GET, OMX_VERB_PRESET_GET_FMT, OMX_VERB_PRESET_GET_REPLY, { OMX_HOST_VERB_OPTIONAL, OMX_HOST_VERB_OPTIONAL, OMX_HOST_VERB_UNSUPPORTED } },
    { OMX_VERB_PRESET_SHOW, OMX_VERB_PRESET_SHOW_FMT, OMX_VERB_PRESET_SHOW_REPLY, { OMX_HOST_VERB_UNSUPPORTED, OMX_HOST_VERB_OPTIONAL, OMX_HOST_VERB_UNSUPPORTED } },
    { OMX_VERB_HISTORY_GET, OMX_VERB_HISTORY_GET_FMT, OMX_VERB_HISTORY_GET_REPLY, { OMX_HOST_VERB_REQUIRED, OMX_HOST_VERB_UNSUPPORTED, OMX_HOST_VERB_UNSUPPORTED } },
    { OMX_VERB_HISTORY_CLEAR, OMX_VERB_HISTORY_CLEAR_FMT, OMX_VERB_HISTORY_CLEAR_REPLY, { OMX_HOST_VERB_REQUIRED, OMX_HOST_VERB_UNSUPPORTED, OMX_HOST_VERB_UNSUPPORTED } },
    { OMX_VERB_PLAN, OMX_VERB_PLAN_FMT, OMX_VERB_PLAN_REPLY, { OMX_HOST_VERB_REQUIRED, OMX_HOST_VERB_UNSUPPORTED, OMX_HOST_VERB_UNSUPPORTED } },
    { OMX_VERB_PLAN_GET, OMX_VERB_PLAN_GET_FMT, OMX_VERB_PLAN_GET_REPLY, { OMX_HOST_VERB_REQUIRED, OMX_HOST_VERB_UNSUPPORTED, OMX_HOST_VERB_UNSUPPORTED } },
    { OMX_VERB_PLAN_CLEAR, OMX_VERB_PLAN_CLEAR_FMT, OMX_VERB_PLAN_CLEAR_REPLY, { OMX_HOST_VERB_REQUIRED, OMX_HOST_VERB_UNSUPPORTED, OMX_HOST_VERB_UNSUPPORTED } },
    { OMX_VERB_ESTIMATE, OMX_VERB_ESTIMATE_FMT, OMX_VERB_ESTIMATE_REPLY, { OMX_HOST_VERB_REQUIRED, OMX_HOST_VERB_UNSUPPORTED, OMX_HOST_VERB_UNSUPPORTED } },
    { OMX_VERB_NODE_LABEL, OMX_VERB_NODE_LABEL_FMT, OMX_VERB_NODE_LABEL_REPLY, { OMX_HOST_VERB_OPTIONAL, OMX_HOST_VERB_OPTIONAL, OMX_HOST_VERB_UNSUPPORTED } },
    { OMX_VERB_DESCRIPTION_GET, OMX_VERB_DESCRIPTION_GET_FMT, OMX_VERB_DESCRIPTION_GET_REPLY, { OMX_HOST_VERB_REQUIRED, OMX_HOST_VERB_UNSUPPORTED, OMX_HOST_VERB_UNSUPPORTED } },
    { OMX_VERB_HELLO, OMX_VERB_HELLO_FMT, OMX_VERB_HELLO_REPLY, { OMX_HOST_VERB_OPTIONAL, OMX_HOST_VERB_OPTIONAL, OMX_HOST_VERB_UNSUPPORTED } },
    { OMX_VERB_PARAM_GET_MANY, OMX_VERB_PARAM_GET_MANY_FMT, OMX_VERB_PARAM_GET_MANY_REPLY, { OMX_HOST_VERB_OPTIONAL, OMX_HOST_VERB_OPTIONAL, OMX_HOST_VERB_UNSUPPORTED } },
    { OMX_VERB_PARAM_SET_MANY, OMX_VERB_PARAM_SET_MANY_FMT, OMX_VERB_PARAM_SET_MANY_REPLY, { OMX_HOST_VERB_OPTIONAL, OMX_HOST_VERB_OPTIONAL, OMX_HOST_VERB_UNSUPPORTED } },
    { OMX_VERB_CURVE_BIND, OMX_VERB_CURVE_BIND_FMT, OMX_VERB_CURVE_BIND_REPLY, { OMX_HOST_VERB_OPTIONAL, OMX_HOST_VERB_OPTIONAL, OMX_HOST_VERB_UNSUPPORTED } },
    { OMX_VERB_CURVE_POINTS, OMX_VERB_CURVE_POINTS_FMT, OMX_VERB_CURVE_POINTS_REPLY, { OMX_HOST_VERB_OPTIONAL, OMX_HOST_VERB_OPTIONAL, OMX_HOST_VERB_UNSUPPORTED } },
    { OMX_VERB_MONITOR_OUTPUT_OFF, OMX_VERB_MONITOR_OUTPUT_OFF_FMT, OMX_VERB_MONITOR_OUTPUT_OFF_REPLY, { OMX_HOST_VERB_OPTIONAL, OMX_HOST_VERB_OPTIONAL, OMX_HOST_VERB_UNSUPPORTED } },
    { OMX_VERB_TRACK_INFO, OMX_VERB_TRACK_INFO_FMT, OMX_VERB_TRACK_INFO_REPLY, { OMX_HOST_VERB_OPTIONAL, OMX_HOST_VERB_REQUIRED, OMX_HOST_VERB_UNSUPPORTED } },
    { OMX_VERB_REMOTE_PAGES, OMX_VERB_REMOTE_PAGES_FMT, OMX_VERB_REMOTE_PAGES_REPLY, { OMX_HOST_VERB_OPTIONAL, OMX_HOST_VERB_REQUIRED, OMX_HOST_VERB_UNSUPPORTED } },
    { OMX_VERB_REMOTE_PAGE_GET, OMX_VERB_REMOTE_PAGE_GET_FMT, OMX_VERB_REMOTE_PAGE_GET_REPLY, { OMX_HOST_VERB_OPTIONAL, OMX_HOST_VERB_REQUIRED, OMX_HOST_VERB_UNSUPPORTED } },
    { OMX_VERB_PARAM_INFO, OMX_VERB_PARAM_INFO_FMT, OMX_VERB_PARAM_INFO_REPLY, { OMX_HOST_VERB_OPTIONAL, OMX_HOST_VERB_REQUIRED, OMX_HOST_VERB_UNSUPPORTED } },
    { OMX_VERB_PIN_EXPECT, OMX_VERB_PIN_EXPECT_FMT, OMX_VERB_PIN_EXPECT_REPLY, { OMX_HOST_VERB_UNSUPPORTED, OMX_HOST_VERB_REQUIRED, OMX_HOST_VERB_UNSUPPORTED } },
    { OMX_VERB_CHAIN_EXPECT, OMX_VERB_CHAIN_EXPECT_FMT, OMX_VERB_CHAIN_EXPECT_REPLY, { OMX_HOST_VERB_UNSUPPORTED, OMX_HOST_VERB_REQUIRED, OMX_HOST_VERB_UNSUPPORTED } },
    { OMX_VERB_NOTE_INPUT, OMX_VERB_NOTE_INPUT_FMT, OMX_VERB_NOTE_INPUT_REPLY, { OMX_HOST_VERB_OPTIONAL, OMX_HOST_VERB_REQUIRED, OMX_HOST_VERB_UNSUPPORTED } },
    { OMX_VERB_NOTE_ON, OMX_VERB_NOTE_ON_FMT, OMX_VERB_NOTE_ON_REPLY, { OMX_HOST_VERB_OPTIONAL, OMX_HOST_VERB_REQUIRED, OMX_HOST_VERB_UNSUPPORTED } },
    { OMX_VERB_NOTE_OFF, OMX_VERB_NOTE_OFF_FMT, OMX_VERB_NOTE_OFF_REPLY, { OMX_HOST_VERB_OPTIONAL, OMX_HOST_VERB_REQUIRED, OMX_HOST_VERB_UNSUPPORTED } },
    { OMX_VERB_ALL_NOTES_OFF, OMX_VERB_ALL_NOTES_OFF_FMT, OMX_VERB_ALL_NOTES_OFF_REPLY, { OMX_HOST_VERB_OPTIONAL, OMX_HOST_VERB_REQUIRED, OMX_HOST_VERB_UNSUPPORTED } },
    { OMX_VERB_PROPERTY_LIST, OMX_VERB_PROPERTY_LIST_FMT, OMX_VERB_PROPERTY_LIST_REPLY, { OMX_HOST_VERB_OPTIONAL, OMX_HOST_VERB_REQUIRED, OMX_HOST_VERB_UNSUPPORTED } },
    { OMX_VERB_PROPERTY_INFO, OMX_VERB_PROPERTY_INFO_FMT, OMX_VERB_PROPERTY_INFO_REPLY, { OMX_HOST_VERB_OPTIONAL, OMX_HOST_VERB_REQUIRED, OMX_HOST_VERB_UNSUPPORTED } },
    { OMX_VERB_PROPERTY_GET, OMX_VERB_PROPERTY_GET_FMT, OMX_VERB_PROPERTY_GET_REPLY, { OMX_HOST_VERB_OPTIONAL, OMX_HOST_VERB_REQUIRED, OMX_HOST_VERB_UNSUPPORTED } },
    { OMX_VERB_PROPERTY_SET, OMX_VERB_PROPERTY_SET_FMT, OMX_VERB_PROPERTY_SET_REPLY, { OMX_HOST_VERB_OPTIONAL, OMX_HOST_VERB_REQUIRED, OMX_HOST_VERB_UNSUPPORTED } },
    { OMX_VERB_AUDIO_PORTS, OMX_VERB_AUDIO_PORTS_FMT, OMX_VERB_AUDIO_PORTS_REPLY, { OMX_HOST_VERB_UNSUPPORTED, OMX_HOST_VERB_REQUIRED, OMX_HOST_VERB_UNSUPPORTED } },
    { OMX_VERB_HIDE_EXTERNAL_UI, OMX_VERB_HIDE_EXTERNAL_UI_FMT, OMX_VERB_HIDE_EXTERNAL_UI_REPLY, { OMX_HOST_VERB_OPTIONAL, OMX_HOST_VERB_REQUIRED, OMX_HOST_VERB_UNSUPPORTED } },
    { OMX_VERB_IDLE_HOLD, OMX_VERB_IDLE_HOLD_FMT, OMX_VERB_IDLE_HOLD_REPLY, { OMX_HOST_VERB_UNSUPPORTED, OMX_HOST_VERB_REQUIRED, OMX_HOST_VERB_UNSUPPORTED } },
    { OMX_VERB_NOTE_DROPS, OMX_VERB_NOTE_DROPS_FMT, OMX_VERB_NOTE_DROPS_REPLY, { OMX_HOST_VERB_UNSUPPORTED, OMX_HOST_VERB_REQUIRED, OMX_HOST_VERB_UNSUPPORTED } },
    { OMX_VERB_NOTE_OUT_DROPS, OMX_VERB_NOTE_OUT_DROPS_FMT, OMX_VERB_NOTE_OUT_DROPS_REPLY, { OMX_HOST_VERB_UNSUPPORTED, OMX_HOST_VERB_REQUIRED, OMX_HOST_VERB_UNSUPPORTED } },
  };
  return table;
}

/* The verb whose word is `verb`, or NULL. */
static inline const struct omx_host_verb *plugin_hostd_protocol_host_verb_find(const char *verb) {
  const struct omx_host_verb *t = plugin_hostd_protocol_host_verbs();
  for (unsigned i = 0; i < PLUGIN_HOSTD_PROTOCOL_HOST_VERB_COUNT; i++)
    if (!strcmp(t[i].verb, verb)) return &t[i];
  return NULL;
}

/* What the host at column `host` (an OMX_HOST_VERB_HOST_* index) owes `verb`: unsupported for a verb not here. */
static inline enum omx_host_verb_level plugin_hostd_protocol_host_verb_level(const char *verb, unsigned host) {
  const struct omx_host_verb *v = plugin_hostd_protocol_host_verb_find(verb);
  return v && host < OMX_HOST_VERB_HOST_COUNT ? v->level[host] : OMX_HOST_VERB_UNSUPPORTED;
}

#endif /* PLUGIN_HOSTD_PROTOCOL_HOST_VERBS_H */
