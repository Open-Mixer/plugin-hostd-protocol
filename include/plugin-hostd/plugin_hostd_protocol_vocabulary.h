// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
#ifndef PLUGIN_HOSTD_PROTOCOL_VOCABULARY_H
#define PLUGIN_HOSTD_PROTOCOL_VOCABULARY_H
/*
 * GENERATED: DO NOT EDIT BY HAND.
 * Produced by `omx-contract render --target c` from Open-Mixer/plugin-hostd-protocol's data/: the vocabulary
 * (docs/data-layout.md). A code is OMX_ and the code in upper snake case, and a reply code that carries a resp has it as that name and _RESP; a word is OMX_<ITEM>_<NAME>, each words
 * item with its _COUNT and an _INIT of its words in their order. A consumer writes and compares these, never a
 * literal of its own.
 */

/** @file
 * @brief The vocabulary of plugin-hostd-protocol: every code and every word as a string define, with its sentence
 * where it carries one. */

/* HOSTING_VERB_HOSTS (contract.d/hosting-verb/hosting-verb.json) */
#define OMX_HOSTING_VERB_HOSTS_PLUGIN_HOSTD "plugin-hostd" ///< The daemon that places plugins in workers and answers the verbs it adds itself.
/** The source sentence (English) of OMX_HOSTING_VERB_HOSTS_PLUGIN_HOSTD. */
#define OMX_HOSTING_VERB_HOSTS_PLUGIN_HOSTD_TEXT_EN "The daemon that places plugins in workers and answers the verbs it adds itself."
#define OMX_HOSTING_VERB_HOSTS_OMX_CLAP_HOST "omx-clap-host" ///< omx-clap-core's worker: a CLAP plugin's host, and an LV2 plugin's through its adapter.
/** The source sentence (English) of OMX_HOSTING_VERB_HOSTS_OMX_CLAP_HOST. */
#define OMX_HOSTING_VERB_HOSTS_OMX_CLAP_HOST_TEXT_EN "omx-clap-core's worker: a CLAP plugin's host, and an LV2 plugin's through its adapter."
#define OMX_HOSTING_VERB_HOSTS_MOD_HOST "mod-host" ///< mod-host, the LV2 host whose protocol the others speak.
/** The source sentence (English) of OMX_HOSTING_VERB_HOSTS_MOD_HOST. */
#define OMX_HOSTING_VERB_HOSTS_MOD_HOST_TEXT_EN "mod-host, the LV2 host whose protocol the others speak."
/** The number of words of HOSTING_VERB_HOSTS. */
#define OMX_HOSTING_VERB_HOSTS_COUNT 3u
/** The words of HOSTING_VERB_HOSTS in their declared order, as a C initializer list. */
#define OMX_HOSTING_VERB_HOSTS_INIT { "plugin-hostd", "omx-clap-host", "mod-host" }
/** Expands `X(code, "word", NAME)` once per word of HOSTING_VERB_HOSTS, in order, the code being its place in the order. The hosts a verb's implementedBy names (docs/contract-dir.md): the programs a controller sends mod-host's protocol to. A verb states what each one owes it; a host the verb leaves out owes it nothing. */
#define OMX_HOSTING_VERB_HOSTS(X) \
    X(0u, "plugin-hostd", PLUGIN_HOSTD) X(1u, "omx-clap-host", OMX_CLAP_HOST) X(2u, "mod-host", MOD_HOST)

