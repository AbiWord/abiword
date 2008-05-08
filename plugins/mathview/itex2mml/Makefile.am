
noinst_LTLIBRARIES = libitex2mml.la

AM_CPPFLAGS =

libitex2mml_la_SOURCES = \
	itex2MML.h \
	lex.yy.c \
	y.tab.c \
	y.tab.h

YYPREFIX=itex2MML_yy

BUILT_SOURCES = \
	lex.yy.c \
	y.tab.c

EXTRA_DIST = \
	itex2MML.l \
	itex2MML.y \
	README.itex2MML \
	test-itex.html

DISTCLEANFILES = \
	$(BUILT_SOURCES) \
	y.tab.h

y.tab.c:	$(srcdir)/itex2MML.y
		$(YACC) -p $(YYPREFIX) -d $<

lex.yy.c:	$(srcdir)/itex2MML.l
		$(LEX) -P$(YYPREFIX) -o$@ $<

