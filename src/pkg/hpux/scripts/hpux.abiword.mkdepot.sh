#!/bin/sh

#
#  Check that we're on HP-UX:
#
if [ "`uname -s`" != "HP-UX" ] ; then
    echo "$0: Error: This is intended for HP-UX systems only"  >&2
    exit 1
fi

#
# Get command-line arguments:
#
if [ $# -ne 2 ] ; then
    echo "Usage: $0 revision depot_filename"  >&2
    echo '(You may also need to set $TARGET and $BINDIR.)'  >&2
    exit 1
fi
ABIWORDREVISION="$1"
ABIWORDDEPOT="$2"

#
#  Set $TARGET and $BINDIR, if they're not already set:
#
: ${TARGET:=/usr/local/AbiSuite}
: ${BINDIR:=/usr/local/bin}

#
#  Other variables:
#
TMPABIWORDPSF=`mktemp -c` || exit 1
HPUXCONFIGURE=hpux.abiword.configure

#
#  Check that the files to be packaged have been installed:
#
if [ ! -d "$TARGET" ] ; then
    echo "$0: Error: Cannot find files to package under $TARGET"  >&2
    exit 1
fi

#
#  Check that the configure script exists:
#
if [ ! -x "$HPUXCONFIGURE" ] ; then
    echo "$0: Error: Cannot find package configure script $HPUXCONFIGURE"  >&2
    exit 1
fi

#
#
#  Check os_release compatibility:
case "`uname -r`" in
    ?.10.2?)
        OS_RELEASE='?.10.2?|?.11.*'
        ;;
    ?.11.*)
        OS_RELEASE='?.11.*'
        ;;
    *)
        echo "$0: Error: this OS version is not recognized"  >&2
        exit 1
        ;;
esac

#
#  Create the PSF file (input to swpackage):
#
cat << EOF > $TMPABIWORDPSF
#
#  Example hpux.abiword.psf PSF file, used to create an HP-UX depot file.
#  Dynamically created by $0, written by Kevin Vajk.
#  For more information, look for the manual "Managing HP-UX Software
#  With SD-UX" at http://docs.hp.com.
#
#  You must first compile and "make install" abiword on your build system.
#  If the systems you will be distributing this package to might not have
#  all the shared libraries you have (i.e. libpng), then you may want to
#  build static (UNIX_CAN_BUILD_DYNAMIC=0) or simply delete the shared
#  binary before making the package.
#
#  Important fields in this PSF file:
#    - The revision
#    - The os_release must not be *earlier* than this build machine.
#      (e.g. if you build on 11.00, os_release must not allow 10.20
#      machines to install the depot, but if you built on 10.20, it's
#      fine for 11.00 boxes to use.)
#    - The pathnames (/usr/local/AbiSuite, /usr/local/bin)
#  It also uses a configure script (see below), which it expects to
#  find in the current directory.
#
#  Now, here is how to build a depot from this PSF file:
#    # swpackage -x create_target_acls=false -x target_type=tape
#                -d hpux.abiword.depot -s hpux.abiword.psf
#  This creates a file hpux.abiword.depot, which you may distribute.
#
#  To install from this depot file, copy it onto your system, start up
#  "swinstall", choose "Local Directory" for the Source Depot Type, then
#  enter the full path to abiword.depot for the Source Depot Path.  Mark
#  the AbiSuite product, and install it.  Good luck!
#
product
  tag           AbiWord
  revision      $ABIWORDREVISION
  title         AbiWord Word Processor
  description   http://abisource.com
  copyright     GNU General Public License
  # compared to "uname -r":
  os_release    $OS_RELEASE
  # compared to "uname -m":
  machine_type  9000/7*|9000/8*
  # compared to "uname -s":
  os_name       HP-UX
  fileset
    tag           AbiWord
    #  The name of the configure script:
    configure     $HPUXCONFIGURE
    file_permissions -o root -g root
    file /usr/local/bin/AbiWord
    file /usr/local/bin/abiword
    # Note: "file *" isn't really a glob pattern, despite appearances.
    directory /usr/local/AbiSuite
    file *
  end
end
EOF

#
#  Make the package:
#
PATH=/usr/sbin:/sbin:${PATH} ; export PATH
rm -f -- $ABIWORDDEPOT
swpackage -x create_target_acls=false -x target_type=tape  \
               -d $ABIWORDDEPOT -s $TMPABIWORDPSF
if [ $? -ne 0 ] ; then
    echo "$0: Error: swpackage failed"  >&2
    exit 1
fi

#
#  Clean up:
#
rm -f -- $TMPABIWORDPSF

#
#  We're done!
#
echo Created $ABIWORDDEPOT
exit 0

