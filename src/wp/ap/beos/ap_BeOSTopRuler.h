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

#ifndef AP_BEOSTOPRULER_H
#define AP_BEOSTOPRULER_H

// Class for dealing with the horizontal ruler at the top of
// a document window.

/*****************************************************************/

#include "ut_types.h"
#include "ap_TopRuler.h"
#include "gr_BeOSGraphics.h"

#include "be_GRDrawView.h"

class XAP_Frame;


/*****************************************************************/

class AP_BeOSTopRuler : public AP_TopRuler
{
public:
	AP_BeOSTopRuler(XAP_Frame * pFrame);
	virtual ~AP_BeOSTopRuler(void);

	void 		createWidget(BRect r);
	virtual void	setView(AV_View * pView);
	
protected:
	be_GRDrawView *		m_wTopRuler;

protected:

};

#endif /* AP_BEOSTOPRULER_H */
