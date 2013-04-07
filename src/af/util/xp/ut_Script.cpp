/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
/* AbiWord
 * Copyright (C) 2001 Dom Lachowicz
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

#include <stdio.h>

#include "ut_Script.h"
#include "ut_string.h"
#include "ut_vector.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_misc.h"

/************************************************************************/
/************************************************************************/

UT_ScriptSniffer::UT_ScriptSniffer()
	: m_type (-1)
{
}

UT_ScriptSniffer::~UT_ScriptSniffer()
{
}

/************************************************************************/
/************************************************************************/

UT_Script::UT_Script ()
{
}

UT_Script::~UT_Script()
{
}

/************************************************************************/
/************************************************************************/
UT_ScriptLibrary * UT_ScriptLibrary::m_pInstance = NULL;

UT_ScriptLibrary::UT_ScriptLibrary ()
  :     mSniffers (new UT_GenericVector<UT_ScriptSniffer *>(5)),
    m_stErrMsg("")
{
  m_pInstance = this;
  UT_DEBUGMSG(("Construct a scriptlibrary %p \n",this));
}

UT_ScriptLibrary::~UT_ScriptLibrary ()
{
  UT_DEBUGMSG(("Delete the scriptlibrary %p \n",this));
	DELETEP(mSniffers);
}

UT_ScriptLibrary * UT_ScriptLibrary::instance ()
{
	return m_pInstance;
}

UT_Error UT_ScriptLibrary::execute (const char * script,
									UT_ScriptIdType type )
{
	UT_Script* pScript = NULL;
	UT_ScriptIdType scriptId = -1;

	UT_Error err = UT_OK;

	if ((err = constructScript(script, type, &pScript, &scriptId)) == UT_OK)
    {
		if ((err = pScript->execute(script)) != UT_OK)
		{
			UT_DEBUGMSG(("Error executing script: %d\n", err));
			errmsg(pScript->errmsg());
		}

		DELETEP(pScript);
    }

	return err;
}

UT_uint32 UT_ScriptLibrary::getNumScripts () const
{
	return mSniffers->size ();
}

void UT_ScriptLibrary::registerScript ( UT_ScriptSniffer * s )
{
	UT_sint32 ndx = 0;
	UT_Error err = mSniffers->addItem (s, &ndx);

	UT_return_if_fail(err == UT_OK);
	s->setType(ndx+1);
}

void UT_ScriptLibrary::unregisterScript ( UT_ScriptSniffer * s )
{
	UT_uint32 ndx = s->getType(); // 1:1 mapping
  
	UT_return_if_fail( ndx > 0);
	
	mSniffers->deleteNthItem (ndx-1);
  
	// Refactor the indexes
	UT_ScriptSniffer * pSniffer = 0;
	UT_sint32 size  = mSniffers->size();
	UT_sint32 i     = 0;
	for( i = ndx-1; i < size; i++)
    {
		pSniffer = mSniffers->getNthItem(i);
		if (pSniffer)
			pSniffer->setType(i+1);
    }
}

void UT_ScriptLibrary::unregisterAllScripts ()
{
	UT_ScriptSniffer * pSniffer = 0;
	UT_sint32 size = mSniffers->size();
  
	for (UT_sint32 i = 0; i < size; i++)
	{
		pSniffer = mSniffers->getNthItem(i);
		if (pSniffer)
			delete pSniffer;
    }

	mSniffers->clear();
}

UT_ScriptIdType	UT_ScriptLibrary::typeForContents(const char * szBuf,
												  UT_uint32 iNumbytes)
{
	// we have to construct the loop this way because a
	// given filter could support more than one file type,
	// so we must query a match for all file types
	UT_uint32 nrElements = getNumScripts();
  
	for (UT_uint32 k=0; k < nrElements; k++)
    {
		const UT_ScriptSniffer * s = mSniffers->getNthItem (k);
		if (s->recognizeContents(szBuf, iNumbytes))
		{
			for (UT_sint32 a = 0; a < static_cast<int>(nrElements); a++)
			{
				if (s->supportsType(static_cast<UT_ScriptIdType>(a+1)))
					return static_cast<UT_ScriptIdType>(a+1);
			}
	  
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			// Hm... an importer recognizes the given data
			// but refuses to support any file type we request.
			return -1;
		}
    }
  
	// No filter recognizes this data
	return -1;
	
}
	
