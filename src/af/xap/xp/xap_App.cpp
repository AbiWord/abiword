/* AbiSource Application Framework
 * Copyright (C) 1998,1999 AbiSource, Inc.
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

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>

#include "ut_types.h"
#include "ut_assert.h"
#include "ut_string.h"
#include "ut_debugmsg.h"
#include "ev_EditMethod.h"
#include "ev_Menu_Actions.h"
#include "ev_Toolbar_Actions.h"
#include "xap_App.h"
#include "xap_Args.h"
#include "gr_Image.h"
#include "xap_Frame.h"
#include "xap_EditMethods.h"
#include "xap_Menu_ActionSet.h"
#include "xap_Toolbar_ActionSet.h"
#include "xap_LoadBindings.h"
#include "xap_Dictionary.h"
#include "xap_Prefs.h"
#include "xap_EncodingManager.h"


/*****************************************************************/

XAP_App * XAP_App::m_pApp = NULL;

XAP_App::XAP_App(XAP_Args * pArgs, const char * szAppName) : m_hashClones(5)
{
	UT_ASSERT(szAppName && *szAppName);

	m_pArgs = pArgs;
	m_szAppName = szAppName;
	m_szAbiSuiteLibDir = NULL;
	m_pEMC = NULL;
	m_pBindingSet = NULL;
	m_pMenuActionSet = NULL;
	m_pToolbarActionSet = NULL;
	m_pDict = NULL;
	m_prefs = NULL;
	m_pEncMgr = new XAP_EncodingManager();/* HACK: this is done in order 
		not to update the code for each platform. If platform specific
		code has its own implementation of EncodingManager, then it
		should just simply delete an instance pointed by this member
		and assign ptr to platform-specific implementation. - hvv */
       
	m_pApp = this;
	m_lastFocussedFrame = NULL;
        clearIdTable();
}

XAP_App::~XAP_App(void)
{
	// HACK: for now, this works from XAP code
	// TODO: where should this really go?
	if (m_pDict)
		m_pDict->save();

	// run thru and destroy all frames on our window list.
	UT_VECTOR_PURGEALL(XAP_Frame *, m_vecFrames);

	FREEP(m_szAbiSuiteLibDir);
	DELETEP(m_pEMC);
	DELETEP(m_pBindingSet);
	DELETEP(m_pMenuActionSet);
	DELETEP(m_pToolbarActionSet);
	DELETEP(m_pDict);
	DELETEP(m_prefs);
	DELETEP(m_pEncMgr);
}

const XAP_EncodingManager* XAP_App::getEncodingManager(void) const 
{ 
    return m_pEncMgr;
};

bool XAP_App::initialize(void)
{
	// create application-wide resources that
	// are shared by everything.

#if 0 
	m_pEMC = AP_GetEditMethods();
	UT_ASSERT(m_pEMC);

	m_pBindingSet = new AP_BindingSet(m_pEMC);
	UT_ASSERT(m_pBindingSet);
	
	m_pMenuActionSet = AP_CreateMenuActionSet();
	UT_ASSERT(m_pMenuActionSet);

	m_pToolbarActionSet = AP_CreateToolbarActionSet();
	UT_ASSERT(m_pToolbarActionSet);
#endif

	// HACK: for now, this works from XAP code
	// TODO: where should this really go?
	char * szPathname = UT_catPathname(getUserPrivateDirectory(),"custom.dic");
	UT_ASSERT(szPathname);
	m_pDict = new XAP_Dictionary(szPathname);
	FREEP(szPathname);
	UT_ASSERT(m_pDict);
	m_pDict->load();
        clearIdTable();

	// TODO use m_pArgs->{argc,argv} to process any command-line
	// TODO options that we need.
	//
	// Need to initialize the random number generator. 
	//
	UT_uint32 t = (UT_uint32) time( NULL);
	srand(t);
	return true;
}

const char * XAP_App::getApplicationTitleForTitleBar(void) const
{
	static char _title[512];

	// return a string that the platform-specific code
	// can copy to the title bar of a window.

	//sprintf(_title, "%s (www.abisource.com)", m_szAppName);
	sprintf(_title, "%s", m_szAppName);

	return _title;
}

