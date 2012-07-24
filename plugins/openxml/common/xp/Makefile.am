
noinst_LTLIBRARIES = libxp.la

AM_CPPFLAGS = \
	$(OPENXML_CFLAGS) \
	-I$(srcdir)/../../imp/xp -I$(srcdir)/../../exp/xp \
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
	OXML_Element_Paragraph.cpp \
	OXML_Element_Paragraph.h \
	OXML_Element_Table.cpp \
	OXML_Element_Table.h \
	OXML_Element_Row.cpp \
	OXML_Element_Row.h \
	OXML_Element_Cell.cpp \
	OXML_Element_Cell.h \
	OXML_Element_List.cpp \
	OXML_Element_List.h \
	OXML_Element_Math.cpp \
	OXML_Element_Math.h \
	OXML_Element_Image.cpp \
	OXML_Element_Image.h \
	OXML_Element_Hyperlink.cpp \
	OXML_Element_Hyperlink.h \
	OXML_Element_Bookmark.cpp \
	OXML_Element_Bookmark.h \
	OXML_Element_Field.cpp \
	OXML_Element_Field.h \
	OXML_Element_TextBox.cpp \
	OXML_Element_TextBox.h \
	OXML_FontManager.cpp \
	OXML_LangToScriptConverter.h \
	OXML_FontManager.h \
	OXML_ObjectWithAttrProp.cpp \
	OXML_ObjectWithAttrProp.h \
	OXML_Section.cpp \
	OXML_Section.h \
	OXML_Style.cpp \
	OXML_Style.h \
	OXML_List.h \
	OXML_List.cpp \
	OXML_Image.h \
	OXML_Image.cpp \
	OXML_Theme.cpp \
	OXML_Theme.h \
	OXML_Types.h

OXML_LangToScriptConverter.h: $(srcdir)/OXML_LangToScriptConverter.gperf
	gperf $< >$@

EXTRA_DIST = \
	converter.pl \
	OXML_LangToScriptConverter.gperf