UT_ScriptIdType	UT_ScriptLibrary::typeForSuffix(const char * szSuffix)
{
	if (!szSuffix || !(*szSuffix))
		return -1;
	
	// we have to construct the loop this way because a
	// given filter could support more than one file type,
	// so we must query a suffix match for all file types
	UT_uint32 nrElements = getNumScripts();
  
	for (UT_uint32 k=0; k < nrElements; k++)
    {
		const UT_ScriptSniffer * s = mSniffers->getNthItem(k);
		if (s->recognizeSuffix(szSuffix))
		{
			for (UT_sint32 a = 0; a < static_cast<int>(nrElements); a++)
			{
				if (s->supportsType(static_cast<UT_ScriptIdType>(a+1)))
					return static_cast<UT_ScriptIdType>(a+1);
			}
	  
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			// Hm... an importer has registered for the given suffix,
			// but refuses to support any file type we request.
			return -1;
		}
    }
  
	// No filter is registered for that extension
	return -1;  
}

const char * UT_ScriptLibrary::suffixesForType(UT_ScriptIdType ieft)
{
	const char * szSuffixes = 0;
  
	// we have to construct the loop this way because a
	// given filter could support more than one file type,
	// so we must query a suffix match for all file types
	UT_uint32 nrElements = getNumScripts();
  
	for (UT_uint32 k=0; k < nrElements; k++)
    {
		const UT_ScriptSniffer * s = mSniffers->getNthItem(k);
		if (s->supportsType(ieft))
		{
			const char *szDummy;
			UT_ScriptIdType ieftDummy;
			if (s->getDlgLabels(&szDummy,&szSuffixes,&ieftDummy))
			{
				return szSuffixes;
			}
			else
			{
				UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			}
		}
    }
  
	// The passed in filetype is invalid.
	return 0;
}
	
UT_Error UT_ScriptLibrary::constructScript(const char * szFilename,
										   UT_ScriptIdType ieft,
										   UT_Script ** ppscript, 
										   UT_ScriptIdType * pieft)
{
	UT_return_val_if_fail(((ieft != -1) || (szFilename && *szFilename)) &&
						  ppscript, UT_ERROR);
	
	// no filter will support -1, so we try to detect
	// from the contents of the file or the filename suffix
	// the importer to use and assign that back to ieft.
	// Give precedence to the file contents
	if (ieft == -1 && szFilename && *szFilename)
    {
		char szBuf[4096];  // 4096 ought to be enough
		int iNumbytes;
		FILE *f;
		// we must open in binary mode for UCS-2 compatibility
		if ( ( f = fopen( szFilename, "rb" ) ) != static_cast<FILE *>(0) )
		{
			iNumbytes = fread(szBuf, 1, sizeof(szBuf), f);
			fclose(f);
			ieft = typeForContents(szBuf, iNumbytes);
		}
    }
	if (ieft == -1 && szFilename && *szFilename)
    {
		ieft = typeForSuffix(UT_pathSuffix(szFilename).c_str());
    }
  
	UT_return_val_if_fail(ieft != -1, UT_ERROR);
	
	// tell the caller the type of importer they got
	if (pieft != NULL) 
		*pieft = ieft;
  
	// use the importer for the specified file type
	UT_uint32 nrElements = getNumScripts();
  
	for (UT_uint32 k=0; k < nrElements; k++)
    {
		const UT_ScriptSniffer * s = mSniffers->getNthItem (k);
		if (s->supportsType(ieft))
			return s->constructScript(ppscript);
    }

	// all has failed
	return UT_ERROR;
}

bool UT_ScriptLibrary::enumerateDlgLabels(UT_uint32 ndx,
										  const char ** pszDesc,
										  const char ** pszSuffixList,
										  UT_ScriptIdType * ft)
{
	UT_uint32 nrElements = getNumScripts();
	if (ndx < nrElements)
	{
		const UT_ScriptSniffer * s = mSniffers->getNthItem (ndx);
		return s->getDlgLabels(pszDesc,pszSuffixList,ft);
	}

	return false;
}

