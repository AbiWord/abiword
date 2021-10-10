/* AbiSource Application Framework
 * Copyright (C) 1998-2000 AbiSource, Inc.
 * Copyright (C) 2021 Hubert Figuière
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

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#include "ut_types.h"
#include "ut_misc.h"
#include "ut_vector.h"
#include "ut_debugmsg.h"

#include "ev_EditBits.h"

#include "xap_Drawable.h"
#include "xav_Listener.h"

class GR_Graphics;

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

class ABI_EXPORT AV_ScrollObj
{
 public:
	AV_ScrollObj(void * pData,
				 void (*pfnX)(void *,UT_sint32,UT_sint32),
				 void (*pfnY)(void *,UT_sint32,UT_sint32))
			: m_pData(pData), m_pfnX(pfnX), m_pfnY(pfnY)
		{	};

	void* m_pData;
	void (*m_pfnX)(void *, UT_sint32 xoff, UT_sint32 xlimit);
	void (*m_pfnY)(void *, UT_sint32 yoff, UT_sint32 ylimit);
};

class ABI_EXPORT AV_View
	: public XAP_Drawable
{
public:
	AV_View(XAP_App * pApp, void*);
	virtual ~AV_View();

	virtual void focusChange(AV_Focus focus)=0;
	AV_Focus getFocus(){ return m_focus; }
	void setFocus(AV_Focus focus){ m_focus=focus; }

	void*			getParentData() const;

	void			setInsertMode(bool bInsert);

	virtual void    setPoint(UT_uint32 pt) = 0;

	/*! the parameters are in device units! */
	void			setWindowSize(UT_sint32, UT_sint32);
	virtual void	setXScrollOffset(UT_sint32) = 0;
	virtual void	setYScrollOffset(UT_sint32) = 0;
	UT_uint32               getTick(void) const;
	void                    incTick(void);
	inline XAP_App *	getApp(void) const { return m_pApp; };
	virtual void    setCursorToContext(void) =0;

	/*! the return values of these functions are in logical units !!!*/
	UT_sint32		getWindowWidth(void) const;
	UT_sint32		getWindowHeight(void) const;
	inline UT_sint32	getXScrollOffset(void) const { return m_xScrollOffset; }
	inline UT_sint32	getYScrollOffset(void) const { return m_yScrollOffset; }

	virtual void	      updateScreen(bool bDirtyRunsOnly) = 0;
    virtual void          updateLayout(void) = 0;
	virtual void          rebuildLayout(void) = 0;
	virtual void          remeasureCharsWithoutRebuild() = 0;
	virtual void          fontMetricsChange() = 0;

	virtual void	cmdScroll(AV_ScrollCmd cmd, UT_uint32 iPos = 0) = 0;
	void			addScrollListener(AV_ScrollObj*);
	void			removeScrollListener(AV_ScrollObj*);
	/*! input parameters of these functions are in logical units !!! */
	void			sendVerticalScrollEvent(UT_sint32 yoff, UT_sint32 ylimit = -1);
	void			sendHorizontalScrollEvent(UT_sint32 xoff, UT_sint32 xlimit = -1);

	bool                    couldBeActive(void) const
	{  return m_bCouldBeActive;}
	bool			addListener(AV_Listener * pListener, AV_ListenerId * pListenerId);
	bool			removeListener(AV_ListenerId listenerId);

	//! returns true iff the current view is the active/focused window
	virtual bool		isActive(void) const = 0;
        void                    setActivityMask(bool bActive);
	virtual bool	notifyListeners(const AV_ChangeMask hint, void * pPrivateData = NULL);
	virtual bool    isDocumentPresent(void) const = 0;
	virtual bool	canDo(bool bUndo) const = 0;
	virtual void	cmdUndo(UT_uint32 count) = 0;
	virtual void	cmdRedo(UT_uint32 count) = 0;
	virtual UT_Error	cmdSave(void) = 0;
	virtual UT_Error	cmdSaveAs(const char * szFilename, int ieft) = 0;
	virtual UT_Error        cmdSaveAs(const char * szFilename, int ieft, bool cpy) = 0;

	virtual EV_EditMouseContext getMouseContext(UT_sint32 xPos, UT_sint32 yPos) = 0;
	virtual bool 	isSelectionEmpty(void) const = 0;

	virtual void	cmdCopy(bool bToClipboard = true) = 0;
	virtual void	cmdCut(void) = 0;
	virtual void	cmdPaste(bool bHonorFormatting = true) = 0;
	virtual void	cmdPasteSelectionAt(UT_sint32 xPos, UT_sint32 yPos) = 0;

	// For touch screen support
	void	setVisualSelectionEnabled(bool bActive)
	{
		m_VisualSelectionActive = bActive;
	}
	bool	getVisualSelectionEnabled(void) const
	{
		return m_VisualSelectionActive;
	}

//
// Let subclasses override but this is here to avoid a crash on frame closing.
// With a selection in place. (Rather than pure virtual.)
	virtual void		cmdUnselectSelection(void) {UT_DEBUGMSG(("Just saved a segfault! \n"));}

	virtual UT_uint32   calculateZoomPercentForPageWidth() const = 0;
	virtual UT_uint32   calculateZoomPercentForPageHeight() const = 0;
	virtual UT_uint32   calculateZoomPercentForWholePage() const = 0;
	void   setLayoutIsFilling(bool bFill) { m_bIsLayoutFilling = bFill;}
	bool   isLayoutFilling(void)  const {return  m_bIsLayoutFilling;}
	virtual UT_uint32	  getPoint(void) const =0;
	virtual void setCursorWait(void) = 0;
	virtual void clearCursorWait(void) = 0;
	bool   isConfigureChanged(void)
	{ return m_bConfigureChanged;}
	void   setConfigure(bool b)
	{ m_bConfigureChanged = b;}

protected:
	XAP_App *			m_pApp;
	void*				m_pParentData;

	UT_sint32			m_xScrollOffset;
	UT_sint32			m_yScrollOffset;
	AV_Focus			m_focus;
	UT_uint32                       m_iTick; // Count changes
	bool				m_bInsertMode;
	bool				m_VisualSelectionActive;

	UT_GenericVector<AV_ScrollObj*>	m_scrollListeners;
	UT_GenericVector<AV_Listener*>	m_vecListeners;

private:
	AV_View(const AV_View&) = delete;
	void operator=(AV_View&) = delete;
	bool m_bIsLayoutFilling;

	UT_sint32			m_iWindowHeight;
	UT_sint32			m_iWindowWidth;
	double				m_dOneTDU;
	bool                            m_bCouldBeActive;
	bool                            m_bConfigureChanged;
};
