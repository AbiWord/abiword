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

#ifndef UT_SCRIPT_H
#define UT_SCRIPT_H

#include <string>

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif

typedef UT_sint32 UT_ScriptIdType;

#ifndef UTVECTOR_H
#include "ut_vector.h"
#endif

class UT_Script;
class UT_ScriptLibrary;

class ABI_EXPORT UT_ScriptSniffer
{
  	friend class UT_ScriptLibrary;

public:
	virtual ~UT_ScriptSniffer();

	// these you get for g_free
	inline bool supportsType (UT_ScriptIdType type) const { return m_type == type; }
	inline UT_ScriptIdType getType() const { return m_type; }

	// these you must override these
	virtual bool recognizeContents (const char * szBuf,
									UT_uint32 iNumbytes) const = 0;
	virtual bool recognizeSuffix (const char * szSuffix) const = 0;
	virtual bool getDlgLabels (const char ** szDesc,
							   const char ** szSuffixList,
							   UT_ScriptIdType * ft) const = 0;
	virtual UT_Error constructScript (UT_Script ** ppscript) const = 0;

protected:
       	UT_ScriptSniffer();

private:
	// only UT_ScriptLibrary ever calls this
	inline void setType (UT_ScriptIdType type) { m_type = type; }
	UT_ScriptIdType m_type;
};

class ABI_EXPORT UT_Script
{
public:
	virtual UT_Error execute(const char * scriptName) = 0;
	virtual const std::string& errmsg() const = 0;
	virtual ~UT_Script();

protected:
	UT_Script();

private:
	UT_Script(const UT_Script &); // no impl
	UT_Script& operator=(const UT_Script &); // no impl
};

class ABI_EXPORT UT_ScriptLibrary
{
public:
	UT_ScriptLibrary();
	virtual ~UT_ScriptLibrary();

	static UT_ScriptLibrary * instance();

	bool	    enumerateDlgLabels(UT_uint32 ndx,
								   const char ** pszDesc,
								   const char ** pszSuffixList,
								   UT_ScriptIdType * ft);

	UT_Error execute(const char * script, UT_ScriptIdType type = -1);

	UT_uint32 getNumScripts() const;
	void registerScript(UT_ScriptSniffer*);
	void unregisterScript(UT_ScriptSniffer*);
	void unregisterAllScripts();

	const std::string& errmsg() const { return m_stErrMsg; }

private:
	void errmsg(const std::string& st) { m_stErrMsg = st; }

	UT_ScriptIdType	typeForContents(const char * szBuf,
									UT_uint32 iNumbytes);

	UT_ScriptIdType	typeForSuffix(const char * szSuffix);
	const char *          suffixesForType(UT_ScriptIdType ieft);

	UT_Error	constructScript(const char * szFilename,
								UT_ScriptIdType ieft,
								UT_Script ** ppscript,
								UT_ScriptIdType * pieft = NULL);

	UT_ScriptLibrary(const UT_ScriptLibrary&);
	UT_ScriptLibrary& operator=(const UT_ScriptLibrary&);
	static UT_ScriptLibrary * m_pInstance;
	friend void __dummy_method_dont_use(void);

	UT_GenericVector<UT_ScriptSniffer *>* mSniffers;
	std::string m_stErrMsg;
};

#endif /* UT_SCRIPT_H */
