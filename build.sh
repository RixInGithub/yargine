#!/usr/bin/env bash
set -eox pipefail
gcc main.c jorkdir/jorkdir.c -lbinaryen -lreadline -o yargine.x86_64 -O3
./yargine.x86_64
