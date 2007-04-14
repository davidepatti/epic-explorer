#!/bin/bash
cat $1 | grep -v "*" > $1_simulation_history.txt
