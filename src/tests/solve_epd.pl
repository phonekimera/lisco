#! /usr/bin/env perl

use strict;

use Test::More;
use IPC::Open2;

sub parse_epd;

my ($engine, $epdfile, @commands) = @ARGV;

if (!defined $epdfile || !length $epdfile) {
	die "Usage: $@ ENGINE EPDFILE[, COMMANDS...]"
}

open my $epd, '<', $epdfile or die "$epdfile: $!";
my @epd;
while (my $line = <$epd>) {
	chomp $line;
	if ($line !~ s{^
		(
			[1-8pnbrqkPNBRQKa-zA-Z/]+
			[ ]+
			[wb]
			[ ]+
			(?:-|(?:[KQkq]{1,4}))
			[ ]+
			(?:-|(?:[a-h][36]))
		)
		}{}x) {
			die "$epdfile: $.: invalid EPD\n";
	}

	my %task = (
		position => $1,
		hmvc => 0,
		fmvc => 1,
	);

	my @tagspecs = split /[ ]*;[ ]*/, $line;
	foreach my $tagspec (@tagspecs) {
		my ($tag, $value) = @_;
	}

	my $fen = "$task{position} $task{hmvc} $task{fmvc}";
}

die;

unshift @commands, 'uci';

open STDERR, ">&STDOUT";

my ($cout, $cin);
my $pid = open2 $cout, $cin, $engine
	or die "cannot exec '$engine': $!";

foreach my $cmd (@commands) {
	$cin->print("$cmd\n");
}

$cin->print("quit\n");


__END__

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
