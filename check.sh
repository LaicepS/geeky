#!/bin/bash

source venv/bin/activate

set -euo pipefail

for dir in cgi scrapper ocr
do
  python3 -m unittest discover -s "$dir"/ -p "*.py"
done
