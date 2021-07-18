#! /usr/bin/env perl

use strict;

use IPC::Open2;
use Time::HiRes qw(gettimeofday tv_interval);

eval {
	require Chess::Rep;
};
if ($@) {
	warn "Please install Chess::Rep from CPAN if you want to run these tests.\n";
	exit 77;
}

use constant DEBUG => 0;

sub parse_epd;
sub convert_san;
sub bratko_kopec; 

my %protocols = (
	uci => {
		init => \&uci_init_engine,
		set_position => \&uci_set_position,
		go => \&uci_go,
		on_output => \&uci_on_output,
	},
	xboard => {
		init => \&xboard_init_engine,
		set_position => \&xboard_set_position,
		go => \&xboard_go,
		on_output => \&xboard_on_output,
	}
);

my $limit_tests = 100;

my ($engine, $proto, @commands) = @ARGV;

if (!defined $proto || !length $proto) {
	die "Usage: $0 ENGINE (uci|xboard) [COMMANDS...]"
}

my $protocol = $protocols{lc $proto}
	or die "$0: Unknown protocol '$proto' (neither 'uci' nor 'xboard').\n";

unshift @commands;

open STDERR, ">&STDOUT";

my ($cout, $cin);
my $pid = open2 $cout, $cin, $engine
	or die "cannot exec '$engine': $!";

$protocol->{init}->($cin, $cout, @commands);

bratko_kopec();

$cin->print("quit\n");

