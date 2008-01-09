
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libcommand.la

libcommand_la_LIBADD = \
	xp/libxp.la

libcommand_la_LDFLAGS = \
	$(COMMAND_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libcommand_la_SOURCES = 

if TOOLKIT_WIN

plugin_DATA = \
	libcommand.dll

CLEANFILES = \
	libcommand.def \
	libcommand.dll \
	libcommand.dll.a

libcommand.dll: .libs/libcommand.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(COMMAND_LIBS)

endif # TOOLKIT_WIN

