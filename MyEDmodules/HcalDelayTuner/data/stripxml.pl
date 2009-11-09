#!/usr/bin/perl
#
use Time::Local;

%mismatchOccurrences=();
%mismatch1stTime=();

$runnumber="unknown";

if (@ARGV < 1) {
#    print "Usage: $0 chainsawlog.xml\n";
#    exit(1);
    processFile("-");
} else {
    foreach $file (@ARGV) {
	processFile($file);
    }
}

sub processFile {
    my ($inp)=@_;
    #print $inp,"\n";
    open(INFILE,$inp);
    while(<INFILE>) {
	if (/NAME_LABEL>(.*)<\/NAME_LABEL>/) {
	    $rbx=$1;
	}
	elsif (/MODULE_POSITION>(\d{1})<\/MODULE_POSITION>/) {
	    $rm=$1;
	}
	elsif (/QIE_CARD_POSITION>(\d{1})<\/QIE_CARD_POSITION>/) {
	    $card=$1;
	}
	elsif (/QIE_ADC_NUMBER>(\d{1})<\/QIE_ADC_NUMBER>/) {
	    $qie=$1;
	}
	elsif (/INTEGER_VALUE>(\d+)<\/INTEGER_VALUE>/) {
	    $delay=$1;
	    print "$rbx\t$rm\t$card\t$qie\t$delay\n";
	}
    }
    close(INFILE);
}
