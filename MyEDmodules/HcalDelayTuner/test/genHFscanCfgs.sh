#!/bin/bash

BASENAMEFMT="bta_run%s_hfscan_delay=%d"

########################################
# RUN 132442
# cp anal_setup_hfscan.rc anal_setup.rc
# BASENAME=`printf "$BASENAMEFMT" 132442 0`
# echo "ANALOUTPUT=$BASENAME.root" >> anal_setup.rc
# echo "CFGFILE=$BASENAME.py" >> anal_setup.rc
# echo "LSRANGES=\'132442:1-132442:82\'" >> anal_setup.rc
# ./genbtapy.sh 132442

# cp anal_setup_hfscan.rc anal_setup.rc
# BASENAME=`printf "$BASENAMEFMT" 132442 24`
# echo "ANALOUTPUT=$BASENAME.root" >> anal_setup.rc
# echo "CFGFILE=$BASENAME.py" >> anal_setup.rc
# echo "LSRANGES=\'132442:87-132442:124\'" >> anal_setup.rc
# ./genbtapy.sh 132442

# cp anal_setup_hfscan.rc anal_setup.rc
# BASENAME=`printf "$BASENAMEFMT" 132442 22`
# echo "ANALOUTPUT=$BASENAME.root" >> anal_setup.rc
# echo "CFGFILE=$BASENAME.py" >> anal_setup.rc
# echo "LSRANGES=\'132442:129-132442:168\'" >> anal_setup.rc
# ./genbtapy.sh 132442

# cp anal_setup_hfscan.rc anal_setup.rc
# BASENAME=`printf "$BASENAMEFMT" 132442 20`
# echo "ANALOUTPUT=$BASENAME.root" >> anal_setup.rc
# echo "CFGFILE=$BASENAME.py" >> anal_setup.rc
# echo "LSRANGES=\'132442:173-132442:213\'" >> anal_setup.rc
# ./genbtapy.sh 132442

# cp anal_setup_hfscan.rc anal_setup.rc
# BASENAME=`printf "$BASENAMEFMT" 132442 18`
# echo "ANALOUTPUT=$BASENAME.root" >> anal_setup.rc
# echo "CFGFILE=$BASENAME.py" >> anal_setup.rc
# echo "LSRANGES=\'132442:218-132442:256\'" >> anal_setup.rc
# ./genbtapy.sh 132442

########################################
# RUN 132476
# cp anal_setup_hfscan.rc anal_setup.rc
# BASENAME=`printf "$BASENAMEFMT" 132476 16`
# echo "ANALOUTPUT=$BASENAME.root" >> anal_setup.rc
# echo "CFGFILE=$BASENAME.py" >> anal_setup.rc
# echo "LSRANGES=\'132476:29-132476:53\'" >> anal_setup.rc
# ./genbtapy.sh 132476

# # Clock errors on all FEDs (MonVis DCC-HTR)
# cp anal_setup_hfscan.rc anal_setup.rc
# BASENAME=`printf "$BASENAMEFMT" 132476 14`
# echo "ANALOUTPUT=$BASENAME.root" >> anal_setup.rc
# echo "CFGFILE=$BASENAME.py" >> anal_setup.rc
# echo "LSRANGES=\'132476:58-132476:81\'" >> anal_setup.rc
# ./genbtapy.sh 132476

########################################
# RUN 132477
# cp anal_setup_hfscan.rc anal_setup.rc
# BASENAME=`printf "$BASENAMEFMT" 132477 12`
# echo "ANALOUTPUT=$BASENAME.root" >> anal_setup.rc
# echo "CFGFILE=$BASENAME.py" >> anal_setup.rc
# echo "LSRANGES=\'132477:6-132477:33\'" >> anal_setup.rc
# ./genbtapy.sh 132477

# cp anal_setup_hfscan.rc anal_setup.rc
# BASENAME=`printf "$BASENAMEFMT" 132477 10`
# echo "ANALOUTPUT=$BASENAME.root" >> anal_setup.rc
# echo "CFGFILE=$BASENAME.py" >> anal_setup.rc
# echo "LSRANGES=\'132477:36-132477:62\'" >> anal_setup.rc
# ./genbtapy.sh 132477

