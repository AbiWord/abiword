
noinst_LTLIBRARIES = libxp.la

libxp_la_CPPFLAGS = \
	$(OPENXML_CFLAGS) \
	-I$(srcdir)/../../imp/xp \
	-DABI_BUILD_VERSION=\"$(VERSION)\"

libxp_la_SOURCES = \
	ie_impexp_OpenXML.cpp \
	OXML_Document.cpp \
	OXML_Document.h \
	OXML_Element.cpp \
	OXML_Element.h \
	OXML_Element_Run.cpp \
	OXML_Element_Run.h \
	OXML_Element_Text.cpp \
	OXML_Element_Text.h \
	OXML_FontManager.cpp \
	OXML_LangToScriptConverter.h \
	OXML_FontManager.h \
	OXML_ObjectWithAttrProp.cpp \
	OXML_ObjectWithAttrProp.h \
	OXML_Section.cpp \
	OXML_Section.h \
	OXML_Style.cpp \
	OXML_Style.h \
	OXML_Theme.cpp \
	OXML_Theme.h \
	OXML_Types.h

OXML_LangToScriptConverter.h: $(srcdir)/OXML_LangToScriptConverter.gperf
	gperf $< >$@

EXTRA_DIST = \
	converter.pl \
	OXML_LangToScriptConverter.gperf