/* HOST_REPLY_CODES (contract.d/host/host.json) */
#define OMX_REPLY_HOST_INSTANCE_INVALID "reply.host.instance-invalid" ///< mod-host's ERR_INSTANCE_INVALID: an instance number outside the host's table
#define OMX_REPLY_HOST_INSTANCE_INVALID_RESP (-1)
/** The source sentence (English) of OMX_REPLY_HOST_INSTANCE_INVALID. */
#define OMX_REPLY_HOST_INSTANCE_INVALID_TEXT_EN "That instance number is not one of the host\342\200\231s. Use an instance the host actually has."
#define OMX_REPLY_HOST_INSTANCE_ALREADY_EXISTS "reply.host.instance-already-exists" ///< mod-host's ERR_INSTANCE_ALREADY_EXISTS: add asked for an instance number the host already has
#define OMX_REPLY_HOST_INSTANCE_ALREADY_EXISTS_RESP (-2)
/** The source sentence (English) of OMX_REPLY_HOST_INSTANCE_ALREADY_EXISTS. */
#define OMX_REPLY_HOST_INSTANCE_ALREADY_EXISTS_TEXT_EN "That instance number is already taken. Add the plugin under a number the host does not have yet."
#define OMX_REPLY_HOST_INSTANCE_NON_EXISTS "reply.host.instance-non-exists" ///< mod-host's ERR_INSTANCE_NON_EXISTS: an instance no add made
#define OMX_REPLY_HOST_INSTANCE_NON_EXISTS_RESP (-3)
/** The source sentence (English) of OMX_REPLY_HOST_INSTANCE_NON_EXISTS. */
#define OMX_REPLY_HOST_INSTANCE_NON_EXISTS_TEXT_EN "No add ever created that instance. Add it before using it."
#define OMX_REPLY_HOST_URI_INVALID "reply.host.uri-invalid" ///< mod-host's ERR_LV2_INVALID_URI: add named a plugin the host does not find
#define OMX_REPLY_HOST_URI_INVALID_RESP (-101)
/** The source sentence (English) of OMX_REPLY_HOST_URI_INVALID. */
#define OMX_REPLY_HOST_URI_INVALID_TEXT_EN "The host finds no plugin by that name. Check the plugin is installed and its name is spelt as the scan lists it."
#define OMX_REPLY_HOST_INSTANTIATION_FAILED "reply.host.instantiation-failed" ///< mod-host's ERR_LV2_INSTANTIATION: the plugin refused to start
#define OMX_REPLY_HOST_INSTANTIATION_FAILED_RESP (-102)
/** The source sentence (English) of OMX_REPLY_HOST_INSTANTIATION_FAILED. */
#define OMX_REPLY_HOST_INSTANTIATION_FAILED_TEXT_EN "The plugin was found but would not start. Try it again, or choose another plugin if it keeps refusing."
#define OMX_REPLY_HOST_PARAM_SYMBOL_INVALID "reply.host.param-symbol-invalid" ///< mod-host's ERR_LV2_INVALID_PARAM_SYMBOL: a symbol that is not a parameter param_set takes, the plugin's own bypass included
#define OMX_REPLY_HOST_PARAM_SYMBOL_INVALID_RESP (-103)
/** The source sentence (English) of OMX_REPLY_HOST_PARAM_SYMBOL_INVALID. */
#define OMX_REPLY_HOST_PARAM_SYMBOL_INVALID_TEXT_EN "That symbol is not one of this plugin\342\200\231s parameters, not even its bypass. Ask the plugin for its parameter list and use one of those symbols."
#define OMX_REPLY_HOST_PRESET_URI_INVALID "reply.host.preset-uri-invalid" ///< mod-host's ERR_LV2_INVALID_PRESET_URI: a preset this plugin does not have, or a file it cannot take
#define OMX_REPLY_HOST_PRESET_URI_INVALID_RESP (-104)
/** The source sentence (English) of OMX_REPLY_HOST_PRESET_URI_INVALID. */
#define OMX_REPLY_HOST_PRESET_URI_INVALID_TEXT_EN "That preset is not one this plugin can load. Choose a preset from the plugin\342\200\231s own list."
#define OMX_REPLY_HOST_STATE_LOAD_FAILED "reply.host.state-load-failed" ///< mod-host's ERR_LV2_CANT_LOAD_STATE: the plugin refused its saved state
#define OMX_REPLY_HOST_STATE_LOAD_FAILED_RESP (-105)
/** The source sentence (English) of OMX_REPLY_HOST_STATE_LOAD_FAILED. */
#define OMX_REPLY_HOST_STATE_LOAD_FAILED_TEXT_EN "The plugin would not take its saved state back. Load another preset, or set its values again."
#define OMX_REPLY_HOST_JACK_CLIENT_CREATION "reply.host.jack-client-creation" ///< mod-host's ERR_JACK_CLIENT_CREATION: no audio client could be made for the plugin
#define OMX_REPLY_HOST_JACK_CLIENT_CREATION_RESP (-201)
/** The source sentence (English) of OMX_REPLY_HOST_JACK_CLIENT_CREATION. */
#define OMX_REPLY_HOST_JACK_CLIENT_CREATION_TEXT_EN "The host could not connect the plugin to the audio server. Check the audio server is running, then add the plugin again."
#define OMX_REPLY_HOST_JACK_CLIENT_ACTIVATION "reply.host.jack-client-activation" ///< mod-host's ERR_JACK_CLIENT_ACTIVATION: the plugin's audio client would not start
#define OMX_REPLY_HOST_JACK_CLIENT_ACTIVATION_RESP (-202)
/** The source sentence (English) of OMX_REPLY_HOST_JACK_CLIENT_ACTIVATION. */
#define OMX_REPLY_HOST_JACK_CLIENT_ACTIVATION_TEXT_EN "The plugin\342\200\231s audio connection would not start. Check the audio server is running, then add the plugin again."
#define OMX_REPLY_HOST_JACK_CLIENT_DEACTIVATION "reply.host.jack-client-deactivation" ///< mod-host's ERR_JACK_CLIENT_DEACTIVATION: the plugin's audio client would not stop
#define OMX_REPLY_HOST_JACK_CLIENT_DEACTIVATION_RESP (-203)
/** The source sentence (English) of OMX_REPLY_HOST_JACK_CLIENT_DEACTIVATION. */
#define OMX_REPLY_HOST_JACK_CLIENT_DEACTIVATION_TEXT_EN "The plugin\342\200\231s audio connection would not stop. Remove the plugin, or restart the audio server if it stays."
#define OMX_REPLY_HOST_JACK_PORT_REGISTER "reply.host.jack-port-register" ///< mod-host's ERR_JACK_PORT_REGISTER: a port of the plugin could not be made
#define OMX_REPLY_HOST_JACK_PORT_REGISTER_RESP (-204)
/** The source sentence (English) of OMX_REPLY_HOST_JACK_PORT_REGISTER. */
#define OMX_REPLY_HOST_JACK_PORT_REGISTER_TEXT_EN "The host could not make one of the plugin\342\200\231s audio ports. Remove some plugins, then add this one again."
#define OMX_REPLY_HOST_JACK_PORT_CONNECTION "reply.host.jack-port-connection" ///< mod-host's ERR_JACK_PORT_CONNECTION: two ports could not be connected
#define OMX_REPLY_HOST_JACK_PORT_CONNECTION_RESP (-205)
/** The source sentence (English) of OMX_REPLY_HOST_JACK_PORT_CONNECTION. */
#define OMX_REPLY_HOST_JACK_PORT_CONNECTION_TEXT_EN "Those two ports could not be connected. Check both ports exist and carry the same kind of signal."
#define OMX_REPLY_HOST_JACK_PORT_DISCONNECTION "reply.host.jack-port-disconnection" ///< mod-host's ERR_JACK_PORT_DISCONNECTION: two ports could not be disconnected
#define OMX_REPLY_HOST_JACK_PORT_DISCONNECTION_RESP (-206)
/** The source sentence (English) of OMX_REPLY_HOST_JACK_PORT_DISCONNECTION. */
#define OMX_REPLY_HOST_JACK_PORT_DISCONNECTION_TEXT_EN "Those two ports could not be disconnected. Check they are connected to each other."
#define OMX_REPLY_HOST_VALUE_OUT_OF_RANGE "reply.host.value-out-of-range" ///< mod-host's ERR_JACK_VALUE_OUT_OF_RANGE: a tempo or a beats-per-bar the transport does not take
#define OMX_REPLY_HOST_VALUE_OUT_OF_RANGE_RESP (-207)
/** The source sentence (English) of OMX_REPLY_HOST_VALUE_OUT_OF_RANGE. */
#define OMX_REPLY_HOST_VALUE_OUT_OF_RANGE_TEXT_EN "That tempo or beats per bar is outside what the host accepts. Choose a value within the host\342\200\231s range."
#define OMX_REPLY_HOST_EXTERNAL_UI_UNAVAILABLE "reply.host.external-ui-unavailable" ///< mod-host's ERR_EXTERNAL_UI_UNAVAILABLE: the plugin has no window of its own to show
#define OMX_REPLY_HOST_EXTERNAL_UI_UNAVAILABLE_RESP (-404)
/** The source sentence (English) of OMX_REPLY_HOST_EXTERNAL_UI_UNAVAILABLE. */
#define OMX_REPLY_HOST_EXTERNAL_UI_UNAVAILABLE_TEXT_EN "This plugin has no window of its own to show. Use the host\342\200\231s controls for it instead."
#define OMX_REPLY_HOST_MEMORY_ALLOCATION "reply.host.memory-allocation" ///< mod-host's ERR_MEMORY_ALLOCATION: the reply could not be built
#define OMX_REPLY_HOST_MEMORY_ALLOCATION_RESP (-901)
/** The source sentence (English) of OMX_REPLY_HOST_MEMORY_ALLOCATION. */
#define OMX_REPLY_HOST_MEMORY_ALLOCATION_TEXT_EN "The host ran out of memory while building the reply. Try again once memory is free."
#define OMX_REPLY_HOST_INVALID_OPERATION "reply.host.invalid-operation" ///< mod-host's ERR_INVALID_OPERATION: a word outside the verb's grammar, or a verb the host refuses on purpose
#define OMX_REPLY_HOST_INVALID_OPERATION_RESP (-902)
/** The source sentence (English) of OMX_REPLY_HOST_INVALID_OPERATION. */
#define OMX_REPLY_HOST_INVALID_OPERATION_TEXT_EN "That word is not part of this verb, or the host refuses it on purpose. Check the verb\342\200\231s grammar and send a word it accepts."
#define OMX_REPLY_HOST_PIN_ABSENT "reply.host.pin-absent" ///< a layout pin in a scheme this host does not know, refused as no pin at all
#define OMX_REPLY_HOST_PIN_ABSENT_RESP (-508)
/** The source sentence (English) of OMX_REPLY_HOST_PIN_ABSENT. */
#define OMX_REPLY_HOST_PIN_ABSENT_TEXT_EN "This host does not know that layout pin\342\200\231s scheme, so it is refused as if no pin had been sent. Send a pin in a scheme this host knows."
#define OMX_REPLY_HOST_PIN_LAYOUT_MISMATCH "reply.host.pin-layout-mismatch" ///< the add of an instance whose layout after init is not the one pin_expect named: the plugin is not activated
#define OMX_REPLY_HOST_PIN_LAYOUT_MISMATCH_RESP (-510)
/** The source sentence (English) of OMX_REPLY_HOST_PIN_LAYOUT_MISMATCH. */
#define OMX_REPLY_HOST_PIN_LAYOUT_MISMATCH_TEXT_EN "After it started, the plugin\342\200\231s layout did not match the pin you expected, so it was not activated. Expect the layout the plugin actually has, or use a plugin built to that pin."
#define OMX_REPLY_HOST_NO_PARAM_CONTRACT "reply.host.no-param-contract" ///< the host holds no checked unit and scale for this parameter: the controller falls back to the layout the scan gives
#define OMX_REPLY_HOST_NO_PARAM_CONTRACT_RESP (-511)
/** The source sentence (English) of OMX_REPLY_HOST_NO_PARAM_CONTRACT. */
#define OMX_REPLY_HOST_NO_PARAM_CONTRACT_TEXT_EN "The host has no checked unit or scale for this parameter. Fall back to the layout the scan gave."
#define OMX_REPLY_HOST_NO_NOTE_INPUT "reply.host.no-note-input" ///< the instance's plugin takes no notes
#define OMX_REPLY_HOST_NO_NOTE_INPUT_RESP (-513)
/** The source sentence (English) of OMX_REPLY_HOST_NO_NOTE_INPUT. */
#define OMX_REPLY_HOST_NO_NOTE_INPUT_TEXT_EN "This plugin\342\200\231s instance takes no notes. Don\342\200\231t send it note events."
#define OMX_REPLY_HOST_NOTES_BY_JACK "reply.host.notes-by-jack" ///< the instance's worker takes no notes on its socket (mod-host): the plugin is played through its jack MIDI input
#define OMX_REPLY_HOST_NOTES_BY_JACK_RESP (-514)
/** The source sentence (English) of OMX_REPLY_HOST_NOTES_BY_JACK. */
#define OMX_REPLY_HOST_NOTES_BY_JACK_TEXT_EN "This instance\342\200\231s worker takes no notes on its socket; play it through its jack MIDI input instead."
#define OMX_REPLY_HOST_NOTE_QUEUE_FULL "reply.host.note-queue-full" ///< the worker has no room for the line's notes before its next block: nothing of it is kept, and the line may be sent again
#define OMX_REPLY_HOST_NOTE_QUEUE_FULL_RESP (-517)
/** The source sentence (English) of OMX_REPLY_HOST_NOTE_QUEUE_FULL. */
#define OMX_REPLY_HOST_NOTE_QUEUE_FULL_TEXT_EN "The worker had no room for these notes before its next block, so none of them were kept. Send the line again."
#define OMX_REPLY_HOST_PROPERTY_SYMBOL_INVALID "reply.host.property-symbol-invalid" ///< a symbol that is not one of the instance's file or text properties (org.openmixer.properties/1), or an index past its last
#define OMX_REPLY_HOST_PROPERTY_SYMBOL_INVALID_RESP (-106)
/** The source sentence (English) of OMX_REPLY_HOST_PROPERTY_SYMBOL_INVALID. */
#define OMX_REPLY_HOST_PROPERTY_SYMBOL_INVALID_TEXT_EN "That symbol is not one of this plugin\342\200\231s file or text properties. Ask the plugin for its property list and use one of those symbols."
#define OMX_REPLY_HOST_PROPERTY_ACCESS "reply.host.property-access" ///< a set of a property that is not writable, or a get of one that is not readable
#define OMX_REPLY_HOST_PROPERTY_ACCESS_RESP (-107)
/** The source sentence (English) of OMX_REPLY_HOST_PROPERTY_ACCESS. */
#define OMX_REPLY_HOST_PROPERTY_ACCESS_TEXT_EN "This property cannot be used that way: it is not writable, or not readable. Check the property's access before setting or reading it."
#define OMX_REPLY_HOST_PROPERTY_VALUE_INVALID "reply.host.property-value-invalid" ///< a property value the host does not hand on: larger than OMX_CLAP_PROPERTY_VALUE_MAX with its NUL, not UTF-8, or a path that is not absolute
#define OMX_REPLY_HOST_PROPERTY_VALUE_INVALID_RESP (-108)
/** The source sentence (English) of OMX_REPLY_HOST_PROPERTY_VALUE_INVALID. */
#define OMX_REPLY_HOST_PROPERTY_VALUE_INVALID_TEXT_EN "That value cannot be set: it is too long, not valid text, or a path that does not start with a slash. Send a shorter value, or an absolute path."
#define OMX_REPLY_HOST_PROPERTY_QUEUE_FULL "reply.host.property-queue-full" ///< the instance's property ring has no room before its next block: nothing of the set is kept, and it may be sent again
#define OMX_REPLY_HOST_PROPERTY_QUEUE_FULL_RESP (-109)
/** The source sentence (English) of OMX_REPLY_HOST_PROPERTY_QUEUE_FULL. */
#define OMX_REPLY_HOST_PROPERTY_QUEUE_FULL_TEXT_EN "The plugin had no room for this value before its next block, so it was not kept. Send it again."

