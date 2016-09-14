#!/bin/bash

if [ $# -eq 0 ]
then
  echo "---------------------------------------"
  echo "  Usage: ./runHEraddam.sh run1 run2 run3 ... "
  echo "  Usage: ./runHEraddam.sh run1 events = -1"
  echo "---------------------------------------"
  exit
fi

TDCCENT=1075
TDCWINDOW=25 #TDC Mean +/- window

CORRECTEDTIMEMODCEILING=9999 #Phase skip? Subtract off 25ns if greater than
TIMEMODCEILING=9999 #Phase skip? Subtract off 25ns if greater than

FIRSTSAMPLE=2 #Simple reconstructor
SAMPLESTOADD=5 #Simple reconstructor

#RUNS=`printf \"root://cmsxrootd.fnal.gov//store/group/dpg_hcal/comm_hcal/LS1/USC_%06d.root\" $1`
RUNS=`printf \"root://cmsxrootd.fnal.gov//store/group/dpg_hcal/comm_hcal/USC/USC_%06d.root\" $1`
EVENTS=-1
#EVENTS=10

if [ $# -eq 2 ]
then
  EVENTS=$2
fi

if [ $# -gt 2 ]
then
 RUNS="\""
# RUNS="\""
 for runnum in "$@"
 do
#   RUN=`printf \"root://cmsxrootd.fnal.gov//store/group/dpg_hcal/comm_hcal/LS1/USC_%06d.root\", $runnum`
   RUN=`printf \"root://cmsxrootd.fnal.gov//store/group/dpg_hcal/comm_hcal/USC/USC_%06d.root\", $runnum`
   RUNS=${RUNS}${RUN}
 done
 RUNS=${RUNS}"\""
fi
echo ${RUNS}

CFGFILE=runHEraddam_cfg.py

cmsRun ${CFGFILE} run=$1 year=2012 2>&1 | tee ./logs/log_run$1.txt

echo " -----------------------------------------------------------------  " 
echo "   All done with run ${RUNNUMBER}!"
echo " -----------------------------------------------------------------  " 
echo "                                                                    "
