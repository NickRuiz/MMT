#!/bin/sh
#Launch your test here

wdir=$(cd $(dirname $0) ; pwd)

cd $wdir ; python ${wdir}/tagevaluator.py "$@"
