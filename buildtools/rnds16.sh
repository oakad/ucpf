#!/bin/bash

od -x -N 16 -A n /dev/urandom | tr -d ' ' | tr [:lower:] [:upper:]
