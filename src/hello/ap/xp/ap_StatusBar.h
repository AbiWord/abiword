/* AbiHello
 * Copyright (C) 1999 AbiSource, Inc.
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

#ifndef AP_STATUSBAR_H
#define AP_STATUSBAR_H

// Class for dealing with the status bar at the bottom of
// the frame.

#include "ut_types.h"
#include "ut_misc.h"
#include "ut_units.h"
#include "ut_vector.h"
#include "ap_Prefs.h"
#include "ap_Prefs_SchemeIds.h"
#include "xav_Listener.h"

class XAP_Frame;
class GR_Graphics;

#define AP_MAX_MESSAGE_FIELD			256

/*****************************************************************/
/*****************************************************************/

class AP_StatusBar : public AV_Listener
{
public:
	AP_StatusBar(XAP_Frame * pFrame);
	virtual ~AP_StatusBar(void);

	XAP_Frame *			getFrame(void) const;
	GR_Graphics *		getGraphics(void) const;
	virtual void		setView(AV_View * pView);
	void				draw(void);
	UT_uint32			getWidth(void) const;
	UT_uint32			getHeight(void) const;
	void				setWidth(UT_uint32 iWidth);
	void				setHeight(UT_uint32 iHeight);
	void				setStatusMessage(UT_UCSChar * pbufUCS);
	void				setStatusMessage(const char * pbuf);
	const UT_UCSChar *	getStatusMessage(void) const;
	
	/* used with AV_Listener */
	virtual UT_Bool		notify(AV_View * pView, const AV_ChangeMask mask);

protected:
	void				_draw(void);

	XAP_Frame *			m_pFrame;
	AV_View *			m_pView;
	GR_Graphics *		m_pG;
	UT_Dimension		m_dim;
	UT_uint32			m_iHeight;
	UT_uint32			m_iWidth;

	UT_uint32			s_iFixedHeight;

	UT_Bool				m_bInitFields;
	UT_Vector			m_vecFields;			/* vector of 'ap_sb_Field *' */
	void *				m_pStatusMessageField;	/* actually 'ap_sb_Field_StatusMessage *' */

	UT_UCSChar			m_bufUCS[AP_MAX_MESSAGE_FIELD];
};

#endif /* AP_STATUSBAR_H */
