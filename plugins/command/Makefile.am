if TOOLKIT_COCOA

else
SUBDIRS = xp

plugindir = $(ABIWORD_PLUGINSDIR)
plugin_LTLIBRARIES = command.la

command_la_LIBADD = \
	xp/libxp.la

command_la_LDFLAGS = \
	$(COMMAND_LIBS) \
	-avoid-version \
	-module \
	-no-undefined

command_la_SOURCES = 

endif
