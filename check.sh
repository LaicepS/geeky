#!/bin/bash

set -euo pipefail

find . -name "*.py" -exec python3 {} \;

