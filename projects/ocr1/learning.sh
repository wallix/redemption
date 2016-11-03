#!/bin/sh

rm -f "$1"
for f in "$2"/*.pbm ; do
  txt="${f%%.pbm}".txt
  echo "$txt"
  ./learning "$f" "$txt" "$1" || exit $?
done
