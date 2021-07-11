# This file is part of the chess engine tate.
#
# Copyright (C) 2002-2021 cantanea EOOD.
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
#

package EPDParser;

use strict;

our $VERSION = 1;

sub new {
	my ($class, $line) = @_;

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
			die "invalid EPD\n";
	}

	my %self = (
		position => $1,
	);

	while ($line =~ s/^[ \t]*([-_a-zA-Z0-9]+)//) {
		my $tag = $1;
		my @operands;
		
		while (length $line) {
			if ($line =~ s/^[ \t]*;[ \t]*//) {
				last;
			} elsif ($line =~ s/^[ \t]*"(.*?)"[ \t]*//) {
				push @operands, $1;
			} elsif ($line =~ s/^[ \t]*([^ \t;]+)[ \t]*//) {
				push @operands, $1;
			} else {
				die "unparsable EPD: $line\n";
			}
		}

		if (exists $self{$tag}) {
			die "duplicate tag '$tag'\n";
		}

		$self{$tag} = \@operands;
	}

	$line =~ s/[ \t]+//;
	if (length $line) {
		die "epd line has trailing garbage: $line";
	}

	$self{hmvc} ||= [0];
	$self{fmvc} ||= [1];

	bless \%self, $class;
}

sub operation {
	my ($self, $tag) = @_;

	return if !exists $self->{$tag};
	return if !@{$self->{$tag}};

	return wantarray ? @{$self->{$tag}} : $self->{$tag}->[0];
}

1;