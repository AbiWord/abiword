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

#include <QApplication>

#include "ut_uuid.h"
#include "xap_QtApp.h"
#include "gr_QtGraphics.h"

XAP_QtApp::XAP_QtApp(const char* name)
	: XAP_App(name)
	, m_dialogFactory(this)
	, m_controlFactory()
	, m_qtArgc(1)
	, m_qtArgv(name)
{
	// create an instance of UT_UUIDGenerator or appropriate derrived class
	_setUUIDGenerator(new UT_UUIDGenerator());

	_setAbiSuiteLibDir();

	// register graphics allocator
	GR_GraphicsFactory * pGF = getGraphicsFactory();
	UT_ASSERT( pGF );

	if(pGF)
	{
		bool bSuccess;
		bSuccess = pGF->registerClass(GR_QtGraphics::graphicsAllocator,
									   GR_QtGraphics::graphicsDescriptor,
									   GR_QtGraphics::s_getClassId());

		// we are in deep trouble if this did not succeed
		UT_ASSERT( bSuccess );
		pGF->registerAsDefault(GR_QtGraphics::s_getClassId(), true);

		m_app = new QApplication((int&) m_qtArgc, (char**) &m_qtArgv, 0); 
	}
}

XAP_QtApp::~XAP_QtApp()
{
}

void XAP_QtApp::reallyExit()
{
	m_app->quit();
}

int XAP_QtApp::exec()
{
	return m_app->exec();
}

// TODO refactor with XAP_UnixApp::getUserPrivateDirectory()
const char * XAP_QtApp::getUserPrivateDirectory() const
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

// TODO refactor with XAP_UnixApp::_setAbiSuiteLibDir()
void XAP_QtApp::_setAbiSuiteLibDir()
{
	// FIXME: this code sucks hard

	char * buf = NULL;
	
	// see if ABIWORD_DATADIR was set in the environment
	const char * sz = getenv("ABIWORD_DATADIR");
	if (sz && *sz)
	{
		int len = strlen(sz);
		buf = (gchar *)g_malloc(len+1);
		strcpy(buf,sz);
		char * p = buf;
		if ( (p[0]=='"') && (p[len-1]=='"') )
		{
			// trim leading and trailing DQUOTES
			p[len-1]=0;
			p++;
			len -= 2;
		}
		if (p[len-1]=='/')				// trim trailing slash
			p[len-1] = 0;
		XAP_App::_setAbiSuiteLibDir(p);
		g_free(buf);
		return;
	}

	// otherwise, use the hard-coded value
	XAP_App::_setAbiSuiteLibDir(getAbiSuiteHome());

	return;
}
