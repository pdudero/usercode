#!/bin/bash

TZFILE=~/public/inbox/btajc.tz
JCFILE=btajc.rc
CMSREL=CMSSW_3_3_6
MYDATE=`date +%Y%m%d_%H%M%S`
#
WRKDIR=~/scratch0/data/batchdir
PROJDIR=~/scratch0/${CMSREL}/src/MyEDmodules/HcalDelayTuner/
PYDIR=${PROJDIR}/python

if [[ ! -e ${TZFILE} ]] 
then
    echo "nothing to see here, move along."
    exit
fi
#
#======================================================================
# Process the incoming job specification tarball
#
cd ${WRKDIR}
tar xzvf ${TZFILE} >/dev/null
rm ${TZFILE}

if [[ ! -e ${JCFILE} ]] 
then
    echo "${JCFILE} absent from ${TZFILE}, aborting."
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
    echo "Mailing to " ${EMAILLOG2}
fi
#
#======================================================================
# Make a cfi file from the specified dataset
#
if (( ${#DATASET} > 0 ))
then
    for run in ${RUNS}
    do
      echo "Making cfi for run #${run}"
      ~/private/bin/makelistpy4castor.sh ${DATASET} ${run} >${PYDIR}/run${run}files_cfi.py
      if (( `grep -c root ${PYDIR}/run${run}files_cfi.py` == 0 ))
      then
	  echo "No files found for run ${run} in dataset ${DATASET}! " 
	  exit
      fi
    done
fi

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
      echo "Running skim file ${SKIMFILE}"
      cp temp.rc skim_setup_${run}.rc
      echo "RUN=${run}" >>skim_setup_${run}.rc
      ${PROJDIR}/test/runSkimBatch.sh skim_setup_${run}.rc
#     if [[ -e ${WRKDIR}/${SKIMOUTPUT} ]]
#     then
#	  echo "Moving skim output to castor..."
#	  rfcp ${WRKDIR}/${SKIMOUTPUT} ${CASTORSKIMLOC}
#	  rm $WRKDIR/${SKIMOUTPUT}
#	  echo "done."
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
    echo "Running anal file" ${ANALFILE}
    mv ${ANALFILE} anal_setup.rc
    ${PROJDIR}/test/runBeamAnalBatch.sh
fi
#
#======================================================================
# Check for User request for plotting
#
if (( ${#PLOTFILE} > 0 ))
then
    echo "Plotting " ${PLOTFILE}
fi
#
#======================================================================
# Clean up and put results in outbox
#
tar czvf btaoutput_${MYDATE}.tz  *.*
mv btaoutput_${MYDATE}.tz ~/public/outbox
