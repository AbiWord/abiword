 
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

#ifndef EV_UNIXKEYBOARD_H
#define EV_UNIXKEYBOARD_H

#include "ev_Keyboard.h"
#include "ev_EditBits.h"

class FV_View;

class ev_UnixKeyboard : public EV_Keyboard
{
public:
	ev_UnixKeyboard(EV_EditEventMapper * pEEM);

	UT_Bool keyPressEvent(FV_View * pView, GdkEventKey* e);
};

#endif // EV_UNIXKEYBOARD_H

