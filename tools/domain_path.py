# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
"""tools/domain_path.py: where another package's domain is read from. A domain <name> is the document
<contractdir>/<name>/<last segment of name>.json (omx-contract docs/contract-dir.md, "Domains"); the contract directories
are the entries of OMX_CONTRACT_PATH (a colon list, the first that holds the domain wins), then the installed one
(pkg-config --variable=contractdir omx-contract, else /usr/share/omx-contract/contract.d). A domain no directory holds is
a refusal that names the domain and the directories it looked in: nothing is read from a copy."""
import json, os, subprocess, sys


def contract_dirs():
    dirs = [d for d in os.environ.get("OMX_CONTRACT_PATH", "").split(":") if d]
    try:
        out = subprocess.run(["pkg-config", "--variable=contractdir", "omx-contract"], capture_output=True, text=True).stdout.strip()
    except OSError:
        out = ""
    return dirs + [out or "/usr/share/omx-contract/contract.d"]


def read_domain(name, who):
    leaf = name.rsplit("/", 1)[-1]
    dirs = contract_dirs()
    for d in dirs:
        path = os.path.join(d, name, leaf + ".json")
        if os.path.isfile(path):
            with open(path, encoding="utf-8") as f:
                return json.load(f)
    sys.exit("%s: no domain %s in %s" % (who, name, ", ".join(dirs)))
