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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef AP_DIALOG_LISTS_H
#define AP_DIALOG_LISTS_H

#include <string>

#include "xap_Frame.h"
#include "xap_Dialog.h"
#include "ap_Dialog_Modeless.h"
#include "xav_View.h"
#include "fl_BlockLayout.h"
#include "fl_AutoNum.h"
#include "fl_AutoLists.h"
#include "pt_Types.h"
#include "xap_Preview.h"

class FV_View;
class fl_BlockLayout;
class AP_Preview_Paragraph;
class AP_Dialog_Lists;


class ABI_EXPORT AP_Lists_preview : public XAP_Preview
{
public:

	AP_Lists_preview(GR_Graphics * gc, AP_Dialog_Lists * pLists );
	virtual ~AP_Lists_preview(void);

	// data twiddlers
	void drawImmediate(const UT_Rect* clip = nullptr) override;
	AP_Dialog_Lists*	getLists(void);
	void				setData(const gchar * pszFont,float fAlign,float fIndent);
	void				setData(const std::string & font, float fAlign,
                                float fIndent)
    {
        setData(font.c_str(), fAlign, fIndent);
    }

protected:

	AP_Dialog_Lists*	m_pLists;
	GR_Font*			m_pFont;
	float				m_fAlign;
	float				m_fIndent;
	UT_sint32			m_iLine_pos[8];
	UT_sint32			m_iLine_height;
	bool				m_bFirst;
};

