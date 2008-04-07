
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = latex.la

latex_la_LIBADD = \
	xp/libxp.la

latex_la_LDFLAGS = \
	$(PLUGIN_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

latex_la_SOURCES = 

if TOOLKIT_WIN

plugin_DATA = \
	latex.dll

CLEANFILES = \
	latex.def \
	latex.dll \
	latex.dll.a

latex.dll: .libs/liblatex.a
	$(CXX) -shared $^ -o $@ -Wl,--out-implib,$@.a $(PLUGIN_LIBS)

endif # TOOLKIT_WIN

