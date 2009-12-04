#!/usr/bin/perl
#

if (@ARGV < 1) {
#    print "Usage: $0 brick*.xml\n";
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
	if (/Parameter name=\'RBX\'.*>(.*)<\/Parameter>/) {
	    $rbx=$1;
	}
	elsif (/Data.*rm=\"(\d{1})\" card=\"(\d{1})\" qie=\"(\d{1})\">(.+)<\/Data/) {
	    $rm=$1;
	    $card=$2;
	    $qie=$3;
	    $delay=$4;
	    print "$rbx\t$rm\t$card\t$qie\t$delay\n";
	}
    }
    close(INFILE);
}
