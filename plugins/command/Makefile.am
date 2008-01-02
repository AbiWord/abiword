
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

