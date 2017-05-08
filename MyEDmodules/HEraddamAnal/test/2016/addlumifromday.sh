#!/bin/bash
infile=$1
rm -f _$infile
cat $infile | while read line
do
    day=`echo $line | awk '{print $1}'`
    lumi=`./addlumiday.awk dayofyear=$day data/2016-delivered-perday.csv`
    echo "$day $lumi"
    echo $line | sed "s#^$day#$day\t$lumi\t#g" >>_$infile
done
