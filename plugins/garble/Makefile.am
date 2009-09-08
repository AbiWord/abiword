if TOOLKIT_COCOA

else
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = garble.la

garble_la_LIBADD = \
	xp/libxp.la \
	$(GARBLE_LIBS) -lpng12 -ljpeg

garble_la_LDFLAGS = \
	$(GARBLE_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

garble_la_SOURCES = 
nodist_EXTRA_garble_la_SOURCES = dummy.cpp

if TOOLKIT_WIN

plugin_DATA = \
	garble.dll

CLEANFILES = \
	garble.def \
	garble.dll \
	garble.dll.a

.libs/garble.a: garble.la

garble.dll: .libs/garble.a
	$(CXX) -shared $^ -o $@ -Wl,--enable-auto-import -Wl,--out-implib,$@.a $(GARBLE_LIBS) -lpng12 -ljpeg

endif # TOOLKIT_WIN
endif
