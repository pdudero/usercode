#!/usr/bin/perl

if (@ARGV < 2) {
    print "Usage: $0 template.py <list of cfi.py files to analyze>\n";
    exit(1);
}

$template=shift;

#------------------------

$prodSpace="/afs/cern.ch/user/d/dudero/scratch0/data";
$rt=$ENV{"LOCALRT"};

if (length($rt)<2) {
	print "You must run eval `scramv1 runtime -csh` in the right release\n";
        print "before running this script!\n";
	exit(1);
}

#------------------------

$jobBase=$template;
$jobBase=~s|^.*/||g;  # remove leading dirs in path
$jobBase=~s/[.]py$//; # remove trailing suffix
$jobBase=~s/_cfg$//;  # remove trailing "_cfg" suffix if there is one

mkdir("$prodSpace/$jobBase");
mkdir("$prodSpace/$jobBase/cfg");
mkdir("$prodSpace/$jobBase/log");

srand(); # make sure rand is ready to go

#open(SUBMIT,">condor_submit.txt");

for ($i=0; $i<=$#ARGV; $i++) {
    $jobCfg=specializeCfg($template,$jobBase,$ARGV[$i]);
    $stub=$jobCfg;
    $stub=~s|.*/([^/]+).py$|$1|;
    $log="$prodSpace/$jobBase/log/$stub.log";
    open(SUBMIT,"| bsub -q 8nh -o $log");
    print(SUBMIT "#!/bin/tcsh\n");
    print(SUBMIT "source /afs/cern.ch/cms/sw/cmsset_default.csh;\n");
    print(SUBMIT "cd $rt;\n");
    print(SUBMIT "cmsenv;\n");
    print(SUBMIT "cd -;\n");
    print(SUBMIT "cmsRun $jobCfg\n");
    close(SUBMIT);
}


sub specializeCfg($$) {
    my ($inp, $jobBase, $infname)=@_;

    $infname=~s|.*/||g; # strip leading dirs
    $infname=~s/[.]py$//; # strip suffix
    $outfname=$infname;
    $stub=$outfname."-anal";

    $mycfg="$prodSpace/$jobBase/cfg/".$stub.".py";
    open(INP,$inp);
    open(OUTP,">$mycfg");
    $sector=0;
    $depth=0;
    while(<INP>) {
	if (/\s*^#.*/) {
	    print OUTP;
	    next;
	}

	if ($sector==0 && /TFileService/) {
	    $sector=1;
	    $depth=0;
	    $trkstr=""
	}

	if ($sector==0 && /PoolOutputModule/) {
	    $sector=1;
	    $depth=0;
	    $trkstr=".untracked"
	}

	if ($sector==0 && /source.*=/) {
	    $sector=2;
	    $depth=0;
	}

	if (/in_cff/) {
	    s|in_cff|$infname|;
	}

	if ($sector==1 && /fileName/) {
	    $outfname="$prodSpace/$jobBase/$stub".".root";
	    unlink($outfname);
	    print OUTP "   fileName = cms$trkstr.string(\"$outfname\")\n";
	} else {
	    print OUTP;
	}

	$depth++ if (/\(/ && $sector!=0);
	if (/\)/ && $sector!=0) {
	    $depth--;
	    if ($depth==0) {
		$sector=0
	    }
	}
#	printf("%d %d %s",$sector,$depth,$_);
       
    }
    close(OUTP);
    close(INP);   
    return $mycfg;
}
