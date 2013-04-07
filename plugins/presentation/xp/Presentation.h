/*
 * A plugin to Allow AbiWord to make presentations.
 * Copyright (C) 2007 by Martin Sevior
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

#ifndef PLUGIN_PRESENTATION_H
#define PLUGIN_PRESENTATION_H
#include "ut_string_class.h"
#include "ev_EditEventMapper.h"
#include "ev_EditBinding.h"
#include "xap_Frame.h"

class AV_View;
class FV_View;
class XAP_App;
class XAP_Frame;
class GR_Image;

class Presentation
{
public:
	                         Presentation(void);
	                        ~Presentation(void);
	bool                     showNext(void);
	bool                     showNextPage(void);
        bool                     start(AV_View * pView);
	bool                     gotoPage(UT_sint32 iPage);
	bool                     drawNthPage(UT_sint32 i);
	bool                     showPrev(void);
	bool                     showPrevPage(void);

	bool                     end(void);
	GR_Image *               renderPageToImage(UT_sint32 iPage,  UT_uint32 izoom);
private:
        bool                     _loadPresentationBindings(AV_View * view);
	XAP_App *                m_pApp;
	FV_View *                m_pView;
	UT_sint32                m_iPage;
	UT_String                m_sPrevBindings;
	UT_sint32                m_iOldZoom;
	XAP_Frame::tZoomType     m_OldZoomType;
	bool                     m_bDrewNext;
	bool                     m_bDrewPrev;
};

#endif /* PLUGIN_PRESENTATION_H */












