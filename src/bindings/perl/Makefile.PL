use ExtUtils::MakeMaker;
# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.

#abi-js/src/bindings/perl
$abidir = "../..";

WriteMakefile(
    'NAME'		=> 'abi',
    'XSOPT'		=> '-C++',
	'CC' => 'g++',
	'LD' => 'g++',
	#SKIP        => [qw(dynamic dynamic_lib dynamic_bs)],
	#LINKTYPE    => 'static',
    'VERSION_FROM'	=> 'abi.pm', # finds $VERSION
    'PREREQ_PM'		=> {}, # e.g., Module::Name => 1.1
    'LIBS'		=> [''], # e.g., '-lm'
    'DEFINE'		=> '', # e.g., '-DHAVE_SOMETHING'
    'INC'		=> join(' ', map {"-I$abidir/$_"} 
		qw(af/ev/xp af/xap/xp af/util/xp af/util/unix ../../expat/lib 
		text/fmt/xp text/ptbl/xp af/gr/xp wp/impexp/xp wp/ap/xp)),
);
