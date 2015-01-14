#!/bin/bash

sed -n -e 's/^#define\s\+AF_\(\w\+\).*/\L\1/p' | sed  -e '1d;$d'
