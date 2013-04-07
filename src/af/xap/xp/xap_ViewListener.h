/* AbiSource Application Framework
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */



#ifndef XAP_VIEWLISTENER_H
#define XAP_VIEWLISTENER_H

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif

#include "xav_Listener.h"

class AV_View;

class XAP_Frame;

/*
	The ap_ViewListener class handles UI change notifications from an AV_View
	to its associated XAP_Frame.
*/

// TODO shouldn't this class be xap_ ??

class ABI_EXPORT ap_ViewListener : public AV_Listener
{
public:
	ap_ViewListener(XAP_Frame* pFrame);
	virtual ~ap_ViewListener();

	virtual bool		notify(AV_View * pView, const AV_ChangeMask mask);
  	virtual  AV_ListenerType getType(void) { return AV_LISTENER_VIEW;}

protected:
	XAP_Frame*		m_pFrame;
};

#endif /* XAP_VIEWLISTENER_H */
