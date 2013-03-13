#!/bin/bash

#for file in $*
#for run in 196416 196280 196158 195979 195781 195675 195468 195406 195323 195225 195169 195061 195020 194941 194848 194801 194738 194549 194500 194328 194236 194163 194055 193733 193644 193590 193512 193382 193243 193156
#for run in 192848
for run in 194549 194738
do
  file="_windows/scanphases_run${run}.log"
  echo $file
  #run=`echo $file | egrep -o "[0-9]+"`
  outfile=perchanwin_run${run}.txt
  grep Optimal $file | sed 's#[a-zA-Z_,=;\.]##g' | sed 's#:# #g'  |colrm 17 |sed -n '/.\{15,\}/p' >$outfile
done
