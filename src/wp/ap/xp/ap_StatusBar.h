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

#ifndef AP_STATUSBAR_H
#define AP_STATUSBAR_H

#include <limits.h>
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

#define AP_MAX_MESSAGE_FIELD			(256*MB_LEN_MAX)

/*****************************************************************/
/*****************************************************************/

#define PROGRESS_CMD_MASK 0x3		/* 0,1,2,3 Operational values */
enum _progress_flags {
	PROGRESS_RESERVED1 	= 0x0,	
	PROGRESS_START  	= 0x1,		/* Start using the progress bar */
	PROGRESS_STOP	 	= 0x2,		/* Stop using the progress bar */	
	PROGRESS_RESERVED2	= 0x3,
    PROGRESS_SHOW_MSG	= 0x4,		/* Allow message to be displayed */
	PROGRESS_SHOW_RAW	= 0x8,		/* Allow raw value to be displayed */
	PROGRESS_SHOW_PERCENT = 0x10	/* Allow calculation of percent value */
};

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
	void				setStatusMessage(UT_UCSChar * pbufUCS, int redraw = true);
	void				setStatusMessage(const char * pbuf, int redraw = true);
	const UT_UCSChar *	getStatusMessage(void) const;

	void				setStatusProgressType(int start, int end, int flags);
	void 				setStatusProgressValue(int value);

	virtual void		show(void) {} // It must be abstract, but I don't want to screw
	virtual void		hide(void) {} // the platforms that don't implement show/hide
	
	/* used with AV_Listener */
	virtual bool		notify(AV_View * pView, const AV_ChangeMask mask);

protected:
	void				_draw(void);

	XAP_Frame *			m_pFrame;
	AV_View *			m_pView;
	GR_Graphics *		m_pG;
	UT_Dimension		m_dim;
	UT_uint32			m_iHeight;
	UT_uint32			m_iWidth;

	UT_uint32			s_iFixedHeight;

	bool				m_bInitFields;
	UT_Vector			m_vecFields;			/* vector of 'ap_sb_Field *' */
	void *				m_pStatusMessageField;	/* actually 'ap_sb_Field_StatusMessage *' */

	UT_UCSChar			m_bufUCS[AP_MAX_MESSAGE_FIELD];
};

#endif /* AP_STATUSBAR_H */
