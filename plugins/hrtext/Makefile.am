
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = hrtext.la

hrtext_la_LIBADD = \
	xp/libxp.la

hrtext_la_LDFLAGS = \
	$(PLUGIN_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

hrtext_la_SOURCES = 

if TOOLKIT_WIN

plugin_DATA = \
	hrtext.dll

CLEANFILES = \
	hrtext.def \
	hrtext.dll \
	hrtext.dll.a

hrtext.dll: .libs/hrtext.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(PLUGIN_LIBS)

endif # TOOLKIT_WIN

