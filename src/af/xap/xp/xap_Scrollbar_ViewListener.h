/* AbiSource Program Utilities
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

#pragma once

#include "xav_Listener.h"

class AV_View;

class XAP_Frame;

class ABI_EXPORT ap_Scrollbar_ViewListener : public AV_Listener
{
public:
	ap_Scrollbar_ViewListener(XAP_Frame * pFrame, AV_View * pView);

	virtual bool		notify(AV_View * pView, const AV_ChangeMask mask) override;
	virtual AV_ListenerType getType(void) const override { return AV_LISTENER_SCROLLBAR;}

protected:
	XAP_Frame *			m_pFrame;
	AV_View *			m_pView;
};
