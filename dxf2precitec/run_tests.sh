#! /usr/bin/sh

set -e

if [ ! -e venv/.is_complete ]
then
    rm -rf venv

    python3 -m venv venv
    . venv/bin/activate

    pip install ezdxf[draw]

    touch venv/.is_complete
else
    . venv/bin/activate
fi

python3 run_tests.py
