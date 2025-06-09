#!/bin/bash
set -euxo pipefail

	which /usr/bin/pip3 && /usr/bin/pip3 uninstall exodus_lexers -y || true

	/usr/bin/python3 setup.py install --force