#! /usr/bin/env perl

use strict;

use Chess::Rep;

my ($fen) = join ' ', @ARGV;

die "Usage: $0 FEN" unless defined $fen && length $fen;

print <<"EOF";
START_TEST(test_ FILL IN TEST NAME!)
{
	const char *fen = 
		"$fen";
	const char *wanted[] = {
EOF

my $pos = Chess::Rep->new;
$pos->set_from_fen($fen);

my @moves;
foreach my $move (@{$pos->status->{moves}}) {
	my $from = Chess::Rep::get_field_id($move->{from});
	my $to = Chess::Rep::get_field_id($move->{to});

	my $uci_move = lc "$from$to";

	if ((($from =~ /^.7$/ && $to =~ /^.8/) || ($from =~ /^.2$/ && $to =~ /^.1/))
	    && $move->{piece} & 0x1) {
		push @moves, "${uci_move}q", "${uci_move}r", "${uci_move}b", "${uci_move}n";
	} else {
		push @moves, $uci_move;
	}
};

my $count = 0;

my $moves = join ', ', map { qq{"$_"} } @moves;

my $count = 0;
$moves =~ s/, /++$count % 8 == 0 ? ",\n\t\t" : ', '/ge;

print <<"EOF";
		$moves
	};

	TEST_MOVE_GEN(fen, wanted)
}
EOF
