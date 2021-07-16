#! /usr/bin/env perl

use strict;

use Test::More;
use IPC::Open2;

eval {
	require Chess::Rep;
};
if ($@) {
	warn "Please install Chess::Rep from CPAN if you want to run these tests.\n";
	exit 0;
}

sub parse_epd;
sub convert_san;

my $limit_tests = 100;

my ($engine, $epdfile, @commands) = @ARGV;

if (!defined $epdfile || !length $epdfile) {
	die "Usage: $@ ENGINE EPDFILE[, COMMANDS...]"
}

if (exists $ENV{TATE_STRESS_TEST}) {
	warn "Running full EPD '$epdfile'; unset environment variable TATE_STRESS_TEST to limit to $limit_tests.\n";
	$limit_tests = -1;
} else {
	warn "Limiting EPD tests '$epdfile' to $limit_tests; unset environment variable TATE_STRESS_TEST to run full suite.\n";
}

unshift @commands, 'uci';

open STDERR, ">&STDOUT";

my ($cout, $cin);
my $pid = open2 $cout, $cin, $engine
	or die "cannot exec '$engine': $!";

foreach my $cmd (@commands) {
	$cin->print("$cmd\n");
}

open my $epd, '<', $epdfile or die "$epdfile: $!";
my @epd;
while (my $line = <$epd>) {
	my $lineno = $.;
	my ($fen, %epd) = parse_epd $line, $epdfile, $lineno;

	if (!exists $epd{bm} && !exists $epd{am}) {
		die "$epdfile:$lineno: neither bm nor am found.\n";
	}

	my @bm;
	foreach my $bm (@{$epd{bm}}) {
		my $move = eval { convert_san $bm, $fen };
		if ($@) {
			die "$epdfile:$lineno: $@";
		}
		push @bm, $move;
	}

	my @am;
	foreach my $am (@{$epd{am}}) {
		my $move = eval { convert_san $am, $fen };
		if ($@) {
			die "$epdfile:$lineno: $@";
		}
		push @am, $move;
		print "$am => $move\n";
	}

	$cin->print("position fen $fen\ngo\n");

	my $bestmove;
	while (1) {
		my $engine_line = $cout->getline;
		#die "premature end-of-file reading from engine '$engine': $!"
		#	if !defined $engine_line;
		if ($engine_line =~ /^[ \t]*bestmove[ \t]*([^ \t\r\n]+)/i) {
			$bestmove = lc $1;
			last;
		}
	}

	if (@bm) {
		my $found = grep { $_ eq $bestmove } @bm;
		ok $found, "line $lineno";
	} else {
		my $found = grep { $_ eq $bestmove } @am;
		ok !$found, "line $lineno";
	}

	if ($limit_tests > 0 && $lineno >= $limit_tests) {
		last;
	}
}

$cin->print("quit\n");

done_testing;

sub convert_san {
	my ($san, $fen) = @_;

	my $pos = Chess::Rep->new;
	eval {
		$pos->set_from_fen($fen);
	};
	if ($@) {
		die "invalid FEN position\n";
	}

	my $movespec = eval { $pos->go_move($san) };
	if ($@) {
		die "Invalid or illegal move '$san'.\n";
	}

	my $move = "$movespec->{from}$movespec->{to}";
	$move .= $movespec->{promote} if defined $movespec->{promote};

	return lc $move;
}

sub parse_epd {
	my ($line, $epdfile, $lineno) = @_;

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
			die "$epdfile:$lineno: invalid EPD.\n";
	}

	my $position = $1;

	my %task;

	while (length $line) {
		my $operation;

		if ($line !~ s/^[ \t]*([_a-zA_Z0-9]+)//) {
			die "$epdfile:$lineno: invalid EPD.\n";
		}

		$operation = $1;
		die "$epdfile:$lineno: duplicate operation '$operation'.\n"
			if exists $task{$operation};
		
		my @operands;

		while (length $line) {
			if ($line =~ s/^[ \t]*"(.*?)"//) {
				push @operands, $1;
			} elsif ($line =~ s/^[ \t]*([^ \t;]+)//) {
				push @operands, $1;
			} elsif ($line =~ s/^[ \t]*;//) {
				last;
			} else {
				die "$epdfile:$lineno: invalid EPD\n";
			}
		}

		$task{$operation} = [@operands];
	}

	$task{hmvc} = [0] if !exists $task{hmvc};
	$task{fmvc} = [1] if !exists $task{fmvc};

	my $fen = "$position $task{hmvc}->[0] $task{fmvc}->[0]";

	return $fen, %task;
}

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
