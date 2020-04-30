#!/bin/bash

set -euo pipefail

mkdir -p build
pushd build
cmake ..
make -j$(( $(nproc) + 1 ))
popd

tests/end2end.sh
