#!/usr/bin/perl
opendir(my $d, '/tmp');
my $replay = '/tmp/replay.wrm';
my $last = $replay;
my $lasttime = -M $last;
for my $x (readdir($d)){
    next if $x !~ /[.]wrm$/;
    my $candidate = "/tmp/$x";
    my $candidatetime = -M $candidate;
    next if $lasttime && $candidatetime >= $lasttime;
    $lasttime = $candidatetime;
    $last =  $candidate;
}

rename $last, $replay or die "Renaming of $last to $replay failed : $!\n";