const char * XAP_App::getApplicationName(void) const
{
	// return a string that the platform-specific code
	// can use as a class name for various window-manager-like
	// operations.
	return m_szAppName;
}

EV_EditMethodContainer * XAP_App::getEditMethodContainer(void) const
{
	return m_pEMC;
}

EV_EditBindingMap * XAP_App::getBindingMap(const char * szName)
{
	UT_ASSERT(m_pBindingSet);
	return m_pBindingSet->getMap(szName);
}

const EV_Menu_ActionSet * XAP_App::getMenuActionSet(void) const
{
	return m_pMenuActionSet;
}

const EV_Toolbar_ActionSet * XAP_App::getToolbarActionSet(void) const
{
	return m_pToolbarActionSet;
}

bool XAP_App::rememberFrame(XAP_Frame * pFrame, XAP_Frame * pCloneOf)
{
	UT_ASSERT(pFrame);

	// add this frame to our window list
	m_vecFrames.addItem(pFrame);

	if(! m_lastFocussedFrame)
	    rememberFocussedFrame(pFrame);

	// TODO: error-check the following for mem failures
	if (pCloneOf)
	{
		// locate vector of this frame's clones
		UT_HashEntry* pEntry = m_hashClones.findEntry(pCloneOf->getViewKey());

		UT_Vector * pvClones = NULL;

		if (pEntry)
		{
			// hash table entry already exists
			pvClones = (UT_Vector *) pEntry->pData;

			if (!pvClones)
			{
				// nothing there, so create a new one
				pvClones = new UT_Vector();
				UT_ASSERT(pvClones);

				pvClones->addItem(pCloneOf);

				// reuse this slot
				m_hashClones.setEntry(pEntry, NULL, pvClones);
			}
		}
		else
		{
			// create a new one
			pvClones = new UT_Vector();
			UT_ASSERT(pvClones);

			pvClones->addItem(pCloneOf);

			// add it to the hash table
			m_hashClones.addEntry(pCloneOf->getViewKey(), NULL, pvClones);
		}

		pvClones->addItem(pFrame);

		// notify all clones of their new view numbers
		for (UT_uint32 j=0; j<pvClones->getItemCount(); j++)
		{
			XAP_Frame * f = (XAP_Frame *) pvClones->getNthItem(j);
			UT_ASSERT(f);

			f->setViewNumber(j+1);

			if (f != pFrame)
				f->updateTitle();
		}
	}
	
	// TODO do something here...
	return true;
}

bool XAP_App::forgetFrame(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);


        // If this frame is the currently focussed frame write in NULL
        // until another frame appears

	if(pFrame == m_lastFocussedFrame )
	{
		m_lastFocussedFrame = (XAP_Frame *) NULL;
	}

	if (pFrame->getViewNumber() > 0)
	{
		// locate vector of this frame's clones
		UT_HashEntry* pEntry = m_hashClones.findEntry(pFrame->getViewKey());
		UT_ASSERT(pEntry);

		if (pEntry)
		{
			UT_Vector * pvClones = (UT_Vector *) pEntry->pData;
			UT_ASSERT(pvClones);

			// remove this frame from the vector
			UT_sint32 i = pvClones->findItem(pFrame);
			UT_ASSERT(i >= 0);

			if (i >= 0)
			{
				pvClones->deleteNthItem(i);
			}

			// see how many clones are left
			UT_uint32 count = pvClones->getItemCount();
			UT_ASSERT(count > 0);
			XAP_Frame * f = NULL;

			if (count == 1)
			{
				// remaining clone is now a singleton
				f = (XAP_Frame *) pvClones->getNthItem(count-1);
				UT_ASSERT(f);

				f->setViewNumber(0);
				f->updateTitle();

				// remove this entry from hashtable
				m_hashClones.setEntry(pEntry, NULL, NULL);
				delete pvClones;
			}
			else
			{
				// notify remaining clones of their new view numbers
				for (UT_uint32 j=0; j<count; j++)
				{
					f = (XAP_Frame *) pvClones->getNthItem(j);
					UT_ASSERT(f);

					f->setViewNumber(j+1);
					f->updateTitle();
				}
			}
		}
	}

	// remove this frame from our window list
	UT_sint32 ndx = m_vecFrames.findItem(pFrame);
	UT_ASSERT(ndx >= 0);

	if (ndx >= 0)
	{
		m_vecFrames.deleteNthItem(ndx);
	}

	notifyModelessDlgsCloseFrame(pFrame);

	// TODO do something here...

	return true;
}