/* HOST_TRACK_KINDS (contract.d/host/host.json) */
#define OMX_HOST_TRACK_KINDS_BUS "bus"
#define OMX_HOST_TRACK_KINDS_RETURN "return"
#define OMX_HOST_TRACK_KINDS_MASTER "master"
/** The number of words of HOST_TRACK_KINDS. */
#define OMX_HOST_TRACK_KINDS_COUNT 3u
/** The words of HOST_TRACK_KINDS in their declared order, as a C initializer list. */
#define OMX_HOST_TRACK_KINDS_INIT { "bus", "return", "master" }
/** Expands `X(code, "word", NAME)` once per word of HOST_TRACK_KINDS, in order, the code being its place in the order. The kind of strip, the last word of track_info; a line without it is an input channel. */
#define OMX_HOST_TRACK_KINDS(X) \
    X(0u, "bus", BUS) X(1u, "return", RETURN) X(2u, "master", MASTER)

/* HOST_EVENTS (contract.d/host/host.json) */
#define OMX_HOST_EVENTS_REMOTE_PAGES_CHANGED "remote_pages_changed" ///< the plugin changed its remote pages: a controller reads remote_pages and remote_page_get again
#define OMX_HOST_EVENTS_XRUNS "xruns" ///< the worker's JACK xrun count since its start, written when it changes and at most once a second; a worker that never writes it reports no xruns
#define OMX_HOST_EVENTS_ABI_CLOBBER "abi_clobber" ///< the plugin returned from a call with a callee-saved register changed (0.26.2): the instance number, the registers as a hexadecimal mask with their names, the plugin function and the block, written once per instance. The worker put the registers back, so its own frame is whole; the other members' state in the same process is not covered, so the plugin is run only in a worker of its own
/** The number of words of HOST_EVENTS. */
#define OMX_HOST_EVENTS_COUNT 3u
/** The words of HOST_EVENTS in their declared order, as a C initializer list. */
#define OMX_HOST_EVENTS_INIT { "remote_pages_changed", "xruns", "abi_clobber" }
/** Expands `X(code, "word", NAME)` once per word of HOST_EVENTS, in order, the code being its place in the order. The lines a host writes on its feedback port for these verbs, each followed by the instance number, but xruns, which is the worker's own and is followed by its count. */
#define OMX_HOST_EVENTS(X) \
    X(0u, "remote_pages_changed", REMOTE_PAGES_CHANGED) X(1u, "xruns", XRUNS) X(2u, "abi_clobber", ABI_CLOBBER)

