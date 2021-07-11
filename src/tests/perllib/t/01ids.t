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

use strict;

use Test::More;

use EPDParser;

my $epdfile = __FILE__;
$epdfile =~ s/\.t/.epd/;
open my $fh, '<', $epdfile
	or die "error opening '$epdfile': $!\n";
my @epds = <$fh>;

my $expect_file = __FILE__;
$expect_file =~ s/\.t/.expect/;
open my $fh, '<', $expect_file
	or die "error opening '$expect_file': $!\n";
my @expect = map { chomp $_; $_ } <$fh>;

for (my $i = 0; $i < @epds; ++$i) {
	my $epd = EPDParser->new($epds[$i]);
	my $id = $epd->operation('id');
	is $id, $expect[$i];
}

ok 1;

done_testing;
