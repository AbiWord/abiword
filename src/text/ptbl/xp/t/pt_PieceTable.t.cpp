/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 2011 Hub Figuiere
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */


#include "tf_test.h"
#include "pt_PieceTable.h"
#include "pd_Document.h"
#include "pf_Frag_Strux_Block.h"

#define TFSUITE "core.text.ptbl.piecetable"

// FIXME write real test
TFTEST_MAIN("pt_PieceTable")
{
//	PD_Document doc;
	pt_PieceTable pt(NULL);

	TFPASS(pt.getDocument() == NULL);

	// we need to set the state to loading.
	pt.setPieceTableState(PTS_Loading);

	const PP_PropertyVector attrs = {
		"foo", "bar"
	};

	pf_Frag_Strux *frag = NULL;
	TFPASS(pt.appendStrux(PTX_Block, attrs, &frag));
	TFPASS(frag);
	TFPASS(frag->getType() == pf_Frag::PFT_Strux);

	TFPASS(pt.appendFmtMark());
}
