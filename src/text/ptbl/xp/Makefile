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
			pd_Style.cpp		\
			pf_Frag.cpp			\
			pf_Frag_Object.cpp		\
			pf_Frag_Strux.cpp		\
			pf_Frag_Strux_Block.cpp		\
			pf_Frag_Strux_Section.cpp	\
			pf_Frag_TestRoutines.cpp	\
			pf_Frag_Text.cpp		\
			pf_Fragments.cpp		\
			pf_Frags_TestRoutines.cpp	\
			pp_AttrProp.cpp			\
			pp_Property.cpp			\
			pp_TableAttrProp.cpp		\
			pt_PieceTable.cpp		\
			pt_PT_Append.cpp	\
			pt_PT_ChangeObject.cpp	\
			pt_PT_ChangeSpan.cpp	\
			pt_PT_ChangeStrux.cpp	\
			pt_PT_Glob.cpp		\
			pt_PT_DeleteObject.cpp	\
			pt_PT_DeleteSpan.cpp	\
			pt_PT_DeleteStrux.cpp	\
			pt_PT_InsertObject.cpp	\
			pt_PT_InsertSpan.cpp	\
			pt_PT_InsertStrux.cpp	\
			pt_PT_Listener.cpp	\
			pt_PT_TempFmt.cpp	\
			pt_PT_TestRoutines.cpp	\
			pt_PT_Undo.cpp		\
			pt_VarSet.cpp			\
			px_ChangeHistory.cpp		\
			px_ChangeRecord.cpp		\
			px_CR_Glob.cpp	\
			px_CR_Object.cpp		\
			px_CR_ObjectChange.cpp	\
			px_CR_Span.cpp	\
			px_CR_SpanChange.cpp	\
			px_CR_Strux.cpp	\
			px_CR_StruxChange.cpp	\
			px_CR_TempSpanFmt.cpp	\
			px_CR_TestRoutines.cpp

LIBRARY_NAME=		AbiPTbl
LIBRARY_VERSION=	$(ABI_VERSION)
TARGETS=		$(LIBRARY)

include $(ABI_DEPTH)/config/abi_rules.mk

build:: $(TARGETS)
	@echo Exporting library $(LIBRARY_NAME) from `pwd`...
	@$(subst xxxx,$(DIST)/lib,$(VERIFY_DIRECTORY))
	@$(ABICOPY) $(TARGETS) $(DIST)/lib
