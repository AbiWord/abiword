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

#ifndef XAP_PREFS_H
#define XAP_PREFS_H

#include "ut_types.h"
#include "ut_vector.h"
#include "ut_alphahash.h"
#include "ut_string.h"
#include "ut_xml.h"
#include "xap_App.h"
#include "xap_Prefs_SchemeIds.h"

/* For handling the position of windows on the screen, this sets up preferences */
enum {
	PREF_FLAG_GEOMETRY_POS = 	0x1,		// Position is valid
	PREF_FLAG_GEOMETRY_SIZE = 	0x2		// Size is valid
};

typedef void (*PrefsListener) (
	XAP_App				*pApp,
	XAP_Prefs			*pPrefs,
	UT_AlphaHashTable	*phChanges,
	void				*data
	);

/*****************************************************************
** XAP_PrefsScheme is a complete set of preferences.  it contains
** all the info for a UI styling set.  use XAP_Prefs to switch
** between schemes.
******************************************************************/

class XAP_PrefsScheme
{
public:
	XAP_PrefsScheme(XAP_Prefs *pPrefs, const XML_Char * szSchemeName);
	~XAP_PrefsScheme(void);

	const XML_Char *	getSchemeName(void) const;
	UT_Bool				setSchemeName(const XML_Char * szNewSchemeName);
	// The idea of the tick is that some object can cache a preference
	// value if it makes a performance difference.  It should also save
	// a copy of the tick count and the scheme pointer.  If the scheme
	// pointer and the tick count are the same, the cached preference
	// value is current.  If either is changed, the object can refresh
	// its cached value.  The scheme pointer can be different because
	// the preference scheme has changed.  The tick count bumps up once
	// every time any preference value in the scheme is changed.
	inline UT_uint32			getTickCount() {return m_uTick;}
	
	UT_Bool				setValue(const XML_Char * szKey, const XML_Char * szValue);
	UT_Bool				setValueBool(const XML_Char * szKey, UT_Bool bValue);
	
	// the get*Value*() functions return the answer in the last
	// argument; they return error information as the function
	// return value.
	UT_Bool				getValue(const XML_Char * szKey, const XML_Char ** pszValue) const;
	UT_Bool				getValueBool(const XML_Char * szKey, UT_Bool * pbValue) const;
	UT_Bool				getNthValue(UT_uint32 k, const XML_Char ** pszKey, const XML_Char ** pszValue) const;
	
protected:
	XML_Char *			m_szName;
	UT_AlphaHashTable	m_hash;
	XAP_Prefs *			m_pPrefs;
	UT_uint32			m_uTick;   // ticks up every time setValue() or setValueBool() is called
};

/*****************************************************************/
/*****************************************************************/

class XAP_Prefs
{
public:
	XAP_Prefs(XAP_App * pApp);
	virtual ~XAP_Prefs(void);

	UT_Bool					loadPrefsFile(void);
	UT_Bool					loadSystemDefaultPrefsFile(const char * szSystemDefaultPrefsPathname);
	UT_Bool					savePrefsFile(void);
	
	XAP_PrefsScheme *		getNthScheme(UT_uint32 k) const;
	XAP_PrefsScheme *		getScheme(const XML_Char * szSchemeName) const;
	UT_Bool					addScheme(XAP_PrefsScheme * pNewScheme);
	XAP_PrefsScheme *		getCurrentScheme(UT_Bool bCreate = UT_FALSE);
	UT_Bool					setCurrentScheme(const XML_Char * szSchemeName);

	UT_Bool					getPrefsValue(const XML_Char * szKey, const XML_Char ** pszValue) const;
	UT_Bool					getPrefsValueBool(const XML_Char * szKey, UT_Bool * pbValue) const;

	UT_Bool					getAutoSavePrefs(void) const;
	void					setAutoSavePrefs(UT_Bool bAuto);

	UT_Bool					getUseEnvLocale(void) const;
	void					setUseEnvLocale(UT_Bool bUse);

