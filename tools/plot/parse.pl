#!/usr/bin/perl

use strict;
use 5.010;

my %count;
my $file = shift or die "Usage: $0 FILE\n";

my $bytes_sent_file = 'bandwidth.txt';
open my $bytes_sent_fh, '>', $bytes_sent_file or die "Could not open file '$bytes_sent_file' $!";

my $block_event_file = 'blockevent.txt';
open my $block_event_fh, '>', $block_event_file or die "Could not open file '$bytes_sent_file' $!";

open my $fh, '<', $file or die "Could not open '$file' $!";

my $line = <$fh>;
$line = <$fh>; #second line

$line =~ /([0-9]+)\:([0-9]+)\.([0-9]+)\sI\s-\s/;

my $minute = $1;
my $second = $2;
my $millisecond = $3;

my $start = ($1 * 60  + $2) * 1000 + $3 * 10;
my $now;
my $nnodes;

print "start time: $start";
print "\n";

my $timeslot = 0;

my %blocks_sent;

my @total_bytes = ();
for(my $i=0; $i<60; $i++) {
	my %total_bytes_per_node;
	push (@total_bytes, %total_bytes_per_node);
}

my @request_bytes = ();
for (my $i=0; $i<60; $i++) {
	my %request_bytes_per_node;
	push (@request_bytes, %request_bytes_per_node);
}

my @block_bytes = ();
for (my $i=0; $i<60; $i++) {
	my %block_bytes_per_node;
	push (@block_bytes, %block_bytes_per_node);
}

my @interest_bytes = ();
for (my $i=0; $i<60; $i++) {
	my %interest_bytes_per_node;
	push (@interest_bytes, %interest_bytes_per_node);
}

my @meta_bytes = ();
for (my $i=0; $i<60; $i++) {
	my %meta_bytes_per_node;
	push (@meta_bytes, %meta_bytes_per_node);
}

while ($line = <$fh>) {
	if ($line =~ /Created\s(d+)\snodes/) {
		$nnodes = $1;
		print $nnodes;
	} else {
		$line =~ /([0-9]+)\:([0-9]+)\.([0-9]+)\s/;
		$now = ($1 * 60  + $2) * 1000 + $3 * 10;
		if ($now - $start > ($timeslot+1) * 1000) {
			$timeslot++;
		} else {
			if ($line=~ /Send block/) {
			   $blocks_sent{$timeslot}++;
		    } elsif ($line=~ /n\(([0-9]+)\)\s+total bytes sent\:\s+([0-9]+)/) {
			    $total_bytes[$1]{$timeslot}+=$2;
		    } elsif ($line=~ /n\(([0-9]+)\)\s+bytes sent on request\:\s+([0-9]+)/) {
			    $request_bytes[$1]{$timeslot}+=$2;
		    } elsif ($line=~ /n\(([0-9]+)\)\s+bytes sent on block\:\s+([0-9]+)/) {
			    $block_bytes[$1]{$timeslot}+=$2;
		    } elsif ($line=~ /n\(([0-9]+)\)\s+bytes sent on interest\:\s+([0-9]+)/) {
			    $interest_bytes[$1]{$timeslot}+=$2;
		    } elsif ($line=~ /n\(([0-9]+)\)\s+bytes sent on meta\:\s+([0-9]+)/) {
			    $meta_bytes[$1]{$timeslot}+=$2;
		    }
		}
	}
}

for my $slot (sort {$a<=>$b} keys %blocks_sent) {
	print $block_event_fh "$slot\t$blocks_sent{$slot}\n";
}

for(my $i=0; $i<60; $i++) {
	print $bytes_sent_fh "node $i:\n";
	for my $slot (sort {$a<=>$b} keys $total_bytes[$i]) {
		print $bytes_sent_fh "$slot\t$total_bytes[$i]{$slot}\t";
		print $bytes_sent_fh "$request_bytes[$i]{$slot}\t";		
		print $bytes_sent_fh "$block_bytes[$i]{$slot}\t";		
		print $bytes_sent_fh "$interest_bytes[$i]{$slot}\t";
		print $bytes_sent_fh "$meta_bytes[$i]{$slot}\n";						
    }
	print $bytes_sent_fh "\n";
}


#my ($starttime) = ($1);
#print "$starttime";

#while (my $line = <$fh>) {
	

#	chomp $line;
#	my @words = split(/ /,$line);
#    if word
#}

#for my $i (sort keys %count) {
#	print f "%-31s %s\n", $i, $count{$i};
#}