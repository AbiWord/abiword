/* AbiWord
 * Copyright (C) 2002 Patrick Lam
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

#ifndef AP_ARGS_H
#define AP_ARGS_H

#include "ut_types.h"
#include "xap_Args.h"
#include "popt.h"

class AP_Args;
class XAP_App;
class AP_App;

// warning: options array is static.
class AP_Args
{
public:
	AP_Args(XAP_Args * pArgs, const char * szAppName, AP_App * pApp);
	~AP_Args(void);

	XAP_Args *XArgs;
	poptContext poptcon;

	/* Parse options. */
	void parsePoptOpts ();

	bool getShowSplash() const { return m_bShowSplash; }
	AP_App* getApp() const { return m_pApp; }
	bool doWindowlessArgs();

	// Would be nice if this could be non-static.
	const static struct poptOption const_opts[];
	static struct poptOption * options;

#ifdef ABI_OPT_PERL
 	static const char * m_sScript;
#endif
#ifdef DEBUG
	static int    m_iDumpstrings;
#endif
	static const char * m_sGeometry;
	static const char * m_sTo;
	static int    m_iToPNG;
	static const char * m_sPrintTo;
	static int	  m_iVerbose;
	static int	  m_iShow;
	static const char * m_sPlugin;
	static int    m_iNosplash;
	static const char * m_sFile;
	static int    m_iVersion;
	static int    m_iHelp;
	static const char * m_sDisplay;
	static int    m_iAbiControl;
	static const char * m_sMerge;
private:
	bool          m_bShowSplash;
	AP_App*       m_pApp;
};

#endif /* AP_ARGS_H */
