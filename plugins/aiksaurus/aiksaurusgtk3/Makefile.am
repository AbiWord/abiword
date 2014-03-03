AM_CXXFLAGS = $(AIKSAURUS_CFLAGS) -Wall -I../.. -I$(top_srcdir)/base

lib_LTLIBRARIES = libAiksaurusGtk3.la

libAiksaurusGtk3_la_LDFLAGS = -no-undefined -release $(LT_RELEASE) -export-dynamic

libAiksaurusGtk3_la_LIBADD = $(AIKSAURUS_LIBS)

libAiksaurusGtk3_la_SOURCES = \
	AiksaurusGTK.cpp \
	AiksaurusGTK_histlist.cpp \
	AiksaurusGTK_history.cpp \
	AiksaurusGTK_picbutton.cpp \
	AiksaurusGTK_strlist.cpp \
	AiksaurusGTK_utils.cpp \
	Display.cpp \
	Exception.cpp \
	Meaning.cpp \
	Replacebar.cpp \
	Toolbar.cpp \
	AiksaurusGTK-C.cpp

noinst_HEADERS = \
	AiksaurusGTK.h \
	AiksaurusGTK-C.h

OTHERHEADERS = \
	AiksaurusGTK_histlist.h \
	AiksaurusGTK_history.h \
	AiksaurusGTK_picbutton.h \
	AiksaurusGTK_strlist.h \
	AiksaurusGTK_utils.h \
	DialogMediator.h \
	Display.h \
	Exception.h \
	Meaning.h \
	Replacebar.h \
	Toolbar.h

EXTRA_DIST = $(OTHERHEADERS)
