 
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
** The Original Code is AbiSource Utilities.
** 
** The Initial Developer of the Original Code is AbiSource, Inc.
** Portions created by AbiSource, Inc. are Copyright (C) 1998 AbiSource, Inc. 
** All Rights Reserved. 
** 
** Contributor(s):
**  
*/

#ifndef UT_DEBUGMSG_H
#define UT_DEBUGMSG_H

void _UT_OutputMessage(char *s, ...);

#ifdef UT_DEBUG
#define UT_DEBUGMSG(M) _UT_OutputMessage M
#else
#define UT_DEBUGMSG(M)
#endif

#endif /* UT_DEBUGMSG_H */
