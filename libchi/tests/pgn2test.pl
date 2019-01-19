#! /usr/bin/env perl

# This file is part of the chess engine tate.
#
# Copyright (C) 2002-2019 cantanea EOOD.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

# Parse on or more PGN games and convert them into unit tests.

use common::sense;

use Chess::PGN::Parse;

sub parse_pgn($);

my @filenames = @ARGV or die "usage: $0 PGNFILE...\n";

foreach my $filename (@filenames) {
	parse_pgn $filename;
}

sub parse_pgn($) {
	my ($filename) = @_;

	my $pgn = Chess::PGN::Parse->new($filename)
		or die "cannot parse '$filename': $!\n";
	
	while ($pgn->read_game) {
		my $event = $pgn->event;
		my $site = $pgn->site;
		my $date = $pgn->date;
		my $round = $pgn->round;
		my $white = $pgn->white;
		my $black = $pgn->black;
		my $eco = $pgn->eco;
		my $result = $pgn->result;
		$pgn->parse_game() or die "cannot smart parse $filename";
		my $moves = "{\n";
		my $line = "";
		my $line_length = 0;
		foreach my $move (@{$pgn->moves}) {
			my $move_length = 2 + length $move;
			if ($line_length == 0) {
				$line = qq{\t\t\t"$move"};
				$line_length = 24 + $move_length;
			} elsif ($line_length + $move_length + 2 > 78) {
				$moves .= $line . ",\n";
				$line_length = 24 + $move_length;
				$line = qq{\t\t\t"$move"};
			} else {
				$line_length += 2 + $move_length;
				$line .= qq{, "$move"};
			}
		}
		$moves .= "$line\n";
		$moves .= "\t\t},\n";
		print <<EOF;
	{
		__FILE__, __LINE__,
		"$event",
		"$site",
		"$date",
		"$round",
		"$white",
		"$black",
		"$eco",
		"$result",
		$moves
	},
EOF
	}
}
