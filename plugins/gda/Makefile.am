
SUBDIRS = unix

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = libgda.la

libgda_la_LIBADD = \
	unix/libunix.la

libgda_la_LDFLAGS = \
	$(GDA_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

libgda_la_SOURCES = 

EXTRA_DIST = \
	plugin.m4

if TOOLKIT_WIN

plugin_DATA = \
	libgda.dll

CLEANFILES = \
	libgda.def \
	libgda.dll \
	libgda.dll.a

libgda.dll: .libs/libgda.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(GDA_LIBS)

endif # TOOLKIT_WIN

