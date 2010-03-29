#!/bin/bash

TZFILE=~/public/inbox/btajc.tz
MYDATE=`date +%Y%m%d_%H%M%S`

#======================================================================
# First check existence of running jobs; if done,
# Clean up and put results in outbox
#
BATCHDIR=~/scratch0/data/batchdir
cd ${BATCHDIR}
NRUNNINGJOBS=`ls | grep -c btaoutput`
if (( ${NRUNNINGJOBS} ))
then
    for dir in `ls -d btaoutput*`
    do
      NJOBSNOTFINISHED=`ls -a $dir | grep -c "\.btacronmaster_activejob"`
      if (( ${NJOBSNOTFINISHED} == 0 ))
      then
	  echo "$MYDATE: Cleaning up.. " ${dir}
	  tar czvf ${dir}.tz ${dir}/* >/dev/null
	  mv ${dir}.tz ~/public/outbox
	  rm -rf ${dir}
	  echo "$MYDATE: Done with " ${dir}
      fi
    done
fi

#Now check existence of new jobs to run
if [[ ! -e ${TZFILE} ]] 
then
    exit
fi

JCFILE=btajc.rc
CMSREL=CMSSW_3_3_6
#
WRKDIR=${BATCHDIR}/btaoutput_${MYDATE}
PROJDIR=~/scratch0/${CMSREL}/src/MyEDmodules/HcalDelayTuner/
PYDIR=${PROJDIR}/python
mkdir ${WRKDIR}
cd ${WRKDIR}
tar xzvf ${TZFILE} >/dev/null
rm ${TZFILE}

echo "============================================"
echo "$MYDATE: found job control file, processing."

#
#======================================================================
# Process the incoming job specification tarball
#
cd ${WRKDIR}
tar xzvf ${TZFILE} >/dev/null
rm ${TZFILE}

if [[ ! -e ${JCFILE} ]] 
then
    echo "$MYDATE: ${JCFILE} absent from ${TZFILE}, aborting."
    exit
fi

source ${JCFILE}
rm ${JCFILE}

#
#======================================================================
# Check for emailing status messages
#
if (( ${#EMAILLOG2} > 0 ))
then
    echo "$MYDATE: Mailing to " ${EMAILLOG2}
fi
#
#======================================================================
# Make a cfi file from the specified dataset
#
if (( ${#DATASET} > 0 ))
then
    for run in ${RUNS}
    do
      echo "$MYDATE: Making cfi for run #${run}"
      ~/private/bin/makelistpy4castor.sh ${DATASET} ${run} >${PYDIR}/run${run}files_cfi.py
      if (( `grep -c root ${PYDIR}/run${run}files_cfi.py` == 0 ))
      then
	  echo "$MYDATE: No files found for run ${run} in dataset ${DATASET}! " 
	  exit
      fi
    done
fi
#
#======================================================================
# set up CMSSW environment
#
source /afs/cern.ch/cms/sw/cmsset_default.sh
cd ${PROJDIR}
#cmsenv
eval `scramv1 ru -sh`
cd ${WRKDIR}
#
#======================================================================
# Check for User request for skimming
#
if (( ${#SKIMFILE} > 0 ))
then
    mv ${WRKDIR}/${SKIMFILE} temp.rc
    for run in ${RUNS}
    do
#     PREFIX=myskim_run${run}_pool
      echo "$MYDATE: Running skim file ${SKIMFILE}"
      cp temp.rc skim_setup_${run}.rc
      echo "RUN=${run}" >>skim_setup_${run}.rc
      ${PROJDIR}/test/runSkimBatch.sh skim_setup_${run}.rc
#     if [[ -e ${WRKDIR}/${SKIMOUTPUT} ]]
#     then
#	  echo "$MYDATE: Moving skim output to castor..."
#	  rfcp ${WRKDIR}/${SKIMOUTPUT} ${CASTORSKIMLOC}
#	  rm $WRKDIR/${SKIMOUTPUT}
#	  echo "$MYDATE: done."
#	  makelistpy4afs.sh "rfio:${CASTORSKIMLOC}/${SKIMOUTPUT}" >${PYDIR}/${PREFIX}_cfi.py
#     fi
    done
    rm temp.rc
    exit
fi
#
#======================================================================
# Check for User request for timing analysis
#
if (( ${#ANALFILE} > 0 ))
then
    cp ${WRKDIR}/${ANALFILE} temp.rc
    for run in ${RUNS}
    do
#     PREFIX=myskim_run${run}_pool
      echo "$MYDATE: Running anal file " ${ANALFILE} " on run ${run}"
      cp temp.rc anal_setup.rc
      echo "RUN=${run}" >>anal_setup.rc
      echo "RUNMODE=BATCH" >>anal_setup.rc
      BATCH_SEMA4=${WRKDIR}/.btacronmaster_activejob_${run}
      touch ${BATCH_SEMA4}
      echo "BATCH_SEMA4=${BATCH_SEMA4}" >>anal_setup.rc
      ${PROJDIR}/test/runBeamAnalBatch.sh
    done
    rm temp.rc
    exit
fi
#
#======================================================================
# Check for User request for plotting
#
if (( ${#PLOTFILE} > 0 ))
then
    echo "$MYDATE: Plotting " ${PLOTFILE}
fi
