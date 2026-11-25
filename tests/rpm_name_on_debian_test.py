#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pau Aliagas <linuxnow@gmail.com>
"""tests/rpm_name_on_debian_test.py: the library's RPM name installs on Debian too (#180).

plugin-hostd-full depends on plugin-hostd-protocol, the name of this source's main RPM, in its spec and in its
debian/control alike; Debian calls the library libplugin-hostd-protocol0 by policy, so with no package or Provides of
that name the Debian plugin-hostd-full could not be installed. The name must be a Debian package of this source or a
versioned Provides (= ${binary:Version}) of one, so a versioned dependency on it is met.
Usage: rpm_name_on_debian_test.py [root]
"""
import os
import re
import sys

ROOT = sys.argv[1] if len(sys.argv) > 1 else os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
spec = open(os.path.join(ROOT, "packaging/plugin-hostd-protocol.spec"), encoding="utf-8").read()
control = open(os.path.join(ROOT, "debian/control"), encoding="utf-8").read()
name = re.search(r"^Name:\s*(\S+)", spec, re.M).group(1)
packages = re.findall(r"^Package:\s*(\S+)", control, re.M)
provides = re.findall(r"^Provides:.*?\b%s\s*\(=\s*\$\{binary:Version\}\)" % re.escape(name), control, re.M)
if name in packages or provides:
    print("ok   %s, the main RPM's name, is %s on Debian" % (name, "a package" if name in packages else "a versioned Provides"))
    sys.exit(0)
print("FAIL %s, the main RPM's name, is neither a Debian package nor a versioned Provides: a Debian dependency on it "
      "cannot be met" % name)
sys.exit(1)