bool XAP_App::forgetClones(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);

	if (pFrame->getViewNumber() == 0)
	{
		return forgetFrame(pFrame);
	}

	UT_Vector vClones;
	getClones(&vClones, pFrame);
	
	for (UT_uint32 i = 0; i < vClones.getItemCount(); i++)
	{
		XAP_Frame * f = (XAP_Frame *) vClones.getNthItem(i);
		forgetFrame(f);
	}

	return true;
}

bool XAP_App::getClones(UT_Vector *pvClonesCopy, XAP_Frame * pFrame)
{
	UT_ASSERT(pvClonesCopy);
	UT_ASSERT(pFrame);
	UT_ASSERT(pFrame->getViewNumber() > 0);

	// locate vector of this frame's clones
	UT_HashEntry* pEntry = m_hashClones.findEntry(pFrame->getViewKey());
	UT_ASSERT(pEntry);

	UT_Vector * pvClones = (UT_Vector *) pEntry->pData;
	UT_ASSERT(pvClones);

	return pvClonesCopy->copy(pvClones);
}

bool XAP_App::updateClones(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);
	UT_ASSERT(pFrame->getViewNumber() > 0);

	// locate vector of this frame's clones
	UT_HashEntry* pEntry = m_hashClones.findEntry(pFrame->getViewKey());
	UT_ASSERT(pEntry);

	if (pEntry)
	{
		UT_Vector * pvClones = (UT_Vector *) pEntry->pData;
		UT_ASSERT(pvClones);

		UT_uint32 count = pvClones->getItemCount();
		UT_ASSERT(count > 0);
		XAP_Frame * f = NULL;

		for (UT_uint32 j=0; j<count; j++)
		{
			f = (XAP_Frame *) pvClones->getNthItem(j);
			UT_ASSERT(f);

			f->updateTitle();
		}
	}

	return true;
}

UT_uint32 XAP_App::getFrameCount(void) const
{
	return m_vecFrames.getItemCount();
}

XAP_Frame * XAP_App::getFrame(UT_uint32 ndx) const
{
	XAP_Frame * pFrame = NULL;
	
	if (ndx < m_vecFrames.getItemCount())
	{
		pFrame = (XAP_Frame *) m_vecFrames.getNthItem(ndx);
		UT_ASSERT(pFrame);
	}
	return pFrame;
}
	
UT_sint32 XAP_App::findFrame(XAP_Frame * pFrame)
{
	return m_vecFrames.findItem(pFrame);
}
	
UT_sint32 XAP_App::findFrame(const char * szFilename)
{
	if (!szFilename || !*szFilename)
		return -1;

	for (UT_uint32 i=0; i<getFrameCount(); i++)
	{
		XAP_Frame * f = getFrame(i);
		UT_ASSERT(f);
		const char * s = f->getFilename();

		if (s && *s && (0 == UT_stricmp(szFilename, s)))
		{
			return (UT_sint32) i;
		}
	}

	return -1;
}

void XAP_App::_setAbiSuiteLibDir(const char * sz)
{
	FREEP(m_szAbiSuiteLibDir);
	UT_cloneString((char *&)m_szAbiSuiteLibDir,sz);
}

const char * XAP_App::getAbiSuiteLibDir(void) const
{
	return m_szAbiSuiteLibDir;
}

bool XAP_App::addWordToDict(const UT_UCSChar * pWord, UT_uint32 len)
{
	if (!m_pDict)
		return false;

	return m_pDict->addWord(pWord, len);
}

bool XAP_App::isWordInDict(const UT_UCSChar * pWord, UT_uint32 len) const
{
	if (!m_pDict)
		return false;

	return m_pDict->isWord(pWord, len);
}

