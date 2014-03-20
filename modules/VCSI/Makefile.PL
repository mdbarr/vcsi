use ExtUtils::MakeMaker;
# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.
WriteMakefile(
    'NAME'	=> 'VCSI',
    'VERSION_FROM' => 'VCSI.pm', # finds $VERSION
    'LIBS'	=> ['-lm -ldl -lpthread'],   # e.g., '-lm' 
    'DEFINE'	=> '',     # e.g., '-DHAVE_SOMETHING' 
    'INC'	=> '',     # e.g., '-I/usr/include/other' 
    'MYEXTLIB'  => '../../src/libvcsi.a',
);
