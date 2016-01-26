#!/bin/sh

rgrep -c TODO * | grep -v :0 | sort -g -t':' -k2
