/* AbiSource Application Framework
 * Copyright (C) 1998-2000 AbiSource, Inc.
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
#include "ev_EditBits.h"
class XAP_App;

// TODO shouldn't these classes be xav_ prefixed ??

enum AV_Focus
{
	AV_FOCUS_HERE,
	AV_FOCUS_NEARBY,
	AV_FOCUS_NONE,
        AV_FOCUS_MODELESS
};

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
	AV_View(XAP_App * pApp, void*);
	virtual ~AV_View();

	virtual void focusChange(AV_Focus focus)=0;
	AV_Focus getFocus(){ return m_focus; }
	void setFocus(AV_Focus focus){ m_focus=focus; }

	void*			getParentData() const;

	void			setInsertMode(bool bInsert);
	void			setWindowSize(UT_sint32, UT_sint32);
	virtual void	setXScrollOffset(UT_sint32) = 0;
	virtual void	setYScrollOffset(UT_sint32) = 0;
	UT_uint32               getTick(void);
	void                    incTick(void);
	inline XAP_App *	getApp(void) const { return m_pApp; };
	inline UT_sint32	getWindowWidth(void) const { return m_iWindowWidth; };
	inline UT_sint32	getWindowHeight(void) const { return m_iWindowHeight; };
	inline UT_sint32	getXScrollOffset(void) const { return m_xScrollOffset; };
	inline UT_sint32	getYScrollOffset(void) const { return m_yScrollOffset; };

	virtual void	draw(const UT_Rect* pRect=(UT_Rect*) NULL) = 0;

	virtual void	cmdScroll(AV_ScrollCmd cmd, UT_uint32 iPos = 0) = 0;
	void			addScrollListener(AV_ScrollObj*);
	void			removeScrollListener(AV_ScrollObj*);
	void			sendVerticalScrollEvent(UT_sint32 yoff, UT_sint32 ylimit = -1);
	void			sendHorizontalScrollEvent(UT_sint32 xoff, UT_sint32 xlimit = -1);

	bool			addListener(AV_Listener * pListener, AV_ListenerId * pListenerId);
	bool			removeListener(AV_ListenerId listenerId);

	//! returns true iff the current view is the active/focused window
	bool			isActive(void);
	virtual bool	notifyListeners(const AV_ChangeMask hint) = 0;

	virtual bool	canDo(bool bUndo) const = 0;
	virtual void	cmdUndo(UT_uint32 count) = 0;
	virtual void	cmdRedo(UT_uint32 count) = 0;
	virtual UT_Error	cmdSave(void) = 0;
	virtual UT_Error	cmdSaveAs(const char * szFilename, int ieft) = 0;
	virtual UT_Error        cmdSaveAs(const char * szFilename, int ieft, bool cpy) = 0;

	virtual EV_EditMouseContext getMouseContext(UT_sint32 xPos, UT_sint32 yPos) = 0;
	virtual bool 	isSelectionEmpty(void) const = 0;
	virtual void		cmdUnselectSelection(void) = 0;

	virtual UT_uint32   calculateZoomPercentForPageWidth() = 0;
	virtual UT_uint32   calculateZoomPercentForPageHeight() = 0;
	virtual UT_uint32   calculateZoomPercentForWholePage() = 0;
	
protected:
	XAP_App *			m_pApp;
	void*				m_pParentData;

	UT_sint32			m_xScrollOffset;
	UT_sint32			m_yScrollOffset;
	UT_sint32			m_iWindowHeight;
	UT_sint32			m_iWindowWidth;
	AV_Focus			m_focus;
	UT_uint32                       m_iTick; // Count changes
	bool				m_bInsertMode;

	UT_Vector			m_scrollListeners;
	UT_Vector			m_vecListeners;

private:
	AV_View(const AV_View&);	// no impl.
	void operator=(AV_View&);	// no impl.
};

#endif /* AV_VIEW_H */
