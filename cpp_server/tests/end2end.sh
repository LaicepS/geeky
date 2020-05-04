#!/bin/bash

set -euo pipefail

build/geeky &
pid=$$
nc localhost 8081

kill $pid
