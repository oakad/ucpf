#!/bin/bash

sed -n -e '1i\socket' -e 's/^#define\s\+SOL_\(\w\+\).*/\L\1/p'
