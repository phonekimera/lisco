#! /usr/bin/env perl

use strict;

use Test::More;
use IPC::Open2;

my ($epdfile, @commands) = @ARGV;

if (!defined $epdfile || !length $epdfile) {
	die "Usage: $@ EPDFILE[, COMMANDS...]"
}

# Find lisco executable.
my $lisco = "../liscoold";
$lisco = "../liscoold.exe" unless -e $lisco;

open STDERR, ">&STDOUT";

push @commands, "epdfile $epdfile", "quit";

my ($cout, $cin);
my $pid = open2 $cout, $cin, $lisco
	or die "cannot exec '$lisco': $!";

foreach my $cmd (@commands) {
	$cin->print("$cmd\n");
}

my $header_seen;
my $failures = 0;
while (1) {
	my $line = $cout->getline;
	last if !defined $line;

	print STDERR "$line";
	if (!$header_seen) {
		if ($line =~ /^ *\| *Solution *\| *Refuted *\| *R *\| *Name *\| *\n$/) {
			$header_seen = 1;
		}
	} else {
		if ($line =~ /^ *\| *([0-9]+) *\| *([0-9.]+) *\| *([0-9]+) *\| *([0-9.]+) *\| *(X|-) *\| *(.+?) *\| *\n$/) {
			my ($sdepth, $stime, $rdepth, $rtime, $solved, $name) =
				($1, $2, $3, $4, $5, $6);
			ok 'X' eq $solved, $name;
			++$failures if 'X' ne $solved;
		}
	}
}

waitpid $pid, 0;
ok !$?;

done_testing;

my $success = $header_seen && $failures == 0;

exit !$success;
