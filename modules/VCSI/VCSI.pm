package VCSI;

use strict;
use vars qw($VERSION @ISA @EXPORT @EXPORT_OK);

require Exporter;
require DynaLoader;
require AutoLoader;

@ISA = qw(Exporter DynaLoader);
# Items to export into callers namespace by default. Note: do not export
# names by default without a very good reason. Use EXPORT_OK instead.
# Do not simply export all your public functions/methods/constants.
@EXPORT = qw(
	
);
$VERSION = '0.02';

bootstrap VCSI $VERSION;

# Preloaded methods go here.

# Autoload methods go after =cut, and are processed by the autosplit program.

1;
__END__
# Below is the stub of documentation for your module. You better edit it!

=head1 NAME

VCSI - Perl extension VCSI - The Vaguely Coherent Scheme Interpreter

=head1 SYNOPSIS

  use VCSI;
  
  my $context = VCSI::vcsi_init(0);

  my $result = VCSI::rep($context, "(+ 1 1)");

=head1 DESCRIPTION

A Perl interface for VCSI. 
Allows inline evaluation of Scheme.

=head1 Exported functions

  VCSI_CONTEXT vcsi_init(unsigned long heap_size);
  char* rep(VCSI_CONTEXT vc, char* input);

=head1 AUTHOR

Mark Barr

=head1 SEE ALSO

perl(1).

=cut