class ABI_EXPORT AP_Dialog_Lists : public AP_Dialog_Modeless
{
  protected:
    virtual XAP_String_Id getWindowTitleStringId() override;

public:
	AP_Dialog_Lists(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Dialog_Lists(void);

    AP_Lists_preview* getListsPreview() const
    {
        return m_pListsPreview;
    }

	// these are used for the Modal version of the dialog called from the
    // styles dialog.
	typedef enum {
		a_OK,
		a_QUIT,
		a_CLOSE
	} tAnswer;

	AP_Dialog_Lists::tAnswer	getAnswer(void) const;
	void                        setAnswer(AP_Dialog_Lists::tAnswer ans ) {m_Answer = ans;}
	void						StartList(void);
	void						StopList(void);
	void						Apply(void);
	void						fillDialogFromBlock(void);
	void						fillDialogFromVector(UT_GenericVector<const gchar*> * inVec);
	void						PopulateDialogData(void);
	void						fillFakeLabels(void);
	bool						isLastOnLevel(void);
	gchar *					getListStyleString( UT_uint32 iListType);
	UT_uint32					decodeListType(char * listformat);
	UT_sint32					findVecItem(UT_GenericVector<const gchar*> * v, const char * key);
        /// XXX this should be moved out of here.
	static UT_sint32				findVecItem(const PP_PropertyVector & v, const char * key);
	void						fillUncustomizedValues(void);
	UT_uint32					getID(void);
	UT_uint32					getStoredID(void) { return m_iID;}
	fl_AutoNumPtr				getAutoNum(void) const;
	fl_BlockLayout *			getBlock(void) const;
	UT_uint32					getTick(void);
	const UT_Vector *			getOutProps(void) const { return &m_OutProps;}
	void						setTick(UT_uint32 iTick);
	bool						isDirty(void) const {return m_bDirty;}
	void						setDirty(void) {m_bDirty = true;}
	void						clearDirty(void) {m_bDirty = false;}

	AV_View *					getAvView(void);
	void						generateFakeLabels(void);
	UT_UCSChar *				getListLabel(UT_sint32 itemNo);
	virtual void 				event_PreviewAreaExposed();
	virtual void 				_createPreviewFromGC(GR_Graphics * gc, UT_uint32 width, UT_uint32 height);
	void						setModal(void) {m_bIsModal = true;}
	bool						isModal(void) const { return m_bIsModal;}

protected:

	// declare JavaBean-like accessors for private variable needed in the
	// platform code.

#define SET_GATHER(a, u)  inline u get##a(void) const {return m_##a;} \
			  inline void set##a(u p##a) {m_##a = p##a;}

	SET_GATHER(iLocalTick,		UT_uint32);
	SET_GATHER(iStartValue,	    UT_uint32);
	SET_GATHER(newStartValue,	UT_uint32);
	SET_GATHER(fAlign,	  	    float);
	SET_GATHER(fIndent,		    float);
	SET_GATHER(bStartNewList,	bool);
	SET_GATHER(bApplyToCurrent,	bool);
	SET_GATHER(bResumeList,		bool);
	SET_GATHER(bisCustomized,	bool);
	SET_GATHER(isListAtPoint,	bool);
	SET_GATHER(bguiChanged,	    bool);
	SET_GATHER(NewListType,	    FL_ListType);
	SET_GATHER(DocListType,	    FL_ListType);
	SET_GATHER(iLevel,	        UT_uint32);
	SET_GATHER(pView,	        FV_View *);


#undef SET_GATHER

    void copyCharToDelim(const std::string & pszDelim)
    {
        m_pszDelim = pszDelim;
    }
    const std::string & getDelim(void) const
    {
        return m_pszDelim;
    }
    void copyCharToDecimal(const std::string & pszDecimal)
    {
        m_pszDecimal = pszDecimal;
    }
	const std::string & getDecimal( void) const
    {
        return m_pszDecimal;
    }
    void copyCharToFont(const std::string & pszFont)
    {
        m_pszFont = pszFont;
    }
	const std::string & getFont(void)
    {
        return m_pszFont;
    }
    void copyCharToWindowName(const char* pszName);
    const char * getWindowName() const;
	AP_Lists_preview* getListsPreview() { return m_pListsPreview; }
	void              setCurrentFold(UT_sint32 iLevel)
		{ m_iCurrentLevel = iLevel;}
	UT_sint32         getCurrentFold(void)
		{ return m_iCurrentLevel;}
	virtual void      setFoldLevelInGUI(void) = 0;
	virtual bool      isPageLists(void) = 0;
	void              setFoldingLevelChanged(bool b)
	  { m_bFoldingLevelChanged = b;}
private:


	// These are the "current use" dialog data items,
	// which are liberally read and set by the
	// accessor methods above.
	FV_View*				m_pView;
	// is this used in a modeless dialog like this?
	//
	// These will all be rationalized after windows get the
	// new dialog

	tAnswer					m_Answer;
	bool					m_isListAtPoint;
	bool					m_previousListExistsAtPoint;
	UT_UCSChar				m_curListLabel[100];
	UT_UCSChar				m_newListLabel[100];
	FL_ListType				m_NewListType;
    std::string				m_pszDelim;
    std::string				m_pszDecimal;
    std::string				m_pszFont;
	float					m_fAlign;
	float					m_fIndent;
	UT_uint32				m_iLevel;
	UT_uint32				m_iStartValue;

	UT_uint32				m_iWidth;
	UT_uint32				m_iHeight;
	UT_uint32				m_iLocalTick;
	UT_uint32				m_curStartValue;
	UT_uint32				m_newStartValue;
	UT_uint32				m_curListLevel;
	UT_uint32				m_newListLevel;
	UT_uint32				m_iID;
	FL_ListType				m_DocListType;

	bool					m_bStartList;

	bool					m_bStartNewList;
	bool					m_bApplyToCurrent;
	bool					m_bResumeList;
	bool					m_bisCustomized;
	bool					m_bguiChanged;

	AP_Preview_Paragraph*	m_paragraphPreview;
	AP_Lists_preview*		m_pListsPreview;

	fl_Layout*				m_pFakeLayout[4];
	pf_Frag_Strux*		m_pFakeSdh[4];
	fl_AutoNumPtr				m_pFakeAuto;
	PD_Document *			m_pFakeDoc;
	bool					m_bDirty;
	bool					m_bIsModal;
	UT_sint32               m_iCurrentLevel;
	UT_Vector				m_OutProps;
	UT_String				m_Output[5];
	bool                                    m_bFoldingLevelChanged;
};


#endif /* AP_DIALOG_LISTS_H */
