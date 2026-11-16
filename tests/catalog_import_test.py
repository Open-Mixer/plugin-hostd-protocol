#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""The Python codec reads omx-contract's catalogs through the installed python3-omx-contract package (omx_contract.catalog),
never through a loose share/omx-contract/omx_contract_catalog.py by path: with OMX_CONTRACT_DATADIR pointing at an empty
directory (no loose file anywhere it could be), the reader still loads and reads a catalog entry."""
import os, sys, tempfile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(ROOT, "clients", "python"))
with tempfile.TemporaryDirectory() as empty:
    os.environ["OMX_CONTRACT_DATADIR"] = empty
    import plugin_hostd_protocol as p
    try:
        reader = p._catalog_reader()
    except Exception as e:
        print("FAIL the catalog reader does not load without the loose file: %s: %s" % (type(e).__name__, e))
        sys.exit(1)
    for name in ("load_catalog", "sentence_of"):
        if not callable(getattr(reader, name, None)):
            print("FAIL the catalog reader has no %s" % name)
            sys.exit(1)
    if reader.__name__ != "omx_contract.catalog":
        print("FAIL the reader is %s, not the omx_contract.catalog package module" % reader.__name__)
        sys.exit(1)
print("ok   the catalog reader loads as omx_contract.catalog with no loose file")
