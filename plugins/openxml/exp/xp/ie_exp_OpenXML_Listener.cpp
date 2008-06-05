/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource
 * 
 * Copyright (C) 2008 Firat Kiyak <firatkiyak@gmail.com>
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

// Class definition include
#include <ie_exp_OpenXML_Listener.h>

/**
 * IE_Exp_OpenXML_Listener Class responsible for listening to the Abiword Document
 */

bool IE_Exp_OpenXML_Listener::populate(PL_StruxFmtHandle /* sfh */, const PX_ChangeRecord* /* pcr */)
{
	return true;
}

bool IE_Exp_OpenXML_Listener::populateStrux(PL_StruxDocHandle /* sdh */, const PX_ChangeRecord* /* pcr */, PL_StruxFmtHandle* /* psfh */)
{
	return true;
}

bool IE_Exp_OpenXML_Listener::change(PL_StruxFmtHandle /* sfh */, const PX_ChangeRecord* /* pcr */)
{
	return true;
}

bool IE_Exp_OpenXML_Listener::insertStrux(PL_StruxFmtHandle /* sfh */, const PX_ChangeRecord* /* pcr */, PL_StruxDocHandle /* sdhNew */, PL_ListenerId /* lid */,
				 						  void (* /* pfnBindHandles */)(PL_StruxDocHandle /* sdhNew */, PL_ListenerId /* lid */, PL_StruxFmtHandle /* sfhNew */))
{
	return true;
}

bool IE_Exp_OpenXML_Listener::signal(UT_uint32 /* iSignal */)
{
	return true;
}

