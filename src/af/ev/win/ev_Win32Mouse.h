 
/*
** The contents of this file are subject to the AbiSource Public
** License Version 1.0 (the "License"); you may not use this file
** except in compliance with the License. You may obtain a copy
** of the License at http://www.abisource.com/LICENSE/ 
** 
** Software distributed under the License is distributed on an
** "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
** implied. See the License for the specific language governing
** rights and limitations under the License. 
** 
** The Original Code is AbiWord.
** 
** The Initial Developer of the Original Code is AbiSource, Inc.
** Portions created by AbiSource, Inc. are Copyright (C) 1998 AbiSource, Inc. 
** All Rights Reserved. 
** 
** Contributor(s):
**  
*/


#ifndef EV_WIN32MOUSE_H
#define EV_WIN32MOUSE_H

#include "ev_Mouse.h"
#include "ev_EditBits.h"

// TODO should pView be passed in on each method or
// TODO should we pass it into EV_Mouse on the constructor ??
// TODO EV_<platform>Mouse and EV_EditEventMapper are
// TODO unique for each document, although the EditBindings
// TODO may be global.  (ev_<platform>Mouse could be global
// TODO i suppose.)

class FV_View;


class ev_Win32Mouse : public EV_Mouse
{
public:
	ev_Win32Mouse(EV_EditEventMapper * pEEM);

	void reset(void);

	void onButtonDown(FV_View * pView, HWND hWnd, EV_EditMouseButton emb, WPARAM fwKeys, WPARAM xPos, WPARAM yPos);
	void onButtonUp  (FV_View * pView, HWND hWnd, EV_EditMouseButton emb, WPARAM fwKeys, WPARAM xPos, WPARAM yPos);
	void onButtonMove(FV_View * pView, HWND hWnd, WPARAM fwKeys, WPARAM xPos, WPARAM yPos);
	void onDoubleClick(FV_View * pView, HWND hWnd, EV_EditMouseButton emb, WPARAM fwKeys, WPARAM xPos, WPARAM yPos);

protected:
	UT_uint32			m_iCaptureCount;
	EV_EditMouseButton	m_embCaptured;
};

#endif /* EV_WIN32MOUSE_H */
