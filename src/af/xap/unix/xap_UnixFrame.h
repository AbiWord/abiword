 
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

#ifndef AP_UNIXFRAME_H
#define AP_UNIXFRAME_H

#include "ap_Frame.h"
class AP_UnixAp;
class ev_UnixKeyboard;
class EV_UnixMouse;

/*****************************************************************
******************************************************************
** This file defines the unix-platform-specific class for the
** cross-platform application frame.  This is used to hold all
** unix-specific data.  One of these is created for each top-level
** document window.
******************************************************************
*****************************************************************/

class AP_UnixFrame : public AP_Frame
{
public:
	AP_UnixFrame(AP_UnixAp * ap);
	~AP_UnixFrame(void);

	virtual UT_Bool				initialize(int argc, char ** argv);

protected:
	// TODO see why ev_UnixKeyboard has lowercase prefix...
	AP_UnixAp *					m_pUnixAp;
	ev_UnixKeyboard *			m_pUnixKeyboard;
	EV_UnixMouse *				m_pUnixMouse;
};

#endif /* AP_UNIXFRAME_H */