XAP_Prefs * XAP_App::getPrefs(void) const
{
	return m_prefs;
}

#ifdef HAVE_LIBXML2
bool XAP_App::getPrefsValue(const char * szKey, const XML_Char ** pszValue) const
{
	if (!m_prefs)
		return false;

	return m_prefs->getPrefsValue((XML_Char*) (szKey), pszValue);
}
#endif

bool XAP_App::getPrefsValue(const XML_Char * szKey, const XML_Char ** pszValue) const
{
	if (!m_prefs)
		return false;

	return m_prefs->getPrefsValue(szKey,pszValue);
}

#ifdef HAVE_LIBXML2
bool XAP_App::getPrefsValueBool(const char * szKey, bool * pbValue) const
{
	if (!m_prefs)
		return false;

	return m_prefs->getPrefsValueBool((XML_Char*) (szKey), pbValue);
}
#endif

bool XAP_App::getPrefsValueBool(const XML_Char * szKey, bool * pbValue) const
{
	if (!m_prefs)
		return false;

	return m_prefs->getPrefsValueBool(szKey,pbValue);
}

void XAP_App::rememberFocussedFrame( void * pJustFocussedFrame)
{
	m_lastFocussedFrame = (XAP_Frame *) pJustFocussedFrame;

	UT_sint32 i = safefindFrame( m_lastFocussedFrame);
	if(i < 0 ) 
	{   
		m_lastFocussedFrame = (XAP_Frame *) NULL;
	}
	notifyModelessDlgsOfActiveFrame(m_lastFocussedFrame);
}

UT_sint32 XAP_App::safefindFrame( XAP_Frame * f)
{
        long ff = reinterpret_cast<long>(f);
        UT_sint32 num_frames = m_vecFrames.getItemCount();
        UT_sint32 i;
        for( i = 0; i< num_frames; i++)
	{
	       long lf = reinterpret_cast<long>( m_vecFrames.getNthItem(i));
	       if( lf == ff) break;
        }
	if( i == num_frames ) i = -1;
        return i;
}

void    XAP_App::clearLastFocussedFrame(void)
{
        m_lastFocussedFrame = (XAP_Frame *) NULL;
}

XAP_Frame * XAP_App::getLastFocussedFrame( void ) 
{
	if(m_lastFocussedFrame == (XAP_Frame *) NULL)
		return (XAP_Frame *) NULL;
        UT_sint32 i = safefindFrame(m_lastFocussedFrame);
        if( i>= 0)
		return m_lastFocussedFrame;
        return (XAP_Frame *) NULL;
}

XAP_Frame * XAP_App::findValidFrame( void)
{
        XAP_Frame * validFrame =  getFrame(0);
        return validFrame;
}

void XAP_App::clearIdTable( void)
{
        for(UT_sint32 i =0; i <= NUM_MODELESSID; i++)
        {
                m_IdTable[i].id =  -1;
                m_IdTable[i].pDialog = (XAP_Dialog_Modeless *) NULL;
	}
}

void XAP_App::rememberModelessId(UT_sint32 id, XAP_Dialog_Modeless * pDialog)
{

  // find a free slot in the m_IdTable
 
        UT_sint32 i;
        for(i=0; (i<= NUM_MODELESSID) && (m_IdTable[i].id !=  -1); i++);
        UT_ASSERT( i <= NUM_MODELESSID );
        UT_ASSERT( m_IdTable[i].id == -1 );
        UT_ASSERT( pDialog);
        m_IdTable[i].id =  id;
        m_IdTable[i].pDialog =  pDialog;
}

void XAP_App::forgetModelessId( UT_sint32 id )
{

  // remove the id, pDialog pair from the m_IdTable

        UT_sint32 i;
        for(i=0; i <= NUM_MODELESSID && m_IdTable[i].id != id; i++) ;
        UT_ASSERT( i <= NUM_MODELESSID );
        UT_ASSERT( m_IdTable[i].id == id );
        m_IdTable[i].id =  -1;
        m_IdTable[i].pDialog = (XAP_Dialog_Modeless *) NULL;
}

