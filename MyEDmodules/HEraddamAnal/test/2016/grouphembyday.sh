#!/bin/bash
file=fcamps2016hemclean.txt
#
#for lumi in 5360.944 10041.01 15067.825 20217.463 24799.275 30093.022 34461.163 39896.477
for day in 104 171 187 199 220 241 253 287 302
#for day in 302
do
    echo $day
#    for ieta in -29 -28 -27 -25 -23
    for ieta in -16 -17 -18 -19 -20 -21 -22 -24 -26
    do
	for depth in 1 2 3
	do
	    depexists=`grep -c "(HE $ieta,.*,$depth)" $file`
#	    echo "$ieta	$depth	$depexists"
	    if [ $depexists -gt 0 ]
	    then
		grep " $day .*HE $ieta,.*,$depth)" $file >fcamps2016_HE${ieta}d${depth}_day${day}.txt
		mv fcamps2016_HE${ieta}d${depth}_day${day}.txt CLEAN_NODUPE_NOPROBDAYS/_HE${ieta}d${depth}
	    fi
	done
    done
done