/* PLUGIN_HOSTD_PROTOCOL_REPLY_CODES (contract.d/plugin-hostd-protocol/plugin-hostd-protocol.json) */
#define OMX_REPLY_PLUGIN_HOSTD_PLACEMENT_INVALID "reply.plugin-hostd.placement-invalid" ///< not a placement, or not valid here
#define OMX_REPLY_PLUGIN_HOSTD_PLACEMENT_INVALID_RESP (-501)
/** The source sentence (English) of OMX_REPLY_PLUGIN_HOSTD_PLACEMENT_INVALID. */
#define OMX_REPLY_PLUGIN_HOSTD_PLACEMENT_INVALID_TEXT_EN "The plugin cannot be placed there; choose another place for it."
#define OMX_REPLY_PLUGIN_HOSTD_NO_BACKEND "reply.plugin-hostd.no-backend" ///< no worker program for the scheme
#define OMX_REPLY_PLUGIN_HOSTD_NO_BACKEND_RESP (-502)
/** The source sentence (English) of OMX_REPLY_PLUGIN_HOSTD_NO_BACKEND. */
#define OMX_REPLY_PLUGIN_HOSTD_NO_BACKEND_TEXT_EN "No installed program can run this kind of plugin; install one, or choose a plugin of another kind."
#define OMX_REPLY_PLUGIN_HOSTD_WORKER_SPAWN "reply.plugin-hostd.worker-spawn" ///< the worker is not up
#define OMX_REPLY_PLUGIN_HOSTD_WORKER_SPAWN_RESP (-503)
#define OMX_REPLY_PLUGIN_HOSTD_WORKER_SPAWN_TRANSIENT 1
/** The source sentence (English) of OMX_REPLY_PLUGIN_HOSTD_WORKER_SPAWN. */
#define OMX_REPLY_PLUGIN_HOSTD_WORKER_SPAWN_TEXT_EN "The program that runs the plugin is not running; wait a moment and try again."
#define OMX_REPLY_PLUGIN_HOSTD_REPLAY "reply.plugin-hostd.replay" ///< the name of a refused replay on stderr, never a reply
#define OMX_REPLY_PLUGIN_HOSTD_REPLAY_RESP (-504)
/** The source sentence (English) of OMX_REPLY_PLUGIN_HOSTD_REPLAY. */
#define OMX_REPLY_PLUGIN_HOSTD_REPLAY_TEXT_EN "A plugin could not be put back after a restart and was left out; add it again."
#define OMX_REPLY_PLUGIN_HOSTD_GAVE_UP "reply.plugin-hostd.gave-up" ///< the storm bound is spent for that placement
#define OMX_REPLY_PLUGIN_HOSTD_GAVE_UP_RESP (-505)
#define OMX_REPLY_PLUGIN_HOSTD_GAVE_UP_TRANSIENT 1
/** The source sentence (English) of OMX_REPLY_PLUGIN_HOSTD_GAVE_UP. */
#define OMX_REPLY_PLUGIN_HOSTD_GAVE_UP_TEXT_EN "The plugin kept failing and was given up on; check it, then add it again."
#define OMX_REPLY_PLUGIN_HOSTD_NO_SUCH_WORKER "reply.plugin-hostd.no-such-worker" ///< no such worker
#define OMX_REPLY_PLUGIN_HOSTD_NO_SUCH_WORKER_RESP (-506)
/** The source sentence (English) of OMX_REPLY_PLUGIN_HOSTD_NO_SUCH_WORKER. */
#define OMX_REPLY_PLUGIN_HOSTD_NO_SUCH_WORKER_TEXT_EN "No plugin host with that name is running; refresh the list and choose another."
#define OMX_REPLY_PLUGIN_HOSTD_VERB_DROPPED "reply.plugin-hostd.verb-dropped" ///< the worker died on this very command and the daemon dropped it
#define OMX_REPLY_PLUGIN_HOSTD_VERB_DROPPED_RESP (-507)
#define OMX_REPLY_PLUGIN_HOSTD_VERB_DROPPED_TRANSIENT 1
/** The source sentence (English) of OMX_REPLY_PLUGIN_HOSTD_VERB_DROPPED. */
#define OMX_REPLY_PLUGIN_HOSTD_VERB_DROPPED_TEXT_EN "The plugin stopped just as it was asked to do this, so the request was lost; try again."
#define OMX_REPLY_PLUGIN_HOSTD_PIN_BINARY_MISMATCH "reply.plugin-hostd.pin-binary-mismatch" ///< a pinned file is missing or its SHA-256 differs
#define OMX_REPLY_PLUGIN_HOSTD_PIN_BINARY_MISMATCH_RESP (-509)
/** The source sentence (English) of OMX_REPLY_PLUGIN_HOSTD_PIN_BINARY_MISMATCH. */
#define OMX_REPLY_PLUGIN_HOSTD_PIN_BINARY_MISMATCH_TEXT_EN "The plugin's files are not the ones that were approved; install the approved version, or approve this one."
#define OMX_REPLY_PLUGIN_HOSTD_LV2_ADAPTER_REFUSED "reply.plugin-hostd.lv2-adapter-refused" ///< the lv2-clap worker's LV2 adapter cannot run this plugin
#define OMX_REPLY_PLUGIN_HOSTD_LV2_ADAPTER_REFUSED_RESP (-512)
/** The source sentence (English) of OMX_REPLY_PLUGIN_HOSTD_LV2_ADAPTER_REFUSED. */
#define OMX_REPLY_PLUGIN_HOSTD_LV2_ADAPTER_REFUSED_TEXT_EN "This LV2 plugin cannot run here because the adapter does not support something it needs; choose another plugin."
#define OMX_REPLY_PLUGIN_HOSTD_NO_PRESET_LIST "reply.plugin-hostd.no-preset-list" ///< the instance's worker cannot list the plugin's presets
#define OMX_REPLY_PLUGIN_HOSTD_NO_PRESET_LIST_RESP (-515)
/** The source sentence (English) of OMX_REPLY_PLUGIN_HOSTD_NO_PRESET_LIST. */
#define OMX_REPLY_PLUGIN_HOSTD_NO_PRESET_LIST_TEXT_EN "This plugin cannot list its presets; load one by name, or choose one of your own."
#define OMX_REPLY_PLUGIN_HOSTD_PIN_REFUSED "reply.plugin-hostd.pin-refused" ///< the plugin's pin carries verdict=refused
#define OMX_REPLY_PLUGIN_HOSTD_PIN_REFUSED_RESP (-516)
/** The source sentence (English) of OMX_REPLY_PLUGIN_HOSTD_PIN_REFUSED. */
#define OMX_REPLY_PLUGIN_HOSTD_PIN_REFUSED_TEXT_EN "This plugin failed its checks and is blocked; only you can allow it, by approving the plugin."
#define OMX_REPLY_PLUGIN_HOSTD_CHAIN_UNKNOWN_INSTANCE "reply.plugin-hostd.chain-unknown-instance" ///< a member is an instance no add made
#define OMX_REPLY_PLUGIN_HOSTD_CHAIN_UNKNOWN_INSTANCE_RESP (-518)
/** The source sentence (English) of OMX_REPLY_PLUGIN_HOSTD_CHAIN_UNKNOWN_INSTANCE. */
#define OMX_REPLY_PLUGIN_HOSTD_CHAIN_UNKNOWN_INSTANCE_TEXT_EN "The chain includes a plugin that was never added; add it first, or take it out of the chain."
#define OMX_REPLY_PLUGIN_HOSTD_CHAIN_DUPLICATE "reply.plugin-hostd.chain-duplicate" ///< an instance is named twice in one chain
#define OMX_REPLY_PLUGIN_HOSTD_CHAIN_DUPLICATE_RESP (-519)
/** The source sentence (English) of OMX_REPLY_PLUGIN_HOSTD_CHAIN_DUPLICATE. */
#define OMX_REPLY_PLUGIN_HOSTD_CHAIN_DUPLICATE_TEXT_EN "The chain includes the same plugin twice; keep it only once."
#define OMX_REPLY_PLUGIN_HOSTD_CHAIN_INSTANCE_BUSY "reply.plugin-hostd.chain-instance-busy" ///< a member is already in another chain
#define OMX_REPLY_PLUGIN_HOSTD_CHAIN_INSTANCE_BUSY_RESP (-520)
/** The source sentence (English) of OMX_REPLY_PLUGIN_HOSTD_CHAIN_INSTANCE_BUSY. */
#define OMX_REPLY_PLUGIN_HOSTD_CHAIN_INSTANCE_BUSY_TEXT_EN "A plugin in this chain is already in another chain; clear that chain first, or use another plugin."
#define OMX_REPLY_PLUGIN_HOSTD_CHAIN_MIXED_FORMAT "reply.plugin-hostd.chain-mixed-format" ///< no one worker program runs every member
#define OMX_REPLY_PLUGIN_HOSTD_CHAIN_MIXED_FORMAT_RESP (-521)
/** The source sentence (English) of OMX_REPLY_PLUGIN_HOSTD_CHAIN_MIXED_FORMAT. */
#define OMX_REPLY_PLUGIN_HOSTD_CHAIN_MIXED_FORMAT_TEXT_EN "These plugins cannot run together as one chain because no single program can host them all; split the chain."
#define OMX_REPLY_PLUGIN_HOSTD_NO_SUCH_CHAIN "reply.plugin-hostd.no-such-chain" ///< a chain no chain line declared
#define OMX_REPLY_PLUGIN_HOSTD_NO_SUCH_CHAIN_RESP (-522)
/** The source sentence (English) of OMX_REPLY_PLUGIN_HOSTD_NO_SUCH_CHAIN. */
#define OMX_REPLY_PLUGIN_HOSTD_NO_SUCH_CHAIN_TEXT_EN "There is no such chain; declare it first, or choose one that exists."
#define OMX_REPLY_PLUGIN_HOSTD_PRESET_NOT_OFFERED "reply.plugin-hostd.preset-not-offered" ///< a stable name the plugin does not offer
#define OMX_REPLY_PLUGIN_HOSTD_PRESET_NOT_OFFERED_RESP (-523)
/** The source sentence (English) of OMX_REPLY_PLUGIN_HOSTD_PRESET_NOT_OFFERED. */
#define OMX_REPLY_PLUGIN_HOSTD_PRESET_NOT_OFFERED_TEXT_EN "This plugin has no preset with that name; choose one from its list."
#define OMX_REPLY_PLUGIN_HOSTD_PRESET_INVALID "reply.plugin-hostd.preset-invalid" ///< a preset file or path that is not the preset format
#define OMX_REPLY_PLUGIN_HOSTD_PRESET_INVALID_RESP (-524)
/** The source sentence (English) of OMX_REPLY_PLUGIN_HOSTD_PRESET_INVALID. */
#define OMX_REPLY_PLUGIN_HOSTD_PRESET_INVALID_TEXT_EN "The preset file cannot be used because it is unreadable or not in the preset format; choose another file, or save the preset again."
#define OMX_REPLY_PLUGIN_HOSTD_PRESET_NEEDS_STATE "reply.plugin-hostd.preset-needs-state" ///< a preset file that carries state for a worker that cannot take state
#define OMX_REPLY_PLUGIN_HOSTD_PRESET_NEEDS_STATE_RESP (-525)
/** The source sentence (English) of OMX_REPLY_PLUGIN_HOSTD_PRESET_NEEDS_STATE. */
#define OMX_REPLY_PLUGIN_HOSTD_PRESET_NEEDS_STATE_TEXT_EN "This preset holds the plugin's saved state, which this plugin host cannot load yet; use a preset made by the plugin itself."
#define OMX_REPLY_PLUGIN_HOSTD_PRESET_PLUGIN_CHANGED "reply.plugin-hostd.preset-plugin-changed" ///< the preset file's plugin id, version or fingerprint is not the instance's
#define OMX_REPLY_PLUGIN_HOSTD_PRESET_PLUGIN_CHANGED_RESP (-526)
/** The source sentence (English) of OMX_REPLY_PLUGIN_HOSTD_PRESET_PLUGIN_CHANGED. */
#define OMX_REPLY_PLUGIN_HOSTD_PRESET_PLUGIN_CHANGED_TEXT_EN "The preset was made for another plugin, or another version of it, so nothing was applied; choose a preset made for this one."
#define OMX_REPLY_PLUGIN_HOSTD_LOAD_REFUSED "reply.plugin-hostd.load-refused" ///< the plugin refused the preset or the state its file carries
#define OMX_REPLY_PLUGIN_HOSTD_LOAD_REFUSED_RESP (-527)
/** The source sentence (English) of OMX_REPLY_PLUGIN_HOSTD_LOAD_REFUSED. */
#define OMX_REPLY_PLUGIN_HOSTD_LOAD_REFUSED_TEXT_EN "The plugin did not accept the contents of the preset; try another preset."
#define OMX_REPLY_PLUGIN_HOSTD_PRESET_NOT_APPLIED "reply.plugin-hostd.preset-not-applied" ///< the plugin took the preset, and had not run once since, so had not applied it, when the host's wait for that ran out: its values may still be the previous ones
#define OMX_REPLY_PLUGIN_HOSTD_PRESET_NOT_APPLIED_RESP (-544)
/** The source sentence (English) of OMX_REPLY_PLUGIN_HOSTD_PRESET_NOT_APPLIED. */
#define OMX_REPLY_PLUGIN_HOSTD_PRESET_NOT_APPLIED_TEXT_EN "The plugin took the preset but had not applied it yet when the wait ran out; read its values again, or load the preset again."
#define OMX_REPLY_PLUGIN_HOSTD_LABEL_INVALID "reply.plugin-hostd.label-invalid" ///< node_label's label is longer than PHD_NODE_LABEL_MAX bytes or is not UTF-8: the node keeps the label it had
#define OMX_REPLY_PLUGIN_HOSTD_LABEL_INVALID_RESP (-545)
/** The source sentence (English) of OMX_REPLY_PLUGIN_HOSTD_LABEL_INVALID. */
#define OMX_REPLY_PLUGIN_HOSTD_LABEL_INVALID_TEXT_EN "That label is too long or not valid text, so the node keeps its previous name; use at most 64 bytes of UTF-8 text."
#define OMX_REPLY_PLUGIN_HOSTD_DIR_NOT_WRITABLE "reply.plugin-hostd.dir-not-writable" ///< descriptor_dir cannot be made or written
#define OMX_REPLY_PLUGIN_HOSTD_DIR_NOT_WRITABLE_RESP (-528)
/** The source sentence (English) of OMX_REPLY_PLUGIN_HOSTD_DIR_NOT_WRITABLE. */
#define OMX_REPLY_PLUGIN_HOSTD_DIR_NOT_WRITABLE_TEXT_EN "A folder the plugins need cannot be created or written to; check its location and permissions."
#define OMX_REPLY_PLUGIN_HOSTD_DESCRIPTOR_ABSENT "reply.plugin-hostd.descriptor-absent" ///< no descriptor (panel) for the plugin's binary as it is now; of description_get, no layer of the descriptions store holds the plugin
#define OMX_REPLY_PLUGIN_HOSTD_DESCRIPTOR_ABSENT_RESP (-529)
/** The source sentence (English) of OMX_REPLY_PLUGIN_HOSTD_DESCRIPTOR_ABSENT. */
#define OMX_REPLY_PLUGIN_HOSTD_DESCRIPTOR_ABSENT_TEXT_EN "There is no description of this plugin yet, so its panel cannot be shown; add the plugin again so it is scanned and described."
#define OMX_REPLY_PLUGIN_HOSTD_DESCRIPTOR_PENDING "reply.plugin-hostd.descriptor-pending" ///< the scan its add started has not written the descriptor (panel) yet; of description_get, the plugin's ask still runs
#define OMX_REPLY_PLUGIN_HOSTD_DESCRIPTOR_PENDING_RESP (-530)
/** The source sentence (English) of OMX_REPLY_PLUGIN_HOSTD_DESCRIPTOR_PENDING. */
#define OMX_REPLY_PLUGIN_HOSTD_DESCRIPTOR_PENDING_TEXT_EN "The plugin is still being scanned or described; try again in a moment."
#define OMX_REPLY_PLUGIN_HOSTD_DESCRIPTOR_SCAN_FAILED "reply.plugin-hostd.descriptor-scan-failed" ///< the plugin's last scan wrote no descriptor (panel)
#define OMX_REPLY_PLUGIN_HOSTD_DESCRIPTOR_SCAN_FAILED_RESP (-531)
/** The source sentence (English) of OMX_REPLY_PLUGIN_HOSTD_DESCRIPTOR_SCAN_FAILED. */
#define OMX_REPLY_PLUGIN_HOSTD_DESCRIPTOR_SCAN_FAILED_TEXT_EN "The plugin could not be scanned, so its panel is not available; check that it works, then add it again."
#define OMX_REPLY_PLUGIN_HOSTD_NO_SUCH_PLAN "reply.plugin-hostd.no-such-plan" ///< a chain no plan verb planned
#define OMX_REPLY_PLUGIN_HOSTD_NO_SUCH_PLAN_RESP (-532)
/** The source sentence (English) of OMX_REPLY_PLUGIN_HOSTD_NO_SUCH_PLAN. */
#define OMX_REPLY_PLUGIN_HOSTD_NO_SUCH_PLAN_TEXT_EN "There is no plan for this chain; make one first."
#define OMX_REPLY_PLUGIN_HOSTD_NO_RECORD "reply.plugin-hostd.no-record" ///< no qualification record names the plugin
#define OMX_REPLY_PLUGIN_HOSTD_NO_RECORD_RESP (-533)
/** The source sentence (English) of OMX_REPLY_PLUGIN_HOSTD_NO_RECORD. */
#define OMX_REPLY_PLUGIN_HOSTD_NO_RECORD_TEXT_EN "This plugin has not been measured yet, so its cost is not known; run its qualification first."
#define OMX_REPLY_PLUGIN_HOSTD_NO_CELL "reply.plugin-hostd.no-cell" ///< the plugin's qualification records hold no cost and latency at that rate and quantum
#define OMX_REPLY_PLUGIN_HOSTD_NO_CELL_RESP (-534)
/** The source sentence (English) of OMX_REPLY_PLUGIN_HOSTD_NO_CELL. */
#define OMX_REPLY_PLUGIN_HOSTD_NO_CELL_TEXT_EN "This plugin has no measurement at this sample rate and block size; measure it with these settings, or change them."
#define OMX_REPLY_PLUGIN_HOSTD_UNKNOWN_PLUGIN "reply.plugin-hostd.unknown-plugin" ///< the uri names no plugin installed here
#define OMX_REPLY_PLUGIN_HOSTD_UNKNOWN_PLUGIN_RESP (-535)
/** The source sentence (English) of OMX_REPLY_PLUGIN_HOSTD_UNKNOWN_PLUGIN. */
#define OMX_REPLY_PLUGIN_HOSTD_UNKNOWN_PLUGIN_TEXT_EN "No plugin with that address is installed here; install it, or choose another."
#define OMX_REPLY_PLUGIN_HOSTD_CURVE_KIND_UNKNOWN "reply.plugin-hostd.curve-kind-unknown" ///< curve_bind of a family the display's kind is not drawn for (an eq_response is the eq family's, a time_response the delay's or the reverb's, a transfer_curve every other's), or curve_points of a binding whose values pick a band shape, a head filter slope, a reverb algorithm or a delay division with no closed form
#define OMX_REPLY_PLUGIN_HOSTD_CURVE_KIND_UNKNOWN_RESP (-536)
/** The source sentence (English) of OMX_REPLY_PLUGIN_HOSTD_CURVE_KIND_UNKNOWN. */
#define OMX_REPLY_PLUGIN_HOSTD_CURVE_KIND_UNKNOWN_TEXT_EN "This curve cannot be drawn for that kind of plugin or for the shape or slope it is set to; choose another shape or slope."
#define OMX_REPLY_PLUGIN_HOSTD_CURVE_TERM_MISSING "reply.plugin-hostd.curve-term-missing" ///< curve_points of a display with no binding, or of a binding that lacks a term the model needs
#define OMX_REPLY_PLUGIN_HOSTD_CURVE_TERM_MISSING_RESP (-537)
/** The source sentence (English) of OMX_REPLY_PLUGIN_HOSTD_CURVE_TERM_MISSING. */
#define OMX_REPLY_PLUGIN_HOSTD_CURVE_TERM_MISSING_TEXT_EN "The curve has nothing bound to draw from yet; bind the display to the plugin's controls first."
#define OMX_REPLY_PLUGIN_HOSTD_CURVE_UNIT_REFUSED "reply.plugin-hostd.curve-unit-refused" ///< curve_bind of a term in a unit its role does not take
#define OMX_REPLY_PLUGIN_HOSTD_CURVE_UNIT_REFUSED_RESP (-538)
/** The source sentence (English) of OMX_REPLY_PLUGIN_HOSTD_CURVE_UNIT_REFUSED. */
#define OMX_REPLY_PLUGIN_HOSTD_CURVE_UNIT_REFUSED_TEXT_EN "A control was bound in a unit that part of the curve does not take; bind it in the unit that part expects."
#define OMX_REPLY_PLUGIN_HOSTD_CURVE_POINTS_RANGE "reply.plugin-hostd.curve-points-range" ///< curve_points of fewer than 2 points or more than the most a curve carries, an EQ's three anchors a drawn band counted, or of a band index past what fits
#define OMX_REPLY_PLUGIN_HOSTD_CURVE_POINTS_RANGE_RESP (-539)
/** The source sentence (English) of OMX_REPLY_PLUGIN_HOSTD_CURVE_POINTS_RANGE. */
#define OMX_REPLY_PLUGIN_HOSTD_CURVE_POINTS_RANGE_TEXT_EN "The curve was asked for too few or too many points, or for a band that is not there; ask for fewer points or another band."
#define OMX_REPLY_PLUGIN_HOSTD_DURABLE_WRITE_FAILED "reply.plugin-hostd.durable-write-failed" ///< the daemon's write of a file the verb changes (the history, a pin file) failed on its writer thread: the change is not on disk
#define OMX_REPLY_PLUGIN_HOSTD_DURABLE_WRITE_FAILED_RESP (-540)
/** The source sentence (English) of OMX_REPLY_PLUGIN_HOSTD_DURABLE_WRITE_FAILED. */
#define OMX_REPLY_PLUGIN_HOSTD_DURABLE_WRITE_FAILED_TEXT_EN "The change could not be saved to disk, so it will not be there after a restart; check the free space and try again."
#define OMX_REPLY_PLUGIN_HOSTD_WRITER_QUEUE_FULL "reply.plugin-hostd.writer-queue-full" ///< the daemon's writer queue holds its most writes (64) or bytes (64 MiB): the write the verb needs was refused, and nothing of it changed
#define OMX_REPLY_PLUGIN_HOSTD_WRITER_QUEUE_FULL_RESP (-541)
/** The source sentence (English) of OMX_REPLY_PLUGIN_HOSTD_WRITER_QUEUE_FULL. */
#define OMX_REPLY_PLUGIN_HOSTD_WRITER_QUEUE_FULL_TEXT_EN "Too many changes are waiting to be saved, so this one was not made; wait a moment and try again."
#define OMX_REPLY_PLUGIN_HOSTD_CURVE_VALUE_NOT_FINITE "reply.plugin-hostd.curve-value-not-finite" ///< curve_points of a binding whose value, as the instance holds it, is not finite, or of an instance that runs at no rate: no curve is drawn
#define OMX_REPLY_PLUGIN_HOSTD_CURVE_VALUE_NOT_FINITE_RESP (-542)
/** The source sentence (English) of OMX_REPLY_PLUGIN_HOSTD_CURVE_VALUE_NOT_FINITE. */
#define OMX_REPLY_PLUGIN_HOSTD_CURVE_VALUE_NOT_FINITE_TEXT_EN "The curve cannot be drawn because a value of the plugin is not a number, or the plugin is not running; start it or set the value again."
#define OMX_REPLY_PLUGIN_HOSTD_OUTPUT_NOT_MONITORED "reply.plugin-hostd.output-not-monitored" ///< monitor_output_off of an output this connection did not monitor: a monitor ends with the connection that set it, and only that connection ends it earlier
#define OMX_REPLY_PLUGIN_HOSTD_OUTPUT_NOT_MONITORED_RESP (-543)
/** The source sentence (English) of OMX_REPLY_PLUGIN_HOSTD_OUTPUT_NOT_MONITORED. */
#define OMX_REPLY_PLUGIN_HOSTD_OUTPUT_NOT_MONITORED_TEXT_EN "That output is not being watched on this connection, so there is nothing to stop."

