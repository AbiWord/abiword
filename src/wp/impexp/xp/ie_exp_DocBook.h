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


#ifndef IE_EXP_DOCBOOK_H
#define IE_EXP_DOCBOOK_H

#include	"ie_exp.h"
#include	"pl_Listener.h"
#include	"pp_AttrProp.h"
#include	"ut_stack.h"


class PD_Document;
class s_DocBook_Listener;

// The exporter/writer for DocBook

class ABI_EXPORT IE_Exp_DocBook_Sniffer : public IE_ExpSniffer
{
	friend class IE_Exp;

public:
	IE_Exp_DocBook_Sniffer () {}
	virtual ~IE_Exp_DocBook_Sniffer () {}

	virtual bool recognizeSuffix (const char * szSuffix);
	virtual bool getDlgLabels (const char ** szDesc,
							   const char ** szSuffixList,
							   IEFileType * ft);
	virtual UT_Error constructExporter (PD_Document * pDocument,
										IE_Exp ** ppie);
};


class ABI_EXPORT IE_Exp_DocBook : public IE_Exp
{
public:
	IE_Exp_DocBook(PD_Document * pDocument);
	virtual ~IE_Exp_DocBook();
	
		void iwrite (const char *);
		void writeln (const char *);
		int indent (void);
		int unindent (void);

protected:
	virtual UT_Error	_writeDocument(void);
		int s_align;

private:	
	s_DocBook_Listener *	m_pListener;
};


class s_DocBook_Listener : public PL_Listener
{
public:
	s_DocBook_Listener(PD_Document * pDocument,
						IE_Exp_DocBook * pie);
	virtual ~s_DocBook_Listener();

	virtual bool		populate(PL_StruxFmtHandle sfh,
								 const PX_ChangeRecord * pcr);

	virtual bool		populateStrux(PL_StruxDocHandle sdh,
									  const PX_ChangeRecord * pcr,
									  PL_StruxFmtHandle * psfh);

	virtual bool		change(PL_StruxFmtHandle sfh,
							   const PX_ChangeRecord * pcr);

	virtual bool		insertStrux(PL_StruxFmtHandle sfh,
									const PX_ChangeRecord * pcr,
									PL_StruxDocHandle sdh,
									PL_ListenerId lid,
									void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
															PL_ListenerId lid,
															PL_StruxFmtHandle sfhNew));

	virtual bool		signal(UT_uint32 iSignal);
	virtual bool		_initFile(void);
	virtual void		_closeFile(void);
protected:

	void				_closeSection(int);
	void				_closeSectionTitle(int);
	void				_closeParagraph(void);
	void				_closeSpan(void);
	void				_closeList(void);
	void				_closeChapter (void);
	void				_closeChapterTitle (void);
	void				_openParagraph(PT_AttrPropIndex api);
	void				_openList(PT_AttrPropIndex api);
	void				_openSection(PT_AttrPropIndex api, int, bool);
	void				_openSectionTitle(PT_AttrPropIndex api, int, bool);
	void				_openSpan(PT_AttrPropIndex api);
	void				_openChapter (PT_AttrPropIndex api);
	void				_openChapterTitle (PT_AttrPropIndex api);
	void				_outputData(const UT_UCSChar * p, UT_uint32 length);
	void				_handleDataItems(void);
	void				_convertFontSize(char* szDest, const char* pszFontSize);
	void				_convertColor(char* szDest, const char* pszColor);

	PD_Document *		        m_pDocument;
	IE_Exp_DocBook *		m_pie;
private:

	bool				m_bInSection[5];
	bool				m_bInParagraph;
	bool				m_bInSpan;
	bool				m_bInSectionTitle[5];
	bool				m_bInChapter;
	bool				m_bInChapterTitle;
	bool				m_bCanChapterTitle;
	bool				m_bCanSectionTitle[5];
	int					m_iListDepth;
	int					m_iPreviousListDepth;
	const PP_AttrProp*	        m_pAP_Span;

	// Need to look up proper type, and place to stick #defines...

	UT_uint16		m_iBlockType;	// BT_*
    bool                 m_bWasSpace;

	UT_Stack			m_utsListStack;

	UT_Vector		m_utvDataIDs;	// list of data ids for image enumeration
	char *          _stripSuffix(const char* from, char delimiter);
};

#endif /* IE_EXP_DOCBOOK_H */
