#!/usr/bin/perl

use strict;
use 5.010;

my %count;
my $file = shift or die "Usage: $0 FILE\n";

my $bytes_sent_file = 'bandwidth.txt';
open my $bytes_sent_fh, '>', $bytes_sent_file or die "Could not open file '$bytes_sent_file' $!";

my $block_event_file = 'blockevent.txt';
open my $block_event_fh, '>', $block_event_file or die "Could not open file '$bytes_sent_file' $!";

my $meta_stat_file = 'meta.txt';
open my $meta_stat_file_fh, '>', $meta_stat_file or die "Could not open file '$meta_stat_file' $!";

open my $fh, '<', $file or die "Could not open '$file' $!";

my $line = <$fh>;
$line = <$fh>; #second line

$line =~ /([0-9]+)\:([0-9]+)\.([0-9]+)\sI\s-\s/;

my $minute = $1;
my $second = $2;
my $millisecond = $3;
my $start = 0;

#my $start = ($1 * 60  + $2) * 1000 + $3 * 10;
#my $start = ($1 * 60  + $2) + $3 * 0.001;

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

my %meta_received;

while ($line = <$fh>) {
	if ($line =~ /Created\s(d+)\snodes/) {
		$nnodes = $1;
		print $nnodes;
	} else {
		$line =~ /([0-9]+)\:([0-9]+)\.([0-9]+)\s/;
#		$now = ($1 * 60  + $2) * 1000 + $3 * 10;
		$now = (($1-$minute) * 60  + $2-$second + ($3-$millisecond) * 0.001);
#		if ($now - $start < 0) {
#			print "overflow\n";
#			exit;
#		}
#		if ($now - $start > ($timeslot+1) * 1000) {
		if ($now - $start > ($timeslot+1)) {
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
		    } elsif ($line=~ /Receive meta\:\s+publisher\s+([0-9]+)\s+hops\s+([0-9]+)\s+after\s+([0-9]+)/) {
				if (exists($meta_received{$2})) {
					if ($3 < 10000) {
					  push($meta_received{$2}, int($3));
				    }
				}
				else {
					my @meta_per_hop = ();
					if ($3 < 10000) {
					  push(@meta_per_hop, int($3));
				    }
					$meta_received{$2} = \@meta_per_hop;
				}
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

for my $hop (sort{$a<=>$b} keys %meta_received) {
	print $meta_stat_file_fh "hops: $hop\n";
	my $sz = $#{$meta_received{$hop}} + 1;
	my @sorted_array = sort { $a<=>$b} @{ $meta_received{$hop} };
	for my $i ( 0 .. $#{ $meta_received{$hop} } ) {
		print $meta_stat_file_fh "$sorted_array[$i]\t";
		my $percentage = $i * 1.0 / $sz;
		print $meta_stat_file_fh "$percentage\n";
	}
	print $meta_stat_file_fh "\n";
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