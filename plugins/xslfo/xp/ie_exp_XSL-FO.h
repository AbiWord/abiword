/* AbiWord
 * Copyright (C) 2001 AbiSource, Inc.
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

#ifndef IE_EXP_XSL_FO_H
#define IE_EXP_XSL_FO_H

#include "ie_exp.h"
#include "ie_Table.h"
#include "pl_Listener.h"
#include "fl_AutoNum.h"

class PD_Document;
class s_XSL_FO_Listener;

// The exporter/writer for the XML/XSL FO spec

class IE_Exp_XSL_FO_Sniffer : public IE_ExpSniffer
{
	friend class IE_Exp;

public:
	IE_Exp_XSL_FO_Sniffer (const char * name);
	virtual ~IE_Exp_XSL_FO_Sniffer () {}

	virtual bool recognizeSuffix (const char * szSuffix);
	virtual bool getDlgLabels (const char ** szDesc,
							   const char ** szSuffixList,
							   IEFileType * ft);
	virtual UT_Error constructExporter (PD_Document * pDocument,
										IE_Exp ** ppie);
};

class IE_Exp_XSL_FO : public IE_Exp
{
public:
	IE_Exp_XSL_FO(PD_Document * pDocument);
	virtual ~IE_Exp_XSL_FO();

protected:
	virtual UT_Error	_writeDocument();

private:
	s_XSL_FO_Listener *	m_pListener;
	UT_uint32 m_error;
};


// A little class to help export lists
class ListHelper
{
public:
	explicit ListHelper()
		: m_pan(NULL),
		  m_iInc(-1),
		  m_iCount(0),
		  m_iStart(0)
	{
	}

	void addList(fl_AutoNum* pAutoNum)
	{
		UT_return_if_fail(pAutoNum);

		m_pan = pAutoNum;
		m_iStart = m_pan->getStartValue32();

		if(m_pan->getType() < BULLETED_LIST)
			m_iInc = 1;

		populateText(m_pan->getDelim());
	}

	UT_uint32 retrieveID()
	{
		return m_pan->getID();
	}

	UT_UTF8String getNextLabel()
	{
		UT_return_val_if_fail(m_pan,"");

		if(m_iInc > -1)
		{
			UT_uint32 val = m_iStart + (m_iInc * m_iCount);
			m_iCount++;
			return UT_UTF8String_sprintf("%s%d%s", m_sPreText.utf8_str(), val, m_sPostText.utf8_str());
		}
		else
		{
			UT_UTF8String bullet;
			UT_UCS4Char symbol[2] = {'\0', '\0'};

			//TODO: support all lists
			//FIXME: most lists don't seem to work; looks like Bug 3601

			switch(m_pan->getType())
			{
				//taken from fl_AutoNum.cpp

				case BULLETED_LIST:
				{
					symbol[0] = (UT_UCSChar) (unsigned char) 0xb7;
					bullet.appendUCS4(symbol);
					break;
				}

				case DASHED_LIST:
				{
					bullet = "-";
					break;
				}

				case SQUARE_LIST:
				{
					symbol[0] = (UT_UCSChar) (unsigned char) 0x6E;
					bullet.appendUCS4(symbol);
					break;
				}

				case TRIANGLE_LIST:
				{
					symbol[0] = (UT_UCSChar) (unsigned char) 0x73;
					bullet.appendUCS4(symbol);
					break;
				}

				case DIAMOND_LIST:
				{
					symbol[0] = (UT_UCSChar) (unsigned char) 0xA9;
					bullet.appendUCS4(symbol);
					break;
				}

				case STAR_LIST:
				{
					symbol[0] = (UT_UCSChar) (unsigned char) 0x53;
					bullet.appendUCS4(symbol);
					break;
				}

				case IMPLIES_LIST:
				{
					symbol[0] = (UT_UCSChar) (unsigned char) 0xDE;
					bullet.appendUCS4(symbol);
					break;
				}

				case TICK_LIST:
				{
					symbol[0] = (UT_UCSChar) (unsigned char) 0x33;
					bullet.appendUCS4(symbol);
					break;
				}

				case BOX_LIST:
				{
					symbol[0] = (UT_UCSChar) (unsigned char) 0x72;
					bullet.appendUCS4(symbol);
					break;
				}

				case HAND_LIST:
				{
					symbol[0] = (UT_UCSChar) (unsigned char) 0x2B;
					bullet.appendUCS4(symbol);
					break;
				}

				case HEART_LIST:
				{
					symbol[0] = (UT_UCSChar) (unsigned char) 0xAA;
					bullet.appendUCS4(symbol);
					break;
				}

				default:
				{
					UT_DEBUGMSG(("Unhandled list type in XSL-FO exporter: %d\n", m_pan->getType()));
					break;
				}
			}

			return bullet;
		}
	}

protected:

	void populateText(const gchar *lDelim)
	{
		UT_UCS4String text = lDelim;
		bool bPre = true;

		for(UT_uint32 i = 0; i < text.length(); i++)
		{
			if(bPre && (text[i] == '%') && ((i + 1) < text.length()) && (text[i+1] == 'L'))
			{
				bPre = false;
				i++;
			}
			else if(bPre)
			{
				m_sPreText += text[i];
			}
			else
			{
				m_sPostText += text[i];
			}
		}

		m_sPreText.escapeXML();
		m_sPostText.escapeXML();
	}

private:

	const fl_AutoNum* m_pan;
	UT_UTF8String m_sPostText; //text after "%L"
	UT_UTF8String m_sPreText; //text before "%L"
	UT_sint32 m_iInc;
	UT_uint32 m_iCount;
	UT_uint32 m_iStart;
};

/************************************************************/
/************************************************************/

