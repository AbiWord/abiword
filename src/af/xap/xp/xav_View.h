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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */


#ifndef AV_VIEW_H
#define AV_VIEW_H

#include "ut_misc.h"
#include "ut_types.h"
#include "ut_vector.h"
#include "xav_Listener.h"
class AP_TopRulerInfo;

typedef enum _AV_ScrollCmd
{
	AV_SCROLLCMD_PAGEUP,
	AV_SCROLLCMD_PAGEDOWN,
	AV_SCROLLCMD_LINEUP,
	AV_SCROLLCMD_LINEDOWN,
	AV_SCROLLCMD_PAGERIGHT,
	AV_SCROLLCMD_PAGELEFT,
	AV_SCROLLCMD_LINERIGHT,
	AV_SCROLLCMD_LINELEFT,
	AV_SCROLLCMD_TOTOP,
	AV_SCROLLCMD_TOBOTTOM,
	AV_SCROLLCMD_TOPOSITION
} AV_ScrollCmd;

class AV_ScrollObj
{
 public:
	AV_ScrollObj(void * pData,
				 void (*pfnX)(void *,UT_sint32,UT_sint32),
				 void (*pfnY)(void *,UT_sint32,UT_sint32))
		{ m_pData=pData; m_pfnX=pfnX; m_pfnY=pfnY; };
	
	void* m_pData;
	void (*m_pfnX)(void *, UT_sint32 xoff, UT_sint32 xlimit);
	void (*m_pfnY)(void *, UT_sint32 yoff, UT_sint32 ylimit);
};

class AV_View
{
public:
	AV_View(void*);
	virtual ~AV_View();

	void*			getParentData() const;

	void			setWindowSize(UT_sint32, UT_sint32);
	virtual void	setXScrollOffset(UT_sint32) = 0;
	virtual void	setYScrollOffset(UT_sint32) = 0;

	inline UT_sint32	getWindowWidth(void) const { return m_iWindowWidth; };
	inline UT_sint32	getWindowHeight(void) const { return m_iWindowHeight; };
	inline UT_sint32	getXScrollOffset(void) const { return m_xScrollOffset; };
	inline UT_sint32	getYScrollOffset(void) const { return m_yScrollOffset; };

	virtual UT_uint32	getPageViewLeftMargin(void) const = 0;
	virtual UT_uint32	getPageViewTopMargin(void) const = 0;
	
	virtual void	draw(const UT_Rect* pRect=(UT_Rect*) NULL) = 0;

	virtual void	cmdScroll(AV_ScrollCmd cmd, UT_uint32 iPos = 0) = 0;
	void			addScrollListener(AV_ScrollObj*);
	void			removeScrollListener(AV_ScrollObj*);
	void			sendVerticalScrollEvent(UT_sint32 yoff, UT_sint32 ylimit = -1);
	void			sendHorizontalScrollEvent(UT_sint32 xoff, UT_sint32 xlimit = -1);

	UT_Bool			addListener(AV_Listener * pListener, AV_ListenerId * pListenerId);
	UT_Bool			removeListener(AV_ListenerId listenerId);
	virtual UT_Bool	notifyListeners(const AV_ChangeMask hint) = 0;

	virtual UT_Bool	canDo(UT_Bool bUndo) const = 0;
	virtual void	cmdUndo(UT_uint32 count) = 0;
	virtual void	cmdRedo(UT_uint32 count) = 0;
	virtual UT_Bool	cmdSave(void) = 0;
	virtual UT_Bool	cmdSaveAs(const char * szFilename) = 0;

	virtual void	getTopRulerInfo(AP_TopRulerInfo * pInfo) = 0;
	
protected: 
	void*				m_pParentData;

	UT_sint32			m_xScrollOffset;
	UT_sint32			m_yScrollOffset;
	UT_sint32			m_iWindowHeight;
	UT_sint32			m_iWindowWidth;

	UT_Vector			m_scrollListeners;
	UT_Vector			m_vecListeners;
};

#endif /* AV_VIEW_H */
