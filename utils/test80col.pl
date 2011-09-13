#!perl
use Test::More tests => 4;
 
unshift @INC, '.';
require_ok '80col.pl';

my $v1 = <<HERE
// I have a bunch of consecutive lines with C like comments.
HERE
;

ok c80($v1) eq $v1, "One line, nothing to merge or split";

my $v2 = <<HERE
// This line is longer than 80 columns, henceforth it should be splitted between several lines. But I want to avoid splitting words. And every line should begin by a comment mark, like below.
HERE
;

my $e2 = <<HERE
// This line is longer than 80 columns, henceforth it should be splitted between
// several lines. But I want to avoid splitting words. And every line should
// begin by a comment mark, like below.
HERE
;

ok c80($v2) eq $e2, "One long line to split";

my $v3 = <<HERE
// This line is short,
// and this one also, so
// I should concatenate them all.
HERE
;

my $e3 = <<HERE
// This line is short, and this one also, so I should concatenate them all.
HERE
;

ok c80($v3) eq $e3, "Several short lines concatenated";

