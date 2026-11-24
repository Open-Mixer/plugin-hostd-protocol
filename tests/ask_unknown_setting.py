# SPDX-License-Identifier: GPL-3.0-or-later
# The descriptions store's ask is set by ask_unknown (omx-panel docs/descriptions-store.md section 7): protocol/plugin-hostd.json
# declares it once, as a string setting whose default is auto, with no environment variable, and its meaning names each of
# its words, auto, 1 and 0, and the add word that overrides it. The daemon reads only what the protocol declares (R-095).
import json, sys

document = json.load(open(sys.argv[1]))
setting = [c for c in document["config"] if c["key"] == "ask_unknown"]
if len(setting) != 1:
    sys.exit("FAIL %s declares ask_unknown %d times" % (sys.argv[1], len(setting)))
row = setting[0]
if row["type"] != "string" or row["default"] != "auto" or row["env"] != "":
    sys.exit("FAIL ask_unknown is %r: a string setting, default auto, no environment variable" % row)
words = row["meaning"].replace(",", " ").replace(":", " ").replace(";", " ").split()
for word in ("auto", "1", "0", "ask", "no-ask", "plugin-qualify-sync"):
    if word not in words:
        sys.exit("FAIL the meaning of ask_unknown does not name %s: %r" % (word, row["meaning"]))
print("ok   ask_unknown is declared once, default auto, and names auto, 1, 0 and the add word")
