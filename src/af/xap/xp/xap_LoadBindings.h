
 
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


#ifndef AP_LOADBINDINGS_H
#define AP_LOADBINDINGS_H
class EV_EditMethodContainer;
class EV_EditBindingMap;

UT_Bool AP_LoadBindings(const char * szName, EV_EditMethodContainer * pemc,
						EV_EditBindingMap **ppebm);

#endif /* AP_LOADBINDINGS_H */

