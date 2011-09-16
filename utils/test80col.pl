#!perl
use Test::More tests => 5;
 
unshift @INC, '.';
require_ok '80col.pl';

my $v1 = <<HERE
// I have a bunch of consecutive lines with C like comments.
HERE
;
my $r1 = c80($v1);
is $r1, $v1, "One line, nothing to merge or split";

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

my $r2 = c80($v2);
is $r2, $e2, "One long line to split";

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

my $r3 = c80($v3);
is $r3, $e3, "Several short lines concatenated";


my $v4 = <<HERE
// These lines
// should be concatenated
These should be ignored because not starting with comment mark 
// And these should
// also be concatenated

// and these
HERE
;

my $e4 = <<HERE
// These lines should be concatenated
These should be ignored because not starting with comment mark 
// And these should also be concatenated

// and these
HERE
;

my $r4 = c80($v4);
is $r4, $e4, "Lines not begining as comment are ignored";


