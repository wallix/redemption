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


my $v5 = <<HERE
// pad4octetsB (4 bytes): A 32-bit, unsigned integer. Padding. Values in this field MUST be ignored.

// desktopSaveSize (4 bytes): A 32-bit, unsigned integer. The maximum usable size of bitmap space for bitmap packing in the SaveBitmap Primary Drawing Order (see [MS-RDPEGDI] section 2.2.2.2.1.1.2.12). This field is ignored by the client and assumed to be 230400 bytes (480 * 480).

// pad2octetsC (2 bytes): A 16-bit, unsigned integer. Padding. Values in this field MUST be ignored.

// pad2octetsD (2 bytes): A 16-bit, unsigned integer. Padding. Values in this field MUST be ignored.

// textANSICodePage (2 bytes): A 16-bit, unsigned integer. ANSI code page descriptor being used by the client (for a list of code pages, see [MSDN-CP]). This field is ignored by the client and SHOULD be set to 0 by the server.

// pad2octetsE (2 bytes): A 16-bit, unsigned integer. Padding. Values in this field MUST be ignored.
HERE
;

print c80($v5);
