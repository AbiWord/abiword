#! gmake

## AbiWord
## Copyright (C) 1998 AbiSource, Inc.
##
## This program is free software; you can redistribute it and/or
## modify it under the terms of the GNU General Public License
## as published by the Free Software Foundation; either version 2
## of the License, or (at your option) any later version.
## 
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
## 
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
## 02111-1307, USA.

ABI_DEPTH = ../../..

include $(ABI_DEPTH)/config/abi_defs.mk

EXPORT_HEADERS=		pd_Document.h			\
			pl_Listener.h			\
			pp_AttrProp.h			\
			pp_Property.h			\
			pt_Types.h			\
			px_ChangeRecord.h		\
			px_ChangeRecord_Glob.h		\
			px_ChangeRecord_Span.h		\
			px_ChangeRecord_SpanChange.h	\
			px_ChangeRecord_Strux.h		\
			px_ChangeRecord_StruxChange.h	\
			px_ChangeRecord_TempSpanFmt.h

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
			pd_Document_TestRoutines.cpp	\
			pf_Frag.cpp			\
			pf_Frag_Strux.cpp		\
			pf_Frag_Strux_Block.cpp		\
			pf_Frag_Strux_Column.cpp	\
			pf_Frag_Strux_ColumnSet.cpp	\
			pf_Frag_Strux_Section.cpp	\
			pf_Frag_TestRoutines.cpp	\
			pf_Frag_Text.cpp		\
			pf_Fragments.cpp		\
			pf_Fragments_TestRoutines.cpp	\
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
			pt_PieceTable_Listener.cpp	\
			pt_PieceTable_TempFmt.cpp	\
			pt_PieceTable_TestRoutines.cpp	\
			pt_PieceTable_Undo.cpp		\
			pt_VarSet.cpp			\
			px_ChangeHistory.cpp		\
			px_ChangeRecord.cpp		\
			px_ChangeRecord_Glob.cpp	\
			px_ChangeRecord_Span.cpp	\
			px_ChangeRecord_SpanChange.cpp	\
			px_ChangeRecord_Strux.cpp	\
			px_ChangeRecord_StruxChange.cpp	\
			px_ChangeRecord_TempSpanFmt.cpp	\
			px_ChangeRecord_TestRoutines.cpp


INCLUDES=		-I$(DIST)/include -I. -DUT_DEBUG -DPT_TEST

LIBRARY_NAME=		AbiPTbl
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
