#!/bin/bash

cd "$(dirname $0)"

if [ rwl.dot -nt rwl.jpg ] ; then
	t=$(cat rwl.dot)
	t=${t//OpenBlock/'{'}
	t=${t//CloseBlock/'}'}
	t=${t//Semicolon/';'}
	t=${t//TargetName/'[A-Z][a-zA-Z0-9]*'}
	t=${t//Identifier/'[a-z]+'}
	t=${t//Integer/'[0-9]+'}
	t=${t//Dot/'.'}
	t=${t//Comma/','}
	t=${t//Quote/"[\\\"|']"}
	t=${t//TwoPoint/':'}
	t=${t//OpenExpr/'('}
	t=${t//CloseExpr/')'}
	t=${t//DefinedColor/'#'}
	t=${t//MathOperator/'[/*-+]'}

	dot -Tjpg rwl.dot -o rwl.jpg && \
	echo "$t" | dot -Tjpg -o rwlc.jpg
fi
