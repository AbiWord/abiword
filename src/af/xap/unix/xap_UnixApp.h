 
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

#ifndef AP_UNIXAP_H
#define AP_UNIXAP_H

#include "ap_Ap.h"

/*****************************************************************
******************************************************************
** This file defines the unix-platform-specific class for the
** cross-platform application.  This is used to hold all of the
** platform-specific, application-specific data.  Only one of these
** is created by the application.
******************************************************************
*****************************************************************/

class AP_UnixAp : public AP_Ap
{
public:
	AP_UnixAp(void);
	virtual ~AP_UnixAp(void);

	virtual UT_Bool					initialize(int argc, char ** argv);

protected:

	/* TODO put anything we need here.  for example, our
	** TODO connection to the XServer.
	*/
};

#endif /* AP_UNIXAP_H */
