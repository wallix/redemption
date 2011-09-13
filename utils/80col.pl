#!perl

sub c80
{
    my $line = substr($_[0], 3);
    my @words = split /\s/, $line;
    my @res = ();
    my $n = @words[0];
    shift @words;
    for my $w (@words){
        if (length($n)+length($w) > 77){
            push @res, $n;
            $n = $w;
            next;
        }
        $n.=' '.$w;
    }
    push @res, $n;
    my $res = "";
    for my $l (@res){
        my $r = sprintf "// %s\n", $l;
#        print $r;
        $res.= $r;
    }
    return $res;
}

1;
