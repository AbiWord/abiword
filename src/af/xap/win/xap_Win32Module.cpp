/* AbiSource Application Framework
 * Copyright (C) 2001 Mike Nordell
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

#define WIN32_LEAN_AND_MEAN
#define NOUSER
#define NOGDI
#define NOSERVICE
#include <windows.h>

#include "xap_Win32Module.h"
#include "ut_types.h"
#include "ut_string.h"
#include "ut_assert.h"


static const char szErrBadParam[]		= "Bad parameter";
static const char szErrNoDllFound[]		= "Could not load library";
static const char szErrNoDllLoaded[]	= "No library loaded";
static const char szErrAlreadyLoaded[]	= "Library already loaded";
static const char szErrCouldNotUnload[]	= "Could not unload library";

class ABI_EXPORT XAP_Win32ModuleImpl
{
public:
	XAP_Win32ModuleImpl() : m_hMod(0), m_pszErr(0), m_pszModuleName(0) { }
	~XAP_Win32ModuleImpl()
	{
	}

	bool load(const char* name)
	{
		if (m_hMod)
		{
			m_pszErr = szErrAlreadyLoaded;
			return false;
		}

		WCHAR szName[512];

		MultiByteToWideChar(CP_UTF8,0,name,-1,szName,512);

		m_hMod = LoadLibraryW(szName);
		if (!m_hMod)
		{
			m_pszErr = szErrNoDllFound;
			return false;
		}
		m_pszErr = 0;
		m_pszModuleName = new char[strlen(name) + 1];
		if (m_pszModuleName)
		{
			strcpy(m_pszModuleName, name);
		}
		return true;
	}

	bool unload()
	{
		if (m_hMod)
		{
			if (FreeLibrary(m_hMod))
			{
				m_hMod = 0;
				delete [] m_pszModuleName;
				m_pszModuleName = 0;
				m_pszErr = 0;
				return true;
			}
			m_pszErr = szErrCouldNotUnload;
			return false;
		}
		m_pszErr = szErrNoDllLoaded;
		return false;
	}


	bool resolveSymbol(const char* symbol_name, void** symbol)
	{
		if (!symbol_name || !*symbol_name || !symbol ) // TODO Fix this so it works || IsBadWritePtr(*symbol, sizeof(*symbol)))
		{
			m_pszErr = szErrBadParam;
			return false;
		}

		FARPROC pProc = GetProcAddress(m_hMod, symbol_name);
		if (pProc)
		{
			#ifdef __MINGW32__
			*symbol = (void*)(pProc);
			#else
			*symbol = reinterpret_cast<void*>(pProc);
			#endif
			m_pszErr = 0;
			return true;
		}
		return false;
	}

	bool getModuleName(char** dest) const
	{
		if (!m_hMod)
		{
			m_pszErr = szErrNoDllLoaded;
			return false;
		}
		if (!dest) // TODO Fix this so it works || IsBadWritePtr(*dest, sizeof(*dest)))
		{
			m_pszErr = szErrBadParam;
			return false;
		}
		if (m_pszModuleName)
		{
			*dest = g_strdup(m_pszModuleName);
			m_pszErr = 0;
			return true;
		}
		return false;
	}

	bool getErrorMsg(char** dest) const
	{
		if (!dest) // TODO Fix this so it works || IsBadWritePtr(*dest, sizeof(*dest)))
		{
			m_pszErr = szErrBadParam;
			return false;
		}
		if (m_pszErr)
		{
			*dest = g_strdup(m_pszErr);
			return true;
		}
		return false;
	}

	HMODULE				m_hMod;
	mutable const char* m_pszErr;
	mutable char*		m_pszModuleName;
};

XAP_Win32Module::XAP_Win32Module()
:	pimpl(new XAP_Win32ModuleImpl)
{
	UT_ASSERT_HARMLESS(pimpl);
}

XAP_Win32Module::~XAP_Win32Module()
{
	delete pimpl;
}

bool XAP_Win32Module::load(const char* name)
{
	return pimpl->load(name);
}

bool XAP_Win32Module::unload()
{
	return pimpl->unload();
}

bool XAP_Win32Module::resolveSymbol(const char* symbol_name, void** symbol)
{
	return pimpl->resolveSymbol(symbol_name, symbol);
}

bool XAP_Win32Module::getModuleName(char** dest) const
{
	return pimpl->getModuleName(dest);
}

bool XAP_Win32Module::getErrorMsg(char** dest) const
{
	return pimpl->getErrorMsg(dest);
}

