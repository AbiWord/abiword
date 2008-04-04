
ACLOCAL_AMFLAGS = -I .

DIST_SUBDIRS = goffice-bits src plugins user

if WITH_GOFFICE_BUILTIN
SUBDIRS = goffice-bits
else 
SUBDIRS =
endif
SUBDIRS += src plugins user

desktopdir = $(datadir)/applications
desktop_DATA = abiword.desktop

icondir= $(datadir)/icons
icon_DATA = abiword_48.png

mimedir = $(datadir)/mime-info
mime_DATA = abiword.keys

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = abiword-$(ABIWORD_SERIES).pc

EXTRA_DIST = \
	AUTHORS \
	COPYING	\
	INSTALL \
	NEWS \
	README \
	ChangeLog \
	abiword.desktop \
	abiword.keys \
	abiword_48.png \
	libabiword.pc.in \
	plugin-configure.m4 \
	plugin-list.m4 \
	plugin-makefiles.m4

DISTCLEANFILES = \
	abiword-$(ABIWORD_SERIES).pc

abiword-$(ABIWORD_SERIES).pc: libabiword.pc
	cp $< $@

if TOOLKIT_WIN
zip:
	rm -rf $(PACKAGE)-$(VERSION)
	make DESTDIR=$(shell pwd)/zip install
	mv zip$(prefix) $(PACKAGE)-$(VERSION)
	cd $(PACKAGE)-$(VERSION)
	zip -r $(PACKAGE)-$(VERSION).zip $(PACKAGE)-$(VERSION) -x \*.a \*.la
	rm -rf $(PACKAGE)-$(VERSION) $(shell pwd)/zip
endif

