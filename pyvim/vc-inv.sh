#!/bin/sh

# ----------------------------------------------------------------------
#
# 2024-09-11: Shell script wrapper for python to pick up virtual environment
#
# ----------------------------------------------------------------------

HBINV=$HOME/hb-inv/

source $HBINV/.venv/bin/activate
$HBINV/vc-inv.py -c $HBINV/config.yaml $@
