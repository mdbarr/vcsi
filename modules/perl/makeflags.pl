#!/usr/bin/perl

use Config;

my $libs = $Config{libs};
my $path = $Config{archlibexp};

#print STDERR "Perl libs: $libs\n";
#print STDERR "Perl Path: $path\n";

if($ARGV[0] eq "-c") {
    print "-I" . $path . "/CORE" . " -L" . $path . "/CORE";
} elsif($ARGV[0] eq "-l") {
    print $libs;
}
