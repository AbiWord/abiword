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

#ifndef AP_VIEW_H
#define AP_VIEW_H

#include "xav_View.h"

class GR_Graphics;

class AP_View : public AV_View
{
 public:
	AP_View(XAP_App*, GR_Graphics*, void*);
	~AP_View(void);

	virtual void    setXScrollOffset(UT_sint32);
	virtual void    setYScrollOffset(UT_sint32);
	virtual void    draw(const UT_Rect* pRect=(UT_Rect*) NULL);

	virtual void    cmdScroll(AV_ScrollCmd cmd, UT_uint32 iPos = 0);
	virtual bool notifyListeners(const AV_ChangeMask hint);

	virtual bool canDo(bool bUndo) const;
	virtual void    cmdUndo(UT_uint32 count);
	virtual void    cmdRedo(UT_uint32 count);
	virtual UT_Error cmdSave(void);
	virtual UT_Error cmdSaveAs(const char * szFilename, int ieft);
		
	virtual EV_EditMouseContext getMouseContext(UT_sint32 xPos, UT_sint32 yPos);
	virtual EV_EditMouseContext getInsertionPointContext(UT_sint32 * pxPos, UT_sint32 * pyPos);
	virtual bool 	isSelectionEmpty(void) const;
	virtual void		cmdUnselectSelection(void);


 private:
	GR_Graphics* m_pG;
};

#endif // AP_VIEW_H
