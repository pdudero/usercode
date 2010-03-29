#!/bin/bash
RUNS=
for run in 124009 124020 124022 124023 124024 124027 124030 124230
#for run in 124009
do
    RUNS="${RUNS} ${run}myskim"
done
cat >>btajc.rc<<EOF
ANALFILE=anal_setup_2009runs.rc
RUNS="${RUNS}"
EOF
tar czvf btajc.tz btajc.rc anal_setup_2009runs.rc
mv btajc.tz ~/public/inbox
#/runBeamAnalBatch.sh ${run}myskim
