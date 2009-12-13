
ACLOCAL_AMFLAGS = -I .

DIST_SUBDIRS = goffice-bits src plugins user

if WITH_GOFFICE_BUILTIN
SUBDIRS = goffice-bits
else 
SUBDIRS =
endif
SUBDIRS += src plugins user

if TOOLKIT_GTK

if !EMBEDDED_HILDON
desktopdir = $(datadir)/applications
desktop_DATA = abiword.desktop
endif

mimedir = @ABIWORD_DATADIR@/mime-info
mime_DATA = abiword.keys

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = abiword-$(ABIWORD_SERIES).pc

dist_man1_MANS = abiword.1
endif

icondir= $(datadir)/icons
icon_DATA = abiword_48.png

EXTRA_DIST = \
	AUTHORS \
	BiDiReadme.txt \
	BUILD.TXT \
	COPYING	\
	COPYRIGHT.TXT \
	INSTALL \
	NEWS \
	README \
	ChangeLog \
	abiword.desktop \
	abiword.keys \
	abiword_48.png \
	libabiword.pc.in \
	compile-resource \
	lt-compile-resource

DISTCLEANFILES = \
	abiword-$(ABIWORD_SERIES).pc \
	plugin-builtin.m4 \
	plugin-configure.m4 \
	plugin-list.m4 \
	plugin-makefiles.m4 \
	doltcompile \
	doltlibtool

abiword-$(ABIWORD_SERIES).pc: libabiword.pc
	cp $< $@

if TOOLKIT_WIN
zip:
	rm -rf $(PACKAGE)-$(VERSION)
	make DESTDIR=$(abs_builddir)/zip install
	mv zip$(prefix) $(PACKAGE)-$(VERSION)
	cd $(PACKAGE)-$(VERSION)
	zip -r $(PACKAGE)-$(VERSION).zip $(PACKAGE)-$(VERSION) -x \*.a \*.la
	rm -rf $(PACKAGE)-$(VERSION) ./zip
endif

# needed to make sure we don't include .svn directories.
dist-hook:
	rm -rf `find $(distdir) -name .svn`

