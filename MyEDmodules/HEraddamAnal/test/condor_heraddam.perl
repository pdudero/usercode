#!/usr/bin/perl
use File::Basename;


#------------------------

@runs2011=(182303,182304);

@runs2012=(192165,192167,193081,193082,193162,193164,193256,193261,193387,193388,193637,193639,193993,193994,194172,194173,194241,194242,194339,194344,194447,194559,194563,194864,194866,194955,194957,194958,195032,195033,195071,195072,195128,195129,195178,195182,195231,195232,195339,195342,195417,195418,195475,195477,195697,195703,195792,195793,195987,195988,196172,196173,196266,196267,196411,196413,196414,196478,196479,196867,196868,197147,197148,197149,197391,197397,197513,197514,197979,197989,198260,198261,198565,198568,198872,198873,199042,199061,199389,199393,199485,199491,199709,199712,199920,199926,200293,200297,200403,200404,200566,200659,200661,200722,200725,200822,200825,200885,200896,201220,201301,201754,201757,201881,201887,202357,202360,202435,202436,202637,202638,202920,202923,203439,203440,203866,203869,204255,204258,204952,204954,205279,205280,205811,205812,205933,206418,206419,206783,206785,207000,207006,207154,207156,207583,207586,207915,208060,208062,208412,208413);

@runs2012clean=(197989,198260,198261,198565,198568,198872,198873,199042,199061,199389,199393,199485,199491,199709,199712,199920,199926,200293,200297,200403,200404,200566,200659,200661,200722,200725,200822,200825,202435,202436,202637,202638,202920,202923,203439,203440,203866,203869,204255,204258,204952,204954,205279,205280,205811,205812,205933,206418,206419,206783,206785,207000,207006,207154,207156,207583,207586,207915,208060,208062,208412,208413);

@runs2016=(269643,269648,269660,270923,270927,270951,270954,271125,271126,271479,271487,272097,272099,272957,272967,273209,273211,273460,273462,273562,273563,273897,273899,274184,274185,274259,274260,274355,274357,274408,274409,274459,274462,274765,274767,275017,275019,275272,275273,275357,275359,275388,275392,275706,275707,275801,275802,275874,275877,276082,276083,276252,276256,276420,276424,276464,276465,276470,276478,276522,276523,276532,276533,276555,276556,276614,276616,276624,276626,276679,276683,276822,276823,276860,276862,276913,276914,276964,276966,277133,277134,277174,277175,277338,277340,277869,277871,278004,278008,278071,278074,278221,278223,278325,278326,278898,278901,279097,279098,279608,279610,279701,279702,279732,279734,279802,279804,279909,279910,280138,280141,280207,280204,280264,280266,280376,280378,);

@failedruns=(270923,270951,270954,272957,272967,273462,274185,274355,275388,276252,277175,278074,278223,276083,279608);

@newruns=(281364,281366,281646,281647,281736,281737,282042,282043,282777,282780,282848,282850,282929,282931,283114,283117);

#------------------------

$prodSpace="/uscmst1b_scratch/lpc1/3DayLifetime/pdudero";

$rt=$ENV{"CMSSW_VERSION"};
$pwd=$ENV{"PWD"};

if (length($rt)<2) {
	print "You must run eval `scramv1 runtime -csh` in the right release\n";
        print "before running this script!\n";
	exit(1);
}

#------------------------

$config="runHEraddam_cfg.py";
$jobBase=$config;
$jobBase=basename("$jobBase",".py");  # remove leading dirs in path and .py suffix
$jobBase=~s/_cfg$//; # remove trailing "_cfg" suffix if there

mkdir("$prodSpace/$jobBase");

$linearn=0;

srand(); # make sure rand is ready to go

#$executable=$ENV{"HOME"}."/work/batch_cmsRun";
#$executable=$ENV{"HOME"}."/work/batchit.sh";
$executable="$prodSpace/$jobBase/batchit.sh";

open(BATCHEXE,">$executable");

print BATCHEXE << "EOT";
#!/bin/sh

CMSSWVER=\$1
#OUTDIR=\$2
shift
#shift

ln -s . data

tar -xzf \${CMSSWVER}.tar.gz
cd \${CMSSWVER}
scram b ProjectRename
source /cvmfs/cms.cern.ch/cmsset_default.sh
# cmsenv
eval `scramv1 runtime -sh`
cd -

echo "\$*"
eval "\$*"

#if [ "\$OUTDIR" != "-" ]
#then
#    for file in *.root
#    do
## Recommended by Catalin (cms-t1)
#    xrdcp -f -d 0 \$file \$OUTDIR/\$file
#    done
#fi
EOT

open(SUBMIT,">sub.txt");
#open(SUBMIT,"|condor_submit");
print(SUBMIT "Executable = $executable\n");
print(SUBMIT "Universe = vanilla\n");
print(SUBMIT "Requirements =FileSystemDomain==\"fnal.gov\" && Arch==\"X86_64\"");
#print(SUBMIT " && (Machine != \"cmswn1894.fnal.gov\")");
print(SUBMIT "\n");
print(SUBMIT "Notification = ERROR\n");
print(SUBMIT "Should_Transfer_Files = YES\n");
print(SUBMIT "WhenToTransferOutput = ON_EXIT\n");
#print(SUBMIT "output_destination = file://$outputdir\n");
print(SUBMIT "x509userproxy = \$ENV(X509_USER_PROXY)\n");

#print(SUBMIT "Log = $prodSpace/$jobBase/log/\$(Cluster)_\$(Process).log\n");

$year=2016;

foreach $run (@newruns) {
    $log="$prodSpace/$jobBase/log/heraddamanal_run$run.log";
    print(SUBMIT "Output = $log\n");
    print(SUBMIT "Error = $log\n");
    print(SUBMIT "transfer_input_files = $pwd/$rt.tar.gz,$pwd/$config,$pwd/../data/rundates$year.txt,$pwd/../data/$year-delivered-perday.csv\n");
#    print(SUBMIT "Arguments = $rt $outputdir cmsRun $stub.py\n");
    print(SUBMIT "Arguments = $rt cmsRun $config run=$run year=$year\n");
    print(SUBMIT "Queue\n");
}

close(SUBMIT);

