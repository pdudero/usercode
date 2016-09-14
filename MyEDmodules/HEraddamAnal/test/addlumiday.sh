#!/bin/bash
for run in `grep -v ALIAS hem2012cleaned.drv`
do
    runlumiday=`grep $run fcamps2012.txt | colrm 21 | uniq`
    echo $runlumiday
    grep $run fcamps2012cleanedhem.txt | sed "s#$run#$runlumiday#g" >>_fcamps2012cleanedhem.txt
done
for run in `grep -v ALIAS hep2012cleaned.drv`
do
    runlumiday=`grep $run fcamps2012.txt | colrm 21 | uniq`
    echo $runlumiday
    grep $run fcamps2012cleanedhep.txt | sed "s#$run#$runlumiday#g" >>_fcamps2012cleanedhep.txt
done
