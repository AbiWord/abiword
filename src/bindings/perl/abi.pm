package Abi;

use strict;

require Exporter;
require DynaLoader;

@abi::ISA = qw(Exporter DynaLoader);

# Items to export into callers namespace by default. Note: do not export
# names by default without a very good reason. Use EXPORT_OK instead.
# Do not simply export all your public functions/methods/constants.

# This allows declaration	use abi ':all';
# If you do not need this, moving things directly into @EXPORT or @EXPORT_OK
# will save memory.
%abi::EXPORT_TAGS = ( 'all' => [ qw(
	
) ] );

@abi::EXPORT_OK = ( @{ $abi::EXPORT_TAGS{'all'} } );

@abi::EXPORT = qw(
	
);
$abi::VERSION = '0.01';

bootstrap abi $abi::VERSION;

# Preloaded methods go here.

1;
__END__
# Below is stub documentation for your module. You better edit it!

=head1 NAME

abi - Perl extension for AbiWord

=head1 SYNOPSIS

  use abi;
  $frame = abi::openFile("blah.abw");
  $view = abi::getCurrentViewFromFrame($frame);
  abi::insertData($view, "Hello world!");

=head1 DESCRIPTION

Stub documentation for abi, created by h2xs. It looks like the
author of the extension was negligent enough to leave the stub
unedited.

Blah blah blah.

=head2 EXPORT

None by default.


=head1 AUTHOR

J. Cuenca Abela, cuenca@celium.net

=head1 SEE ALSO

perl(1).

=cut