/* PLUGIN_HOSTD_PROTOCOL_EXTENSIONS (contract.d/plugin-hostd-protocol/plugin-hostd-protocol.json) */
#define OMX_PLUGIN_HOSTD_PROTOCOL_EXTENSIONS_TAGS "tags" ///< request tags: a line may start with \@\<tag\>, and its answer then starts with the same \@\<tag\>
#define OMX_PLUGIN_HOSTD_PROTOCOL_EXTENSIONS_ERRORS "errors" ///< error words: a refusal resp \<n\> is followed by the stable word of its code, reply.\<group\>.\<reason\>, when the protocol declares one
#define OMX_PLUGIN_HOSTD_PROTOCOL_EXTENSIONS_SEQ "seq" ///< event sequence numbers: every feedback line starts with #\<n\>, 1 for the first after this hello and one more for each, so a gap is a missed line
/** The number of words of PLUGIN_HOSTD_PROTOCOL_EXTENSIONS. */
#define OMX_PLUGIN_HOSTD_PROTOCOL_EXTENSIONS_COUNT 3u
/** The words of PLUGIN_HOSTD_PROTOCOL_EXTENSIONS in their declared order, as a C initializer list. */
#define OMX_PLUGIN_HOSTD_PROTOCOL_EXTENSIONS_INIT { "tags", "errors", "seq" }
/** Expands `X(code, "word", NAME)` once per word of PLUGIN_HOSTD_PROTOCOL_EXTENSIONS, in order, the code being its place in the order. The extensions hello can grant, each a word a client names after its version. A session that was granted none is answered as mod-host answers it. */
#define OMX_PLUGIN_HOSTD_PROTOCOL_EXTENSIONS(X) \
    X(0u, "tags", TAGS) X(1u, "errors", ERRORS) X(2u, "seq", SEQ)

#endif /* PLUGIN_HOSTD_PROTOCOL_VOCABULARY_H */
