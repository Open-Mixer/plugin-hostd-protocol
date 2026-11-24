# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
"""tests/no_installed_contract.py: the environment under which tools/domain_path.py searches one empty contract directory and
nothing installed, for the arms that ask a generator for a domain no directory holds (a refusal naming it).

tools/domain_path.py searches OMX_CONTRACT_PATH, then the installed contract directory pkg-config names. A box with omx-contract
installed holds real domains there, so the arm points pkg-config at a scratch omx-contract.pc whose contractdir is the empty
directory, by PKG_CONFIG_LIBDIR AND PKG_CONFIG_PATH: pkg-config reads PKG_CONFIG_PATH first, and rpmbuild's %check exports
PKG_CONFIG_PATH=...:/usr/lib64/pkgconfig:/usr/share/pkgconfig, which answered the real directory and turned the v0.8.0 release
red (#146). searched() reads what domain_path.py would search under that environment, so an arm proves the installed
directory is held out before it believes a refusal."""
import os, subprocess, sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def env(tmp, empty):
    """The variables (to add to os.environ, beside OMX_CONTRACT_PATH=empty) under which the installed contract directory is
    `empty` too."""
    pc = os.path.join(tmp, "no-installed-contract-pc")
    os.makedirs(pc, exist_ok=True)
    with open(os.path.join(pc, "omx-contract.pc"), "w") as f:
        f.write("contractdir=%s\nName: omx-contract\nDescription: scratch\nVersion: 0\n" % empty)
    return {"PKG_CONFIG_LIBDIR": pc, "PKG_CONFIG_PATH": pc}


def searched(contract_path, extra):
    """The contract directories tools/domain_path.py searches under os.environ, OMX_CONTRACT_PATH=contract_path and `extra`."""
    code = "import sys; sys.path.insert(0, %r); import domain_path; print('\\n'.join(domain_path.contract_dirs()))" % os.path.join(ROOT, "tools")
    r = subprocess.run([sys.executable, "-B", "-c", code], env=dict(os.environ, OMX_CONTRACT_PATH=contract_path, **extra), capture_output=True, text=True)
    return r.stdout.splitlines()
