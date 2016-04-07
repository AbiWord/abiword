/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

/*
 * Port to Maemo Development Platform 
 * Author: INdT - Renato Araujo <renato.filho@indt.org.br>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib.h>
#include <gtk/gtk.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#include <sys/stat.h>

#include <fontconfig/fontconfig.h>

#include "ut_debugmsg.h"
#include "ut_path.h"
#include "ut_string.h"
#include "ut_uuid.h"

#include "xap_UnixDialogHelper.h"
#include "xap_Strings.h"
#include "xap_UnixApp.h"
#include "xap_FakeClipboard.h"
#include "gr_UnixImage.h"
#include "xap_Unix_TB_CFactory.h"
#include "xap_Prefs.h"
#include "xap_UnixEncodingManager.h"

#include "gr_UnixCairoGraphics.h"
#include <glib/gstdio.h>
#include <gsf/gsf-utils.h>
#include <goffice/goffice.h>

#include "gr_CairoNullGraphics.h"
static CairoNull_Graphics * nullgraphics = NULL;

/*****************************************************************/

XAP_UnixApp::XAP_UnixApp(const char * szAppName)
	: XAP_App(szAppName),
	  m_dialogFactory(this),
	  m_controlFactory(),
	  m_szTmpFile(NULL)
{
	int fc_inited = FcInit();
	UT_UNUSED(fc_inited); // TODO actually deal with the error here
	UT_ASSERT(fc_inited);

	_setAbiSuiteLibDir();

	memset(&m_geometry, 0, sizeof(m_geometry));

	// create an instance of UT_UUIDGenerator or appropriate derrived class
	_setUUIDGenerator(new UT_UUIDGenerator());

	// register graphics allocator
	GR_GraphicsFactory * pGF = getGraphicsFactory();
	UT_ASSERT( pGF );

	if(pGF)
	{
		bool bSuccess;
		bSuccess = pGF->registerClass(GR_UnixCairoGraphics::graphicsAllocator,
									  GR_UnixCairoGraphics::graphicsDescriptor,
									  GR_UnixCairoGraphics::s_getClassId());

		UT_ASSERT( bSuccess );

		if(bSuccess)
		{
			pGF->registerAsDefault(GR_UnixCairoGraphics::s_getClassId(), true);
		}

		bSuccess = pGF->registerClass(CairoNull_Graphics::graphicsAllocator,
									  CairoNull_Graphics::graphicsDescriptor,
									  CairoNull_Graphics::s_getClassId());
		UT_ASSERT( bSuccess );

		/* We need to link CairoNull_Graphics because the AbiCommand
		 * plugin uses it.
		 *
		 * We do not need to keep an instance of it around though, as the
		 * plugin constructs its own (I wonder if there is a more elegant way
		 * to force the linker into including it).
		 */
		{
			GR_CairoNullGraphicsAllocInfo ai;
			nullgraphics =
				(CairoNull_Graphics*) XAP_App::getApp()->newGraphics((UT_uint32)GRID_CAIRO_NULL, ai);

			delete nullgraphics;
			nullgraphics = NULL;
		}
	}
}

XAP_UnixApp::~XAP_UnixApp()
{
	removeTmpFile();
//	FcFini();
}

void XAP_UnixApp::removeTmpFile(void)
{
	if(m_szTmpFile)
	{
		if(g_file_test(m_szTmpFile,G_FILE_TEST_EXISTS))
		{
		//
		// Remove the tempfile if it exists
		//
			g_unlink(m_szTmpFile);
			delete [] m_szTmpFile;
		}
	}
	m_szTmpFile = NULL;
}

bool XAP_UnixApp::initialize(const char * szKeyBindingsKey, const char * szKeyBindingsDefaultValue)
{
	// let our base class do it's thing.
	
	XAP_App::initialize(szKeyBindingsKey, szKeyBindingsDefaultValue);

	libgoffice_init();

	// do any thing we need here...

	return true;
}

void XAP_UnixApp::shutdown()
{
	libgoffice_shutdown();
}

void XAP_UnixApp::reallyExit()
{
	
	gtk_main_quit();
}

UT_sint32 XAP_UnixApp::makeDirectory(const char * szPath, const UT_sint32 mode ) const
{ 
  return mkdir(szPath, mode); 
}

XAP_DialogFactory * XAP_UnixApp::getDialogFactory()
{
	return &m_dialogFactory;
}

XAP_Toolbar_ControlFactory * XAP_UnixApp::getControlFactory()
{
	return &m_controlFactory;
}

void XAP_UnixApp::setWinGeometry(int x, int y, UT_uint32 width, UT_uint32 height,
								 UT_uint32 flags)
{
	// TODO : do some range checking?
	m_geometry.x = x;
	m_geometry.y = y;
	m_geometry.width = width;
	m_geometry.height = height;
	m_geometry.flags = flags;
}

void XAP_UnixApp::getWinGeometry(int * x, int * y, UT_uint32 * width,
								 UT_uint32 * height, UT_uint32 * flags)
{
	UT_return_if_fail(x && y && width && height);
	*x = m_geometry.x;
	*y = m_geometry.y;
	*width = m_geometry.width;
	*height = m_geometry.height;
	*flags = m_geometry.flags;
}

// This should be removed at some time.
void XAP_UnixApp::migrate(const char *oldName,
                          const char *newName, const char *path) const
{
    if (path && newName && oldName && (*oldName == '/')) {

        const char* end = strrchr(path, '/');
        if (!end) {
            UT_WARNINGMSG(("invalid path '%s', '/' not found", path));
            return;
        }

        std::string old(path, end);
        old += oldName;

        if (g_access(old.c_str(), F_OK) == 0) {
            UT_WARNINGMSG(("Renaming: %s -> %s\n", old.c_str(), path));
            g_rename(old.c_str(), path);
        }
    }
}

const char * XAP_UnixApp::getUserPrivateDirectory() const
{
    /* return a pointer to a static buffer */
    static std::string private_dir;

    if (private_dir.empty()) {
        const char * szAbiDir = "abiword";
        const char * szCfgDir = ".config";

        const char * szXDG = getenv("XDG_CONFIG_HOME");
        if (!szXDG || !*szXDG) {
            const char * szHome = getenv("HOME");
            if (!szHome || !*szHome)
                szHome = "./";

            private_dir = szHome;
            if (szHome[strlen(szHome)-1] != '/') {
                private_dir.push_back('/');
            }
            private_dir += szCfgDir;
        } else {
            private_dir = szXDG;
        }

        private_dir += '/';
        private_dir += szAbiDir;

        // migration / legacy
        // XXX shouldn't that be /.AbiSuite ?
        migrate("/AbiSuite", szAbiDir, private_dir.c_str());
    }

    return private_dir.c_str();
}


void XAP_UnixApp::_setAbiSuiteLibDir()
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

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

void XAP_UnixApp::setTimeOfLastEvent(UT_uint32 eventTime)
{
	m_eventTime = eventTime;
}

