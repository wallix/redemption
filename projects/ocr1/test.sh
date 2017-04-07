#! /bin/bash

files=$(find -L train -maxdepth 1 -name '*.ppm')

for f in $files; do
    echo "Processing: $f."
    extract_text $f
done
