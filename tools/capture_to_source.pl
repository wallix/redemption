#!perl

sub dump_to_data
{
    my $filename = shift;
    my $start = shift;
    my $stop = shift;

    open(my $f, $filename);
    my $x = 0;
    for (<$f>){
        next unless /\s\-\-\s(.*)$/;
        my $line = $1;
        $x = 1 if $line =~ /$start/;
        $x = 0 if $line =~ /$stop/;
        print '// ' if !($x && ($line =~ /^ \/\* .... \*\/.*$/));
        print $line." |";
        print "\n";
    }
}


print "const char outdata[] =\n{\n";
dump_to_data  $ARGV[0], '^.*Sending on .* bytes.*$', '^.*Sent dumped on' ;
print "} /* end outdata */;\n\n";

print "const char indata[] =\n{\n";
dump_to_data  $ARGV[0], '^.*Recv done on.* bytes.*$', '^.*Dump done' ;
print "} /* end indata */;\n\n";

1;
