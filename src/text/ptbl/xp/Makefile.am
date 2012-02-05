
DIST_SUBDIRS = t

noinst_LTLIBRARIES = libxp.la

# WP includes needed because of prefs
AM_CPPFLAGS = \
	$(WP_CPPFLAGS) $(CHAMPLAIN_CFLAGS)

if HAVE_REDLAND
REDLANDSOURCES = pd_RDFSupportRed.h
endif

libxp_la_SOURCES = \
	fd_Field.cpp \
	fd_Field.h \
	pd_Document.cpp \
	pd_Document.h \
	pd_Document_TestRoutines.cpp \
	pd_DocumentRDF.h \
	pd_DocumentRDF.cpp \
	pd_RDFQuery.h \
	pd_RDFQuery.cpp \
	$(REDLANDSOURCES) \
	pd_RDFSupport.h \
	pd_RDFSupport.cpp \
	pd_Iterator.cpp \
	pd_Iterator.h \
	pd_Style.cpp \
	pd_Style.h \
	pf_Frag.cpp \
	pf_Frag_FmtMark.cpp \
	pf_Frag_FmtMark.h \
	pf_Frag.h \
	pf_Fragments.cpp \
	pf_Fragments.h \
	pf_Frag_Object.cpp \
	pf_Frag_Object.h \
	pf_Frags_TestRoutines.cpp \
	pf_Frag_Strux_Block.cpp \
	pf_Frag_Strux_Block.h \
	pf_Frag_Strux.cpp \
	pf_Frag_Strux.h \
	pf_Frag_Strux_Section.cpp \
	pf_Frag_Strux_Section.h \
	pf_Frag_TestRoutines.cpp \
	pf_Frag_Text.cpp \
	pf_Frag_Text.h \
	pl_Listener.h \
	pl_ListenerCoupleCloser.h \
	pl_ListenerCoupleCloser.cpp \
	po_Bookmark.cpp \
	po_Bookmark.h \
	pp_AttrProp.cpp \
	pp_AttrProp.h \
	pp_Property.cpp \
	pp_Property.h \
	pp_PropertyMap.cpp \
	pp_PropertyMap.h \
	pp_Revision.cpp \
	pp_Revision.h \
	pp_TableAttrProp.cpp \
	pp_TableAttrProp.h \
	pt_PieceTable.cpp \
	pt_PieceTable.h \
	pt_PT_Append.cpp \
	pt_PT_ChangeObject.cpp \
	pt_PT_ChangeSpan.cpp \
	pt_PT_ChangeStrux.cpp \
	pt_PT_DeleteObject.cpp \
	pt_PT_DeleteSpan.cpp \
	pt_PT_DeleteStrux.cpp \
	pt_PT_FmtMark.cpp \
	pt_PT_Glob.cpp \
	pt_PT_InsertObject.cpp \
	pt_PT_InsertSpan.cpp \
	pt_PT_InsertStrux.cpp \
	pt_PT_Listener.cpp \
	pt_PT_Styles.cpp \
	pt_PT_TestRoutines.cpp \
	pt_PT_Undo.cpp \
	pt_Types.h \
	pt_VarSet.cpp \
	pt_VarSet.h \
	px_ChangeHistory.cpp \
	px_ChangeHistory.h \
	px_ChangeRecord.cpp \
	px_ChangeRecord.h \
	px_CH_TestRoutines.cpp \
	px_CR_DataItem.cpp \
	px_CR_DataItem.h \
	px_CR_DocProp.cpp \
	px_CR_DocProp.h \
	px_CR_FmtMarkChange.cpp \
	px_CR_FmtMarkChange.h \
	px_CR_FmtMark.cpp \
	px_CR_FmtMark.h \
	px_CR_Glob.cpp \
	px_CR_Glob.h \
	px_CR_ObjectChange.cpp \
	px_CR_ObjectChange.h \
	px_CR_Object.cpp \
	px_CR_Object.h \
	px_CR_SpanChange.cpp \
	px_CR_SpanChange.h \
	px_CR_Span.cpp \
	px_CR_Span.h \
	px_CR_StruxChange.cpp \
	px_CR_StruxChange.h \
	px_CR_Strux.cpp \
	px_CR_Strux.h \
	px_CR_Style.cpp \
	px_CR_Style.h \
	px_CR_TestRoutines.cpp \
	pp_Author.h\
	pp_Author.cpp

