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

CPPSRCS=		pd_Document.cpp			\
			pd_Document_TestRoutines.cpp	\
			pf_Frag.cpp			\
			pf_Frag_Object.cpp		\
			pf_Frag_Strux.cpp		\
			pf_Frag_Strux_Block.cpp		\
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
			pt_PieceTable_ChangeObject.cpp	\
			pt_PieceTable_ChangeSpan.cpp	\
			pt_PieceTable_ChangeStrux.cpp	\
			pt_PieceTable_Glob.cpp		\
			pt_PieceTable_DeleteObject.cpp	\
			pt_PieceTable_DeleteSpan.cpp	\
			pt_PieceTable_DeleteStrux.cpp	\
			pt_PieceTable_InsertObject.cpp	\
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
			px_ChangeRecord_Object.cpp		\
			px_ChangeRecord_ObjectChange.cpp	\
			px_ChangeRecord_Span.cpp	\
			px_ChangeRecord_SpanChange.cpp	\
			px_ChangeRecord_Strux.cpp	\
			px_ChangeRecord_StruxChange.cpp	\
			px_ChangeRecord_TempSpanFmt.cpp	\
			px_ChangeRecord_TestRoutines.cpp

LIBRARY_NAME=		AbiPTbl
LIBRARY_VERSION=	$(ABI_VERSION)
TARGETS=		$(LIBRARY)

include $(ABI_DEPTH)/config/abi_rules.mk

build:: $(TARGETS)
	@echo Exporting library $(LIBRARY_NAME) from `pwd`...
	@$(subst xxxx,$(DIST)/lib,$(VERIFY_DIRECTORY))
	@$(ABICOPY) $(TARGETS) $(DIST)/lib
