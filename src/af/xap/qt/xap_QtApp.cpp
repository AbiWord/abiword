/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
/* AbiSource Application Framework
 * Copyright (C) 2012 Hubert Figuiere
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#include <sys/stat.h>
#include <sys/types.h>

#include "xap_QtApp.h"

XAP_QtApp::XAP_QtApp(const char* name)
	: XAP_App(name)
	, m_dialogFactory(this)
	, m_app(NULL)
{
}

XAP_QtApp::~XAP_QtApp()
{
}

void XAP_QtApp::reallyExit()
{
#warning TODO
}

int XAP_QtApp::exec()
{
#warning TODO
//	return m_app->exec();
}

// TODO refactor with XAP_UnixApp::makeDirectory()
UT_sint32 XAP_QtApp::makeDirectory(const char * szPath, const UT_sint32 mode ) const
{
	return mkdir(szPath, mode);
}

// TODO refactor with XAP_UnixApp::getUserPrivateDirectory()
const char * XAP_QtApp::getUserPrivateDirectory()
{
	/* return a pointer to a static buffer */
    static char *buf = NULL;

    if (buf == NULL)
    {
		const char * szAbiDir = "abiword";
		const char * szCfgDir = ".config";

		const char * szXDG = getenv("XDG_CONFIG_HOME");
		if (!szXDG || !*szXDG) {
			const char * szHome = getenv("HOME");
			if (!szHome || !*szHome)
				szHome = "./";

			buf = new char[strlen(szHome)+strlen(szCfgDir)+strlen(szAbiDir)+4];

			strcpy(buf, szHome);
			if (buf[strlen(buf)-1] != '/')
				strcat(buf, "/");
			strcat(buf, szCfgDir);
		} else {
			buf = new char[strlen(szXDG)+strlen(szAbiDir)+4];
			strcpy(buf, szXDG);
		}

		strcat(buf, "/");
		strcat(buf, szAbiDir);

#ifdef PATH_MAX
        if (strlen(buf) >= PATH_MAX)
            DELETEPV(buf);
#endif

		// migration / legacy
		migrate("/AbiSuite", szAbiDir, buf); 
    }

	return buf;
}
