#!/bin/bash

set -euo pipefail

python3 -m unittest discover -s cgi/ -p "*.py"
python3 -m unittest discover -s scrapper/ -p "*.py"
python3 -m unittest discover -s ocr/ -p "*.py"

