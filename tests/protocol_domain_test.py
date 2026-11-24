# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
"""The words of the protocol (the daemon's verbs, the protocol's own verbs, the reply codes only plugin-hostd answers and
the extensions hello grants) are the domain plugin-hostd-protocol of this package, contract.d/plugin-hostd-protocol/plugin-hostd-protocol.json,
and are read from there by every reader of this tree (plugin-hostd-protocol#26, the move out of data/verbs.json):
  (a) the domain holds the four items, in the number of words they had, and data/verbs.json does not exist;
  (b) every reply code of the domain is in the protocol's table with its number, and every verb of it is a command of
      the table (the tools read the domain);
  (c) the Python codec, given a contract directory that holds nothing but this domain, gives a reply code its sentence and
      names this package its owner: the sentence is read from the domain;
  (d) a stale 3/plugin-hostd-protocol/verbs.json beside the domain, with a sentence of its own, is never read: the codec
      reads the domains alone, so the domain's sentence is the one given.

  protocol_domain_test.py"""
import os
import shutil
import sys
import tempfile
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
sys.path.insert(0, os.path.join(ROOT, "tools"))
sys.path.insert(0, os.path.join(ROOT, "clients", "python"))
import json  # noqa: E402

try:
    import protocol_words  # noqa: E402
except ImportError:  # the reader of the domain does not exist yet
    protocol_words = None
from plugin_hostd_protocol import Protocol  # noqa: E402

GOLDEN = {"PLUGIN_HOSTD_PROTOCOL_REPLY_CODES": ("codes", 39), "PLUGIN_HOSTD_PROTOCOL_DAEMON_VERBS": ("verbs", 30),
          "PLUGIN_HOSTD_PROTOCOL_LIBRARY_VERBS": ("verbs", 6), "PLUGIN_HOSTD_PROTOCOL_EXTENSIONS": ("words", 3)}
TABLE = os.path.join(ROOT, "protocol", "plugin-hostd-protocol.json")


DOMAIN = "contract.d/plugin-hostd-protocol/plugin-hostd-protocol.json"


class ProtocolDomain(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        if not os.path.isfile(os.path.join(ROOT, DOMAIN)):
            raise AssertionError("%s does not exist: the words of the protocol are still in data/verbs.json" % DOMAIN)
        if protocol_words is None:
            raise AssertionError("tools/protocol_words.py does not exist: no tool reads the domain")

    def test_a_the_domain_holds_the_four_items_and_the_data_file_is_gone(self):
        self.assertTrue(os.path.isfile(os.path.join(ROOT, DOMAIN)))
        self.assertFalse(os.path.exists(os.path.join(ROOT, "data", "verbs.json")), "data/verbs.json is the old home")
        items = protocol_words.items()
        self.assertEqual(list(items), list(GOLDEN))
        for name, (kind, count) in GOLDEN.items():
            self.assertEqual(items[name]["kind"], kind, name)
            self.assertEqual(len(items[name][kind]), count, name)

    def test_b_the_table_is_written_from_the_domain(self):
        with open(TABLE) as f:
            text = f.read()
        table = json.loads(text)
        resp = {r["code"]: r["resp"] for r in table["reply_codes"]}
        for name, code in protocol_words.items()["PLUGIN_HOSTD_PROTOCOL_REPLY_CODES"]["codes"].items():
            self.assertEqual(resp.get(code["code"]), code["resp"], name)
        commands = {c["verb"]: c for c in table["commands"]}
        for group in ("PLUGIN_HOSTD_PROTOCOL_DAEMON_VERBS", "PLUGIN_HOSTD_PROTOCOL_LIBRARY_VERBS"):
            for key, v in protocol_words.items()[group]["verbs"].items():
                self.assertIn(v["verb"], commands, key)
                self.assertIn(DOMAIN + " " + group + "." + key, text, key)

    def test_c_the_codec_reads_the_sentences_from_the_domain_alone(self):
        d = tempfile.mkdtemp(prefix="protocol-domain-")
        try:
            os.makedirs(os.path.join(d, "plugin-hostd-protocol"))
            shutil.copy(os.path.join(ROOT, DOMAIN), os.path.join(d, "plugin-hostd-protocol", "plugin-hostd-protocol.json"))
            p = Protocol(table=TABLE, contract_dirs=[d])
            name = "reply.plugin-hostd.no-cell"
            self.assertEqual(p.owners[name], "plugin-hostd-protocol")
            want = protocol_words.items()["PLUGIN_HOSTD_PROTOCOL_REPLY_CODES"]["codes"]["noCell"]["text"]["en"]
            self.assertEqual(p.sentences[name], want)
        finally:
            shutil.rmtree(d)

    def test_d_a_stale_verbs_file_beside_the_domain_is_never_read(self):
        d = tempfile.mkdtemp(prefix="protocol-domain-")
        try:
            name = "reply.plugin-hostd.no-cell"
            os.makedirs(os.path.join(d, "plugin-hostd-protocol"))
            shutil.copy(os.path.join(ROOT, DOMAIN), os.path.join(d, "plugin-hostd-protocol", "plugin-hostd-protocol.json"))
            stale = os.path.join(d, "3", "plugin-hostd-protocol")
            os.makedirs(stale)
            code = protocol_words.items()["PLUGIN_HOSTD_PROTOCOL_REPLY_CODES"]["codes"]["noCell"]
            with open(os.path.join(stale, "verbs.json"), "w") as f:
                json.dump({"PLUGIN_HOSTD_PROTOCOL_REPLY_CODES": {"kind": "codes", "codes": {"noCell": {
                    "code": name, "resp": code["resp"], "text": {"en": "A stale sentence of the old home."}}}}}, f)
            p = Protocol(table=TABLE, contract_dirs=[d])
            self.assertEqual(p.sentences[name], code["text"]["en"], "the sentence of a stale verbs.json was read")
            self.assertEqual(p.owners[name], "plugin-hostd-protocol")
        finally:
            shutil.rmtree(d)


if __name__ == "__main__":
    unittest.main()
