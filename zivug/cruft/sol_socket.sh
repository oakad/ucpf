#!/bin/bash

sed -n -e 's/^#define\s\+SO_\(\w\+\).*/\L\1/p'
