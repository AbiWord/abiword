
noinst_LTLIBRARIES = libgtk.la

AM_CPPFLAGS = \
	$(GRAMMAR_CFLAGS) \
	-I$(srcdir)/.. \
	-I$(srcdir)/../xp 

libgtk_la_SOURCES = \
	UnixDialogGrammar.cpp \
	UnixDialogGrammar.h

uidir=$(ABIWORD_UIDIR)
ui_DATA = \
	UnixDialogGrammar.xml

EXTRA_DIST = \
	$(ui_DATA)
