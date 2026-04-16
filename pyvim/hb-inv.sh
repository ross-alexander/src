#!/bin/sh

# ----------------------------------------------------------------------
#
# 2026-16-04: Remove fixed -v and replace with $@
#
# 2024-09-11: Shell script wrapper for python to pick up virtual environment
#
# ----------------------------------------------------------------------

HBINV=$HOME/hb-inv/

source $HBINV/.venv/bin/activate

$HBINV/hb-inv.py -c $HBINV/config.yaml $@

