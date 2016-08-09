
ACLOCAL_AMFLAGS = -I m4

DIST_SUBDIRS = src plugins po user icons tools

SUBDIRS = src plugins po user

if TOOLKIT_GTK_ALL

SUBDIRS += icons

desktopdir = $(datadir)/applications
desktop_DATA = abiword.desktop

mimedir = @ABIWORD_DATADIR@/mime-info
mime_DATA = abiword.keys

appdatadir = $(datarootdir)/appdata
appdata_DATA = abiword.appdata.xml

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = abiword-$(ABIWORD_SERIES).pc

dist_man1_MANS = abiword.1

endif

if TOOLKIT_GTK
# we disable introspection for distcheck as it fails because of
# installation needing sudo
DISTCHECK_CONFIGURE_FLAGS = \
	--enable-introspection=no \
	--with-gir-dir=\$${datadir}/gir-1.0 \
	--with-typelib-dir=\$${libdir}/girepository-1.0
endif

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
	$(appdata_DATA) \
	abiword.keys \
	abiword.png \
	libabiword.pc.in \
	compile-resource \
	lt-compile-resource

DISTCLEANFILES = \
	abiword-$(ABIWORD_SERIES).pc \
	plugin-builtin.m4 \
	plugin-configure.m4 \
	plugin-list.m4 \
	plugin-makefiles.m4

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

