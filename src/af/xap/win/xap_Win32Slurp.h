/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
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

#include <windows.h>
#include <ddeml.h>
class XAP_Win32APP;

class XAP_Win32Slurp
{
public:
	XAP_Win32Slurp(XAP_Win32App * pApp);
	~XAP_Win32Slurp(void);

	HDDEDATA CALLBACK		doCallback(UINT uType, UINT uFmt, HCONV hConv,
									   HSZ hsz1, HSZ hsz2, HDDEDATA hData,
									   DWORD dwData1, DWORD dwData2);
	UT_Bool					connectSlurper(void);
	UT_Bool					disconnectSlurper(void);
	void					processCommand(HDDEDATA hData);
	void					stuffRegistry(const char * szSuffix,
										  const char * szApplicationName,
										  const char * szExePathname,
										  const char * szContentType);

protected:
	UT_Bool					_askForStealFromAnotherApplication(void) const;
	UT_Bool					_askForUpdateExePathname(void) const;
	UT_Bool					_askForStealMimeFromAnotherApplication(void) const;
	
	XAP_Win32App *			m_pApp;
	UT_Bool					m_bInitialized;
	HSZ						m_hszServerName;
	HSZ						m_hszTopic;
	DWORD					m_idDdeServerInst;
};

