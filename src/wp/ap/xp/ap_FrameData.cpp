/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
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
 

#include "ap_FrameData.h"
#include "gr_Graphics.h"
#include "fl_DocLayout.h"
#include "ap_TopRuler.h"
#include "ap_LeftRuler.h"
#include "ap_StatusBar.h"

/*****************************************************************/

AP_FrameData::AP_FrameData()
{
	m_pDocLayout = NULL;
	m_pG = NULL;
	m_pTopRuler = NULL;
	m_pLeftRuler = NULL;
	m_pStatusBar = NULL;
}

AP_FrameData::~AP_FrameData()
{
	DELETEP(m_pDocLayout);
	DELETEP(m_pG);
	DELETEP(m_pTopRuler);
	DELETEP(m_pLeftRuler);
	DELETEP(m_pStatusBar);
}