class s_XSL_FO_Listener : public PL_Listener
{
public:

	s_XSL_FO_Listener(PD_Document * pDocument,
					  IE_Exp_XSL_FO * pie);
	virtual ~s_XSL_FO_Listener();

	virtual bool		populate(fl_ContainerLayout* sfh,
								 const PX_ChangeRecord * pcr);

	virtual bool		populateStrux(pf_Frag_Strux* sdh,
									  const PX_ChangeRecord * pcr,
									  fl_ContainerLayout* * psfh);

	virtual bool		change(fl_ContainerLayout* sfh,
							   const PX_ChangeRecord * pcr);

	virtual bool		insertStrux(fl_ContainerLayout* sfh,
									const PX_ChangeRecord * pcr,
									pf_Frag_Strux* sdh,
									PL_ListenerId lid,
									void (* pfnBindHandles)(pf_Frag_Strux* sdhNew,
															PL_ListenerId lid,
															fl_ContainerLayout* sfhNew));

	virtual bool		signal(UT_uint32 iSignal);

protected:

	void                _handlePageSize(PT_AttrPropIndex api);
	void				_handleDataItems(void);
	void				_handleLists(void);
	void				_handleBookmark(PT_AttrPropIndex api);
	void				_handleEmbedded(PT_AttrPropIndex api);
	void				_handleField(const PX_ChangeRecord_Object * pcro, PT_AttrPropIndex api);
	void				_handleFrame(PT_AttrPropIndex api);
	void				_handleHyperlink(PT_AttrPropIndex api);
	void				_handleImage(PT_AttrPropIndex api);
	void				_handleMath(PT_AttrPropIndex api);
	void				_handlePositionedImage(PT_AttrPropIndex api);
	void                _outputData(const UT_UCSChar * data, UT_uint32 length);

	void				_closeSection(void);
	void				_closeBlock(void);
	void				_closeSpan(void);
	void				_closeLink(void);
	void				_openBlock(PT_AttrPropIndex api);
	void				_openSection(PT_AttrPropIndex api);
	void				_openSpan(PT_AttrPropIndex api);
	void				_openListItem(void);
	void				_popListToDepth(UT_sint32 depth);

	void				_openTable(PT_AttrPropIndex api);
	void				_openRow(void);
	void				_openCell(PT_AttrPropIndex api);
	void				_closeTable(void);
	void				_closeRow(void);
	void				_closeCell(void);
	void				_handleTableColumns(void);

	UT_UTF8String		_getCellColors(void);
	UT_UTF8String		_getCellThicknesses(void);
	UT_UTF8String		_getTableColors(void);
	UT_UTF8String		_getTableThicknesses(void);

	void				_tagClose(UT_uint32 tagID, const UT_UTF8String & content, bool newline = true);
	void				_tagOpen(UT_uint32 tagID, const UT_UTF8String & content, bool newline = true);
	void				_tagOpenClose(const UT_UTF8String & content, bool suppress = true, bool newline = true);
	UT_uint32			_tagTop(void);

private:

	PD_Document *		m_pDocument;
	IE_Exp_XSL_FO *	    m_pie;

	bool				m_bFirstWrite;
	bool				m_bInLink;
	bool				m_bInNote;
	bool				m_bInSection;
	bool				m_bInSpan;
	bool				m_bWroteListField;

	UT_sint32			m_iBlockDepth;
	UT_uint32			m_iLastClosed;
	UT_sint32			m_iListBlockDepth;
	UT_uint32			m_iListID;

	ie_Table			mTableHelper;
	UT_Vector			m_utvDataIDs;
	UT_NumberStack		m_utnsTagStack;
	UT_GenericVector<ListHelper *> m_Lists;
};

#endif /* IE_EXP_XSL_FO_H */
