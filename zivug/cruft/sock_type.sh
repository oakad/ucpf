#!/bin/bash

sed -n -e '/^enum sock_type/,/^\};/{s/\s*SOCK_\(\w\+\).*/\L\1/p}'
