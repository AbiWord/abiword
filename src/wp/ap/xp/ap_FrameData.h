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


#ifndef AP_FRAMEDATA_H
#define AP_FRAMEDATA_H

class DG_Graphics;
class FL_DocLayout;
class AP_TopRuler;
class AP_LeftRuler;

// A trivial helper class to hold app-specific frame data

class AP_FrameData
{
public:
	AP_FrameData();
	~AP_FrameData(void);

	FL_DocLayout *		m_pDocLayout;
	DG_Graphics *		m_pG;
	AP_TopRuler *		m_pTopRuler;
	AP_LeftRuler *		m_pLeftRuler;
};

#endif /* AP_FRAMEDATA_H */