	UT_uint32				getMaxRecent(void) const;
	void					setMaxRecent(UT_uint32 k);
	UT_uint32				getRecentCount(void) const;
	const char *			getRecent(UT_uint32 k) const;		// one-based
	void					addRecent(const char * szRecent);
	void					removeRecent(UT_uint32 k);			// one-based

	UT_Bool					setGeometry(UT_sint32 posx, UT_sint32 posy, UT_uint32 width, UT_uint32 height, UT_uint32 flags = 0);
	UT_Bool					getGeometry(UT_sint32 *posx, UT_sint32 *posy, UT_uint32 *width, UT_uint32 *height, UT_uint32 *flags = 0);
	
	virtual void				fullInit(void) = 0;
	virtual UT_Bool				loadBuiltinPrefs(void) = 0;
	virtual const XML_Char *	getBuiltinSchemeName(void) const = 0;
	virtual const char *		getPrefsPathname(void) const = 0;

	void					addListener	  ( PrefsListener pFunc, void *data );
	void					removeListener ( PrefsListener pFunc, void *data = 0 );
	void					startBlockChange();
	void					endBlockChange();

	// a only-to-be-used-by XAP_PrefsScheme::setValue
	void					_markPrefChange	( const XML_Char *szKey );
protected:
	void					_pruneRecent(void);

	XAP_App *				m_pApp;
	UT_Bool					m_bAutoSavePrefs; /* save on any changes or only when user asks */
	UT_Bool					m_bUseEnvLocale; /* use POSIX env vars to set locale */

	UT_Vector				m_vecSchemes;		/* vector of XAP_PrefsScheme */
	XAP_PrefsScheme *		m_currentScheme;
	XAP_PrefsScheme *		m_builtinScheme;

	UT_uint32				m_iMaxRecent;
	UT_Vector				m_vecRecent;		/* vector of (char *) */

	UT_Vector				m_vecPrefsListeners;	/* vectory of struct PrefListnersPair */
	UT_AlphaHashTable		m_ahashChanges;
	UT_Bool					m_bInChangeBlock;
	void					_sendPrefsSignal( UT_AlphaHashTable *hash );

	typedef struct {
		UT_uint32				m_width, m_height;	/* Default width and height */
		UT_sint32				m_posx, m_posy;		/* Default position */
		UT_uint32				m_flags;			/* What is valid, what is not */
	} Pref_Geometry;
	Pref_Geometry				m_geom;
	

#ifdef HAVE_LIBXML2
	UT_Bool _sax(const char *path, UT_Bool sys);
#endif

public:						/* these are needed by the XML parser interface */
	void					_startElement(const XML_Char *name, const XML_Char **atts);
	void					_endElement(const XML_Char *name);
	void					_charData(const XML_Char *s, int len);
	void					_startElement_SystemDefaultFile(const XML_Char *name, const XML_Char **atts);

private:
	struct
	{
		UT_Bool				m_parserStatus;
		UT_Bool				m_bFoundAbiPreferences;
		UT_Bool				m_bFoundSelect;
		XML_Char *			m_szSelectedSchemeName;
		UT_Bool				m_bFoundRecent;
		UT_Bool				m_bFoundGeometry;
	} m_parserState;

	typedef struct					/* used to store the listener/data pairs in the vector  */
	{
		PrefsListener	m_pFunc;
		void			*m_pData;
	} tPrefsListenersPair;
};

//////////////////////////////////////////////////////////////////////////////////////
// The following are the default values and limits for various non-scheme-based
// application-independent preferences.

#define XAP_PREF_DEFAULT_AutoSavePrefs		"1"
#define XAP_PREF_DEFAULT_MaxRecent			"4"
#define XAP_PREF_DEFAULT_UseEnvLocale		"1"

#define XAP_PREF_LIMIT_MaxRecent			9

#endif /* XAP_PREFS_H */
