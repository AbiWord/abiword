 
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

#ifndef AP_MENU_LABELSET_H
#define AP_MENU_LABELSET_H

#include "ev_Menu_Labels.h"

EV_Menu_LabelSet * AP_CreateMenuLabelSet(const char * szLanguage);
UT_uint32 AP_GetMenuLabelSetLanguageCount(void);
const char * AP_GetNthMenuLabelLanguageName(UT_uint32 ndx);

#endif /* AP_MENU_LABELSET_H */
