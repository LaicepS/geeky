#!/bin/bash

set -euo pipefail

#pushd api_server
#./manage.py test
#popd
#
for dir in cgi scraper ocr
do
  python3 -m unittest discover -s "$dir"/ -p "*.py"
done
