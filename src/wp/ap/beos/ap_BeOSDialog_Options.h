/* AbiWord
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

#ifndef AP_BEOSDIALOG_OPTIONS_H
#define AP_BEOSDIALOG_OPTIONS_H

#include "ap_Dialog_Options.h"

class XAP_BeOSFrame;
class UT_String;

/*****************************************************************/
class AP_BeOSDialog_Options: public AP_Dialog_Options
{
	friend class OptionsWin;
	
public:
	AP_BeOSDialog_Options(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_BeOSDialog_Options(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

 protected:

	//GtkWidget *_lookupWidget( tControl id );
	virtual void _controlEnable( tControl id, bool value );

	// we implement these so the XP dialog can set/grab our data
#define SET_GATHER(a,t) virtual t _gather##a(void);  \
 					    virtual void    _set##a( t )
 
 	SET_GATHER			(SpellCheckAsType,	bool );
 	SET_GATHER			(SpellHideErrors,	bool );
 	SET_GATHER			(SpellSuggest,		bool );
 	SET_GATHER			(SpellMainOnly,		bool );
 	SET_GATHER			(SpellUppercase,	bool );
 	SET_GATHER			(SpellNumbers,		bool );
 	SET_GATHER			(SpellInternet,		bool );
 
	SET_GATHER			(SmartQuotesEnable, bool );
 	SET_GATHER			(DefaultPageSize,	fp_PageSize::Predefined );

 	SET_GATHER			(PrefsAutoSave,		bool );
 
 	SET_GATHER			(ViewShowRuler,		bool );
	//TF: Deprecated with specific toolbar toggling
//	SET_GATHER			(ViewShowToolbars,	bool );
	SET_GATHER			(ViewShowStandardBar,	bool );
 	SET_GATHER			(ViewShowFormatBar,	bool );
 	SET_GATHER			(ViewShowExtraBar,	bool );

 	SET_GATHER			(ViewShowStatusBar,	bool );
 
	SET_GATHER			(ViewRulerUnits,	UT_Dimension);		
	SET_GATHER			(ViewCursorBlink,	bool);
 
 	SET_GATHER			(ViewAll,			bool );
 	SET_GATHER			(ViewHiddenText,	bool );
 	SET_GATHER			(ViewUnprintable,	bool );
    SET_GATHER          (AllowCustomToolbars, bool);
    SET_GATHER          (AutoLoadPlugins, bool);
  
	SET_GATHER			(AutoSaveFile, bool);
	SET_GATHER			(ShowSplash, bool);
	virtual void _gatherAutoSaveFilePeriod(UT_String &stRetVal);
	virtual void _setAutoSaveFilePeriod(const UT_String &stPeriod);
	virtual void _gatherAutoSaveFileExt(UT_String &stRetVal);
	virtual void _setAutoSaveFileExt(const UT_String &stExt);

 	SET_GATHER			(NotebookPageNum,	int );

#undef SET_GATHER
	
 protected:
	class OptionsWin  *newwin;

	// FIXME: replace this with *real* gui code */
	fp_PageSize::Predefined defaultPageSize;
};

#endif /* AP_BEOSDIALOG_OPTIONS_H */
