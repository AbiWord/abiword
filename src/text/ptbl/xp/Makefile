#! gmake
 
##
## The contents of this file are subject to the AbiSource Public
## License Version 1.0 (the "License"); you may not use this file
## except in compliance with the License. You may obtain a copy
## of the License at http://www.abisource.com/LICENSE/ 
## 
## Software distributed under the License is distributed on an
## "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
## implied. See the License for the specific language governing
## rights and limitations under the License. 
## 
## The Original Code is AbiWord.
## 
## The Initial Developer of the Original Code is AbiSource, Inc.
## Portions created by AbiSource, Inc. are Copyright (C) 1998 AbiSource, Inc. 
## All Rights Reserved. 
## 
## Contributor(s):
##  

ABI_DEPTH = ../../..

include $(ABI_DEPTH)/config/abi_defs.mk

EXPORT_HEADERS=		pd_Document.h			\
			pl_Listener.h			\
			pp_AttrProp.h			\
			pp_Property.h			\
			pt_Types.h			\
			px_ChangeRecord.h		\
			px_ChangeRecord_Fmt.h		\
			px_ChangeRecord_Span.h		\
			px_ChangeRecord_SpanChange.h	\
			px_ChangeRecord_Strux.h		\
			px_ChangeRecord_StruxChange.h

HEADERS=		$(EXPORT_HEADERS)		\
			pf_Frag.h			\
			pf_Frag_Object.h		\
			pf_Frag_Strux.h			\
			pf_Frag_Strux_Block.h		\
			pf_Frag_Strux_Column.h		\
			pf_Frag_Strux_ColumnSet.h	\
			pf_Frag_Strux_Section.h		\
			pf_Frag_Text.h			\
			pf_Fragments.h			\
			pp_TableAttrProp.h		\
			pt_PieceTable.h			\
			pt_VarSet.h			\
			px_ChangeHistory.h

RELEASE_HEADERS=	$(EXPORT_HEADERS)
RELEASE_HEADERS_DEST=	$(RELEASE_INCLUDE_DIR)

CPPSRCS=		pd_Document.cpp			\
			pf_Frag.cpp			\
			pf_Frag_Strux.cpp		\
			pf_Frag_Strux_Block.cpp		\
			pf_Frag_Strux_Column.cpp	\
			pf_Frag_Strux_ColumnSet.cpp	\
			pf_Frag_Strux_Section.cpp	\
			pf_Frag_Text.cpp		\
			pf_Fragments.cpp		\
			pp_AttrProp.cpp			\
			pp_Property.cpp			\
			pp_TableAttrProp.cpp		\
			pt_PieceTable.cpp		\
			pt_PieceTable_Append.cpp	\
			pt_PieceTable_ChangeSpan.cpp	\
			pt_PieceTable_ChangeStrux.cpp	\
			pt_PieceTable_Glob.cpp		\
			pt_PieceTable_DeleteSpan.cpp	\
			pt_PieceTable_DeleteStrux.cpp	\
			pt_PieceTable_InsertSpan.cpp	\
			pt_PieceTable_InsertStrux.cpp	\
			pt_PieceTable_Undo.cpp		\
			pt_VarSet.cpp			\
			px_ChangeHistory.cpp		\
			px_ChangeRecord.cpp		\
			px_ChangeRecord_Span.cpp	\
			px_ChangeRecord_SpanChange.cpp	\
			px_ChangeRecord_Strux.cpp	\
			px_ChangeRecord_StruxChange.cpp

INCLUDES=		-I$(DIST)/include -I. -DUT_DEBUG

LIBRARY_NAME=		AbiDocStrux
LIBRARY_VERSION=	$(ABI_VERSION)
TARGETS=		$(LIBRARY)

include $(ABI_DEPTH)/config/abi_rules.mk

export:: $(EXPORT_HEADERS)
	@echo Exporting headers from `pwd`...
	@$(subst xxxx,$(DIST)/include,$(VERIFY_DIRECTORY))
	@$(INSTALL) -m 444 $(EXPORT_HEADERS) $(DIST)/include

libs:: $(TARGETS)
	@echo Exporting library $(LIBRARY_NAME) from `pwd`...
	@$(subst xxxx,$(DIST)/lib,$(VERIFY_DIRECTORY))
	@$(INSTALL) -m 444 $(TARGETS) $(DIST)/lib

install:: libs