bool XAP_App::isModelessRunning(UT_sint32 id)
{
  // returns true if the modeless dialog given by id is running

        UT_sint32 i;
        for(i=0; i <= NUM_MODELESSID && m_IdTable[i].id != id; i++) ;
        if( i> NUM_MODELESSID)
	{
	     return false;
	}
        UT_ASSERT( m_IdTable[i].id == id );
        return true;
}
        
XAP_Dialog_Modeless * XAP_App::getModelessDialog( UT_sint32 i)
{

  // Retrieve pDialog from the table based on its location in the table

	return m_IdTable[i].pDialog;
}

void XAP_App::closeModelessDlgs( void )
{
	  // Delete all the open modeless dialogs

        for(UT_sint32 i=0; i <= NUM_MODELESSID; i++)
        {
	       if(getModelessDialog(i) != (XAP_Dialog_Modeless *) NULL)
	       {
	             getModelessDialog(i)->destroy();
	       }
	}
}


void XAP_App::notifyModelessDlgsOfActiveFrame(XAP_Frame *p_Frame)
{
	for(UT_sint32 i=0; i <= NUM_MODELESSID; i++)
	{
		if(getModelessDialog(i) != (XAP_Dialog_Modeless *) NULL)
		{
			getModelessDialog(i)->setActiveFrame(p_Frame);
		}
	}
}

void XAP_App::notifyModelessDlgsCloseFrame(XAP_Frame *p_Frame)
{
	for(UT_sint32 i=0; i <= NUM_MODELESSID; i++)
	{
		if(getModelessDialog(i) != (XAP_Dialog_Modeless *) NULL)
		{
			getModelessDialog(i)->notifyCloseFrame(p_Frame);
		}
	}
}

/* Window Geometry Preferences */
bool XAP_App::setGeometry(UT_sint32 x, UT_sint32 y, UT_uint32 width, UT_uint32 height , UT_uint32 flags) 
{
	XAP_Prefs *prefs = getPrefs();
	return prefs->setGeometry(x, y, width, height, flags);
}

bool XAP_App::getGeometry(UT_sint32 *x, UT_sint32 *y, UT_uint32 *width, UT_uint32 *height, UT_uint32 *flags) 
{
	XAP_Prefs *prefs = getPrefs();
	return prefs->getGeometry(x, y, width, height, flags);
}

void XAP_App::parseAndSetGeometry(const char *string) {
	UT_uint32 nw, nh, nflags;
    UT_sint32 nx, ny;
    char *next;

	nw = nh = nflags = 0;
	nx = ny = 0;

    next = (char *)string;
    if (*next != '+' && *next != '-') {
        nw = strtoul(next, &next, 10);
        if(*next == 'x' || *next == 'X') {
            nh = strtoul(++next, &next, 10);
            nflags |= PREF_FLAG_GEOMETRY_SIZE;
        }
    }
    if (*next == '+' || *next == '-') {
        nx = strtoul(next, &next, 10);
        if(*next == '+' || *next == '-') {
            ny = strtoul(next, &next, 10);
            nflags |= PREF_FLAG_GEOMETRY_POS;
        }
    }

	//Don't update the geometry from the file
	if(nflags) {
		nflags |= PREF_FLAG_GEOMETRY_NOUPDATE;
		setGeometry(nx, ny, nw, nh, nflags);
	}
} 

void XAP_App::_printUsage(void)
{
	// just print to stdout, not stderr
	printf("\nUsage: %s [option]... [file]...\n\n", m_pArgs->m_argv[0]);
	printf("  -to               The target format of the file\n");
        printf("                    (abw, zabw, rtf, txt, utf8, html, latex)\n");
	printf("  -verbose          The verbosity level (0, 1, 2)\n");
	printf("  -show             If you really want to start the GUI\n");
        printf("                    (even if you use the -to option)\n");
#ifdef DEBUG
	printf("  -dumpstrings      dump strings strings to file\n");
#endif
	printf("  -geometry geom    set initial frame geometry\n");
	printf("  -lib dir          use dir for application components\n");
	printf("  -nosplash         do not show splash screen\n");

	printf("\n");
}