sub convert_san {
	my ($san, $fen) = @_;

	$san =~ s/[!?]+$//;

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

sub bratko_kopec {
	my $data = <<'EOF';
1k1r4/pp1b1R2/3q2pp/4p3/2B5/4Q3/PPP2B2/2K5 b - - bm Qd1+; id "BK.01";
3r1k2/4npp1/1ppr3p/p6P/P2PPPP1/1NR5/5K2/2R5 w - - bm d5; id "BK.02";
2q1rr1k/3bbnnp/p2p1pp1/2pPp3/PpP1P1P1/1P2BNNP/2BQ1PRK/7R b - - bm f5; id "BK.03";
rnbqkb1r/p3pppp/1p6/2ppP3/3N4/2P5/PPP1QPPP/R1B1KB1R w KQkq - bm e6; id "BK.04";
r1b2rk1/2q1b1pp/p2ppn2/1p6/3QP3/1BN1B3/PPP3PP/R4RK1 w - - bm Nd5 a4; id "BK.05";
2r3k1/pppR1pp1/4p3/4P1P1/5P2/1P4K1/P1P5/8 w - - bm g6; id "BK.06";
1nk1r1r1/pp2n1pp/4p3/q2pPp1N/b1pP1P2/B1P2R2/2P1B1PP/R2Q2K1 w - - bm Nf6; id "BK.07";
4b3/p3kp2/6p1/3pP2p/2pP1P2/4K1P1/P3N2P/8 w - - bm f5; id "BK.08";
2kr1bnr/pbpq4/2n1pp2/3p3p/3P1P1B/2N2N1Q/PPP3PP/2KR1B1R w - - bm f5; id "BK.09";
3rr1k1/pp3pp1/1qn2np1/8/3p4/PP1R1P2/2P1NQPP/R1B3K1 b - - bm Ne5; id "BK.10";
2r1nrk1/p2q1ppp/bp1p4/n1pPp3/P1P1P3/2PBB1N1/4QPPP/R4RK1 w - - bm f4; id "BK.11";
r3r1k1/ppqb1ppp/8/4p1NQ/8/2P5/PP3PPP/R3R1K1 b - - bm Bf5; id "BK.12";
r2q1rk1/4bppp/p2p4/2pP4/3pP3/3Q4/PP1B1PPP/R3R1K1 w - - bm b4; id "BK.13";
rnb2r1k/pp2p2p/2pp2p1/q2P1p2/8/1Pb2NP1/PB2PPBP/R2Q1RK1 w - - bm Qd2 Qe1; id "BK.14";
2r3k1/1p2q1pp/2b1pr2/p1pp4/6Q1/1P1PP1R1/P1PN2PP/5RK1 w - - bm Qxg7+; id "BK.15";
r1bqkb1r/4npp1/p1p4p/1p1pP1B1/8/1B6/PPPN1PPP/R2Q1RK1 w kq - bm Ne4; id "BK.16";
r2q1rk1/1ppnbppp/p2p1nb1/3Pp3/2P1P1P1/2N2N1P/PPB1QP2/R1B2RK1 b - - bm h5; id "BK.17";
r1bq1rk1/pp2ppbp/2np2p1/2n5/P3PP2/N1P2N2/1PB3PP/R1B1QRK1 b - - bm Nb3; id "BK.18";
3rr3/2pq2pk/p2p1pnp/8/2QBPP2/1P6/P5PP/4RRK1 b - - bm Rxe4; id "BK.19";
r4k2/pb2bp1r/1p1qp2p/3pNp2/3P1P2/2N3P1/PPP1Q2P/2KRR3 w - - bm g4; id "BK.20";
3rn2k/ppb2rpp/2ppqp2/5N2/2P1P3/1P5Q/PB3PPP/3RR1K1 w - - bm Nh6; id "BK.21";
2r2rk1/1bqnbpp1/1p1ppn1p/pP6/N1P1P3/P2B1N1P/1B2QPP1/R2R2K1 b - - bm Bxe4; id "BK.22";
r1bqk2r/pp2bppp/2p5/3pP3/P2Q1P2/2N1B3/1PP3PP/R4RK1 b kq - bm f6; id "BK.23";
r2qnrnk/p2b2b1/1p1p2pp/2pPpp2/1PP1P3/PRNBB3/3QNPPP/5RK1 w - - bm f4; id "BK.24";
EOF

	chomp $data;
	my @lines = split /\n/, $data;
	
	my $lineno = 0;
	my @results;
	print <<EOF;
Kratko-Kopec Test
=================

24 tasks are done.

1 point is awarded if the correct move is found within 120 seconds. If it was
the suggested move after 90 seconds but the suggestion later changed, 1/2
point is awarded.  Accordingly 1/3 point for a correct suggestion after 60
seconds, and 1/4 point for a correct suggestion after 30 seconds.

The estimated rating is given as follows:

| Score | Rating    |
+-------+-----------+
|   0-4 | 1300-1599 |
|   5-6 | 1600-1799 |
|   7-8 | 1800-1999 |
|  9-12 | 2000-2199 |
| 13-16 | 2200-2399 |
| 17-24 | 2400+     |

See https://www.researchgate.net/publication/237550539_NOTE_THE_BRATKO-KOPEC_TEST_RECALIBRATED

Results:

| Test | Type | Best |  Eng30 |  Eng60 |  Eng90 | Eng120 | Points |
+------+------+------+--------+--------+--------+--------+--------+
EOF

	my $total = 0;

	foreach my $line (@lines) {
		++$lineno;
		my ($fen, %task) = parse_epd $line, 'Bratko-Kopec', $lineno;

		# Convert to coordinate notation.
		my $bm = convert_san $task{bm}->[0], $fen;
		
		$protocol->{set_position}->($cin, $cout, $fen);
		$protocol->{go}->($cin, $cout, 120);

		my @start = gettimeofday;
		my @moves;
		while (1) {
			my $line = $cout->getline;
			print STDERR "<<< $line" if DEBUG;
			my $elapsed = tv_interval [@start];
			my @now = gettimeofday;
			my ($move, $done) = $protocol->{on_output}->($line, $fen);
			next if !$move;
			if ($elapsed <= 30) {
				$moves[0] = $move;
			} elsif ($elapsed <= 60) {
				$moves[1] = $move;
			} elsif ($elapsed <= 90) {
				$moves[1] = $move;
			} elsif ($elapsed <= 120) {
				$moves[1] = $move;
			} elsif ($elapsed > 120.1) {
				warn "Engine took $elapsed (>120) seconds on KP test #$lineno.\n";
			}
			last if $done;
		}

		my $points = 0;
		for (my $i = 1; $i < 4; ++$i) {
			if (!$moves[$i] && $moves[$i - 1]) {
				$moves[$i] = $moves[$i - 1];
			}
		}

		if ($moves[3] && $moves[3] eq $bm) {
			$points = 1;
		} elsif ($moves[2] && $moves[2] eq $bm) {
			$points = 1 / 2;
		} elsif ($moves[1] && $moves[1] eq $bm) {
			$points = 1 / 3;
		} elsif ($moves[0] && $moves[0] eq $bm) {
			$points = 1 / 4;
		}

		my $type = '?';
		printf "| %4d |   %s | %5s | %6s | %6s | %6s | %6s |  %.3f |\n",
			$lineno, $type, $bm, @moves, $points;
		
		$total += $points;
	}

	print <<'EOF';
+------+-----+-------+--------+--------+--------+--------+--------+

EOF

	printf "Total score: %.3f\n", $total;
}

sub uci_init_engine {
	my ($cin, $cout, @commands) = @_;

	foreach my $cmd ('uci', @commands) {
		$cin->print("$cmd\n");
		print STDERR ">>> $cmd\n" if DEBUG;
	}
}

sub xboard_init_engine {
	my ($cin, $count, @commands) = @_;

	foreach my $cmd ('xboard', 'protover 2', 'post', 'easy', @commands) {
		$cin->print("$cmd\n");
		print STDERR ">>> $cmd\n" if DEBUG;
	}
}

sub uci_set_position {
	my ($cin, $cout, $fen) = @_;

	$cin->print("position fen $fen\n");
	print STDERR ">>> position fen $fen\n" if DEBUG;
}

sub xboard_set_position {
	my ($cin, $cout, $fen) = @_;

	$cin->print("setboard $fen\n");
	print STDERR ">>> setboard $fen\n";
}

sub uci_go {
	my ($cin, $cout, $seconds) = @_;

	die if !$seconds;

	my $mseconds = 1000 * $seconds;
	$cin->print("ucinewgame\n");
	print STDERR ">>> ucinewgame\n" if DEBUG;
	$cin->print("go movetime $mseconds\n");
	print STDERR ">>> go movetime $mseconds\n" if DEBUG;
}

sub xboard_go {
	my ($cin, $cout, $seconds) = @_;

	die if !$seconds;

	my $cseconds = 100 * $seconds;
	$cin->print("time $cseconds\n");
	print STDERR ">>> time $cseconds\n";
	$cin->print("go\n");
	print STDERR ">>> go\n";
}

sub uci_on_output {
	my ($line) = @_;

	chomp $line;

	my @tokens = split /[ \t]+/, $line;
	if ('bestmove' eq $tokens[0]) {
		return $tokens[1], 1;
	} elsif ('info' eq $tokens[0]) {
		shift @tokens;
		while (@tokens) {
			my $token = shift @tokens;
			if ('pv' eq $token) {
				my $move = shift @tokens;
				return $move;
			}
		}
	}

	return;
}

sub xboard_on_output {
	my ($line, $fen) = @_;

	chomp $line;

	my $on_move = $fen =~ / b / ? 'b' : 'w';
	my @tokens = split /[ \t]+/, $line;
	if ('move' eq $tokens[0]) {
		if ($tokens[1] =~ /^[NBRQK]([a-h][0-8]).([a-h][0-8])/) {
			return "$1$2", 1;
		}
		return convert_san($tokens[1], $fen), 1;
	} elsif ('My' eq $tokens[0] && 'move' eq $tokens[1] && 'is' eq $tokens[2]
	         && ':' eq $tokens[3]) {
		return $tokens[4], 1;
	} elsif ('w' eq $on_move && $line =~ /.* 1\. ([^ ]+)/) {
		return convert_san $1, $fen;
	} elsif ('b' eq $on_move && $line =~ /.* 1\. \.\.\. ([^ ]+)/) {
		return convert_san $1, $fen;
	} elsif ('b' eq $on_move && $line =~ /^(?: [-+]?[0-9]+){4} +([^ ]+)/) {
		return convert_san $1, $fen;
	}

	return;
}
