#!perl

sub c80
{
    my @lines = split /\n/, $_[0];

    my $res = "";
    my $line = "";
    for my $l (@lines){
        if ($l !~ /[\/][\/] /){
            if (not $line eq ""){
                my @words = split /\s/, $line;
                my $n = @words[0];
                shift @words;
                for my $w (@words){
                    if (length($n)+length($w) > 77){
                        $res .= sprintf "// %s\n", $n;
                        $n = $w;
                        next;
                    }
                    $n.=' '.$w;
                }
                $res .= sprintf "// %s\n", $n if not $n eq "";
            }
            $res.=$l."\n";
            $line = "";
        }
        else {
            if ($line eq ""){
                $line = substr($l, 3);
            }
            else {
                $line .= substr($l, 2);
            }
        }    
    }
    if (not $line eq ""){
        my @words = split /\s/, $line;
        my $n = @words[0];
        shift @words;
        for my $w (@words){
            if (length($n)+length($w) > 77){
                $res .= sprintf "// %s\n", $n;
                $n = $w;
                next;
            }
            $n.=' '.$w;
        }
        $res .= sprintf "// %s\n", $n if not $n eq "";
    }
    return $res;
}

1;
