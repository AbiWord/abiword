/* AbiWord
 * Copyright (C) 2004 Marc Maurer
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

#ifndef RV_VIEW_H
#define RV_VIEW_H

#include "ut_types.h"
#include "xav_View.h"
#include "rl_DocLayout.h"

class ABI_EXPORT rv_View : public AV_View
{
public:
	rv_View(XAP_App*, void*, rl_DocLayout*);
	virtual ~rv_View();

	virtual inline GR_Graphics*	getGraphics(void) const { return m_pG; }
	void  setGraphics(GR_Graphics *pG) { m_pG = pG; }

	virtual void focusChange(AV_Focus focus);

	virtual void	setXScrollOffset(UT_sint32);
	virtual void	setYScrollOffset(UT_sint32);
	virtual void    setCursorToContext(void);

	virtual void	      draw(const UT_Rect* pClipRect=static_cast<UT_Rect*>(NULL));
	virtual void	      updateScreen(bool bDirtyRunsOnly=true);
    virtual void          updateLayout(void);
	virtual void	cmdScroll(AV_ScrollCmd cmd, UT_uint32 iPos = 0);

	virtual bool	notifyListeners(const AV_ChangeMask hint);
	virtual bool    isDocumentPresent(void) { return true; }
	virtual bool	canDo(bool bUndo) const { return true; }
	virtual void	cmdUndo(UT_uint32 count) {}
	virtual void	cmdRedo(UT_uint32 count) {}
	virtual UT_Error	cmdSave(void) { return UT_OK; }
	virtual UT_Error	cmdSaveAs(const char * szFilename, int ieft) { return UT_OK; }
	virtual UT_Error        cmdSaveAs(const char * szFilename, int ieft, bool cpy) { return UT_OK; }

	virtual EV_EditMouseContext getMouseContext(UT_sint32 xPos, UT_sint32 yPos) { return EV_EMC_UNKNOWN; }
	virtual bool 	isSelectionEmpty(void) const { return true; }

	virtual void	cmdCopy(bool bToClipboard = true) {}
	virtual void	cmdCut(void) {}
	virtual void	cmdPaste(bool bHonorFormatting = true) {}
	virtual void	cmdPasteSelectionAt(UT_sint32 xPos, UT_sint32 yPos) {}
	virtual void	cmdUnselectSelection(void) {}

	virtual UT_uint32   calculateZoomPercentForPageWidth() { return UT_OK; }
	virtual UT_uint32   calculateZoomPercentForPageHeight() { return UT_OK; }
	virtual UT_uint32   calculateZoomPercentForWholePage() { return UT_OK; }

	virtual UT_uint32	  getPoint(void) const { return 0; }
	virtual void setCursorWait(void) {}
	virtual void clearCursorWait(void) {}
		
protected:
	virtual void _draw(UT_sint32 x, UT_sint32 y,
			UT_sint32 width, UT_sint32 height,
			bool bClip);

private:
	rl_DocLayout *m_pLayout;
	GR_Graphics *m_pG;
};

#endif /* RV_VIEW_H */