# cp anal_setup_hfscan.rc anal_setup.rc
# BASENAME=`printf "$BASENAMEFMT" 132477 11`
# echo "ANALOUTPUT=$BASENAME.root" >> anal_setup.rc
# echo "CFGFILE=$BASENAME.py" >> anal_setup.rc
# echo "LSRANGES=\'132477:65-132477:90\'" >> anal_setup.rc
# ./genbtapy.sh 132477

# cp anal_setup_hfscan.rc anal_setup.rc
# BASENAME=`printf "$BASENAMEFMT" 132477 8`
# echo "ANALOUTPUT=$BASENAME.root" >> anal_setup.rc
# echo "CFGFILE=$BASENAME.py" >> anal_setup.rc
# echo "LSRANGES=\'132477:94-132477:119\'" >> anal_setup.rc
# ./genbtapy.sh 132477

# cp anal_setup_hfscan.rc anal_setup.rc
# BASENAME=`printf "$BASENAMEFMT" 132477 6`
# echo "ANALOUTPUT=$BASENAME.root" >> anal_setup.rc
# echo "CFGFILE=$BASENAME.py" >> anal_setup.rc
# echo "LSRANGES=\'132477:122-132477:147\'" >> anal_setup.rc
# ./genbtapy.sh 132477

# cp anal_setup_hfscan.rc anal_setup.rc
# BASENAME=`printf "$BASENAMEFMT" 132477 9`
# echo "ANALOUTPUT=$BASENAME.root" >> anal_setup.rc
# echo "CFGFILE=$BASENAME.py" >> anal_setup.rc
# echo "LSRANGES=\'132477:150-132477:175\'" >> anal_setup.rc
# ./genbtapy.sh 132477

# cp anal_setup_hfscan.rc anal_setup.rc
# BASENAME=`printf "$BASENAMEFMT" 132477 13`
# echo "ANALOUTPUT=$BASENAME.root" >> anal_setup.rc
# echo "CFGFILE=$BASENAME.py" >> anal_setup.rc
# echo "LSRANGES=\'132477:180-132477:224\'" >> anal_setup.rc
# ./genbtapy.sh 132477

# cp anal_setup_hfscan.rc anal_setup.rc
# BASENAME=`printf "$BASENAMEFMT" 132477 7`
# echo "ANALOUTPUT=$BASENAME.root" >> anal_setup.rc
# echo "CFGFILE=$BASENAME.py" >> anal_setup.rc
# echo "LSRANGES=\'132477:237-132477:MAX\'" >> anal_setup.rc
# ./genbtapy.sh 132477
########################################
# RUN 132658
cp anal_setup_hfscan.rc anal_setup.rc
BASENAME=`printf "$BASENAMEFMT" 132658 5`
echo "ANALOUTPUT=$BASENAME.root" >> anal_setup.rc
echo "CFGFILE=$BASENAME.py" >> anal_setup.rc
echo "LSRANGES=\'132658:1-132658:50\'" >> anal_setup.rc
./genbtapy.sh 132658

cp anal_setup_hfscan.rc anal_setup.rc
BASENAME=`printf "$BASENAMEFMT" 132658 4`
echo "ANALOUTPUT=$BASENAME.root" >> anal_setup.rc
echo "CFGFILE=$BASENAME.py" >> anal_setup.rc
echo "LSRANGES=\'132658:56-132658:120\'" >> anal_setup.rc
./genbtapy.sh 132658

cp anal_setup_hfscan.rc anal_setup.rc
BASENAME=`printf "$BASENAMEFMT" 132658 3`
echo "ANALOUTPUT=$BASENAME.root" >> anal_setup.rc
echo "CFGFILE=$BASENAME.py" >> anal_setup.rc
echo "LSRANGES=\'132658:126-132658:MAX\'" >> anal_setup.rc
./genbtapy.sh 132658
