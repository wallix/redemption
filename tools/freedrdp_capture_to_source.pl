#!perl

sub dump_to_data
{
    my $filename = shift;
    my $start = shift;
    my $stop = shift;

    open(my $f, $filename);
    my $x = 0;
    for (<$f>){
        chomp;
        my $line = $_;
        $x = 1 if $line =~ /$start/;
        $x = 0 if $line =~ /$stop/;
        print '// ' if !($x && ($line =~ /^\/\* .... \*\/.*$/));
        print "$line\n";
    }
}


print "const char outdata[] =\n{\n";
dump_to_data  $ARGV[0], '^Local [>] Remote.*bytes.*$', '^transport::transport' ;
print "} /* end outdata */;\n\n";

print "const char indata[] =\n{\n";
dump_to_data  $ARGV[0], '^Local [<] Remote.*bytes.*$', '^.*::' ;
print "} /* end indata */;\n\n";

1;
