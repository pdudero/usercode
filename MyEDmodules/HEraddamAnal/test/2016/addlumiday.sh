#!/bin/bash
#for run in `grep -v ALIAS hem2016cleaned.drv`
for run in `cat newhem.drv`
do
    day=`grep $run data/rundates2016.txt | awk '{print $2}'`
    lumi=`./addlumiday.awk dayofyear=$day data/2016-delivered-perday.csv`
    echo "$run $day $lumi"
#    grep $run fcamps2016hem.txt | sed "s#$run#$run  $day  $lumi#g" >>_fcamps2016hem.txt
    grep -w $run newhem.log | sed "s#$run#$run  $day  $lumi#g" >>_fcamps2016hem.txt
done
#for run in `grep -v ALIAS hep2016cleaned.drv`
for run in `cat newhep.drv`
do
    day=`grep $run data/rundates2016.txt | awk '{print $2}'`
    lumi=`./addlumiday.awk dayofyear=$day data/2016-delivered-perday.csv`
    echo "$run $day $lumi"
#    grep $run fcamps2016hep.txt | sed "s#$run#$run  $day  $lumi#g" >>_fcamps2016hep.txt
    grep -w $run newhep.log | sed "s#$run#$run  $day  $lumi#g" >>_fcamps2016hep.txt
done
