if TOOLKIT_COCOA

else
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = garble.la

garble_la_LIBADD = \
	xp/libxp.la

garble_la_LDFLAGS = \
	$(GARBLE_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

garble_la_SOURCES = 

if TOOLKIT_WIN

plugin_DATA = \
	garble.dll

CLEANFILES = \
	garble.def \
	garble.dll \
	garble.dll.a

.libs/garble.a: garble.la

garble.dll: .libs/garble.a
	$(CXX) -shared $^ -o $@ -Wl,--enable-auto-import -Wl,--out-implib,$@.a $(GARBLE_LIBS)

endif # TOOLKIT_WIN
endif
