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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ut_assert.h"
#include "ut_string.h"
#include "ut_debugmsg.h"
#include "ap_Dialog_Lists.h"
#include "ap_Strings.h"
#include "ut_string.h"
#include "ut_string_class.h"

#include "xap_Dialog_Id.h"
#include "xap_DialogFactory.h"
#include "xap_Dlg_MessageBox.h"

#include "fl_DocLayout.h"
#include "fv_View.h"
#include "pd_Document.h"
#include "fl_DocLayout.h"
#include "fl_BlockLayout.h"
#include "ap_Preview_Paragraph.h"
#include "xad_Document.h"
#include <fribidi.h>

AP_Dialog_Lists::AP_Dialog_Lists(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
:	XAP_Dialog_Modeless(pDlgFactory, id, "interface/dialoglists"),
	m_pView(0),
	m_Answer(a_CLOSE),
	m_isListAtPoint(false),
	m_previousListExistsAtPoint(false),
	m_NewListType(NOT_A_LIST),
	m_fAlign(0),
	m_fIndent(0),
	m_iLevel(0),
	m_iStartValue(0),
	m_iWidth(0),
	m_iHeight(0),
	m_iLocalTick(0),
	m_curStartValue(0),
	m_newStartValue(0),
	m_curListLevel(0),
	m_newListLevel(0),
	m_iID(0),
	m_DocListType(NOT_A_LIST),
	m_bStartList(false),
	m_bStartNewList(0),
	m_bApplyToCurrent(0),
	m_bResumeList(0),
	m_bisCustomized(0),
	m_bguiChanged(false),
	m_paragraphPreview(0),
	m_pListsPreview(0),
	m_pFakeAuto(0),
	m_pFakeDoc(0),
	m_bDirty(false),
    m_bIsModal(false)
{
	for(UT_uint32 i=0; i<4; i++)
	{
		m_pFakeLayout[i] = NULL;
		m_pFakeSdh[i] = NULL;
	}

	m_WindowName[0] = '\0';
	m_curListLabel[0] = '\0';
	m_newListLabel[0] = '\0';
	m_pszDelim[0] = '\0';
	m_pszDecimal[0] = '\0';
	m_pszFont[0] = '\0';

}

AP_Dialog_Lists::~AP_Dialog_Lists(void)
{
 	DELETEP(m_pListsPreview);
	for(UT_uint32 i=0; i<4; i++)
	{
		DELETEP(m_pFakeLayout[i]);
	}
	// What do we do about the fakeAutoNum in the Document pDoc?
	// Maybe we need another constrcutor

	DELETEP(m_pFakeAuto);

	UNREFP(m_pFakeDoc);
}

AP_Dialog_Lists::tAnswer AP_Dialog_Lists::getAnswer(void) const
{
	// let our caller know if user hit ok, cancel, etc.
	return m_Answer;
}

/************************************************************************/

/*!
 * Create the preview from the Graphics Context provided by the platform code.
 \params gc the Platform Graphics Context cast into the a XP handle.
 \params width the width of the gc
 \params height the height of the gc
 */
void AP_Dialog_Lists::_createPreviewFromGC(GR_Graphics* gc,
										   UT_uint32 width,
										   UT_uint32 height)
{
	UT_ASSERT(gc);

	m_iWidth = width;
	m_iHeight = height;

// free any attached preview

	DELETEP(m_pListsPreview);

	m_pListsPreview = new AP_Lists_preview(gc, this);
	UT_ASSERT(m_pListsPreview);

	m_pListsPreview->setWindowSize(width, height);
	//
	// Generate the fake layout pointers and autonum we need for the
	// preview
	//
	generateFakeLabels();
	m_isListAtPoint = getBlock()->isListItem();
	if(m_isListAtPoint == false)
	{
		m_NewListType = NOT_A_LIST;
	}
}

/*!
 * Actually draw the preview.
 */
void AP_Dialog_Lists::event_PreviewAreaExposed(void)
{
	if (m_pListsPreview)
	{
		fillFakeLabels();
		m_pListsPreview->draw();
	}
	else
	{
		UT_ASSERT(0);
	}
}

void AP_Dialog_Lists::StartList(void)
{
	UT_ASSERT(!IS_NONE_LIST_TYPE(m_DocListType));
	getBlock()->listUpdate();
	const XML_Char* pStyle = getBlock()->getListStyleString(m_DocListType);
	UT_ASSERT(pStyle);
	getView()->cmdStartList(pStyle);
}


void AP_Dialog_Lists::StopList(void)
{
	getBlock()->listUpdate();
	getView()->cmdStopList();
}

fl_AutoNum * AP_Dialog_Lists::getAutoNum(void) const
{
	return getBlock()->getAutoNum();
}

/*!
 * Returns the block at the current point.
 */
fl_BlockLayout * AP_Dialog_Lists::getBlock(void) const
{
	return getView()->getCurrentBlock();
}

/*!
 * This is the local cache of the change number reported in the AV_View. Only
 * do an auto update if this number is different from the AV_View.
 */
UT_uint32 AP_Dialog_Lists::getTick(void)
{
	return m_iLocalTick;
}

/*!
 * This is the local cache of the change number reported in the AV_View. Only
 * do an auto update if this number is different from the AV_View.
 */
void AP_Dialog_Lists::setTick(UT_uint32 iTick)
{
	m_iLocalTick = iTick;
}

/*!
 * This method Does the stuff requested on the "action" button, "Apply" in the
 * Modeless dialog and "OK" in the Modal dialog.
 * Read comments with for all the stuff that can happen.
 */
void AP_Dialog_Lists::Apply(void)
{
	XML_Char szStart[20];
/*!
 *
 * OK this is failsafe code incase the user has changed the font but wants a
 * bullet list anyway. We don't let then!
 */
	if(m_NewListType == BULLETED_LIST || m_NewListType == IMPLIES_LIST)
	{
		UT_XML_strncpy( (XML_Char *) m_pszFont, 80, (const XML_Char *) "Symbol");
	}
	else if(m_NewListType > DASHED_LIST && m_NewListType < OTHER_NUMBERED_LISTS)
	{
		UT_XML_strncpy( (XML_Char *) m_pszFont, 80, _getDingbatsFontName());
	}

/*!
 * Just to make things even more confusing this method is also used in a Modal
 * mannor by the styles dialog. This method is called when the users clicks "OK"
 * on the modal dialog. When that happens we fill an output vector with all the
 * properties currently defined.
 */
	if(isModal())
	{
//
// Fill out output vector with XML_Char * strings to be accessed via the calling
// function.
//
		if(m_OutProps.getItemCount() > 0)
			m_OutProps.clear();
		sprintf(szStart,"%d",m_iStartValue);
		m_OutProps.addItem((void *) "start-value");
		m_Output[0] = (XML_Char *) szStart;
		m_OutProps.addItem((void *) m_Output[0].c_str());
		m_OutProps.addItem((void *) "list-style");
		m_Output[1] = getBlock()->getListStyleString(m_NewListType);
		m_OutProps.addItem((void *) m_Output[1].c_str());
		m_OutProps.addItem((void *) "list-delim");
		m_OutProps.addItem((void *)  m_pszDelim);
		m_OutProps.addItem((void *) "list-decimal");
		m_OutProps.addItem((void *) m_pszDecimal);
		m_OutProps.addItem((void *) "field-font");
		m_OutProps.addItem((void *) m_pszFont);
		m_OutProps.addItem((void *) "margin-left");
		m_Output[2] =	UT_convertInchesToDimensionString(DIM_IN, m_fAlign, 0);
		m_OutProps.addItem((void *) m_Output[2].c_str());

		m_OutProps.addItem((void *) "text-indent");
		m_Output[3] = UT_convertInchesToDimensionString(DIM_IN, m_fIndent, 0);
		m_OutProps.addItem((void *) m_Output[3].c_str());
		return;
	}
/*!
 * If the "Apply to current" radio buton is chosen we have two options.
 * 1. If "No list" is chosen we stop the current list at on this block.
 * 2. Otherwise we change the current list to the type requested here.
 * This piece of code changes the list style at the current point to the
 * Style requested by the user.
 */
	UT_Vector vBlock;
	UT_uint32 i = 0;
	getView()->getBlocksInSelection(&vBlock);
	UT_uint32 count = vBlock.getItemCount();
	getView()->cmdUnselectSelection();
	if(m_bApplyToCurrent == true && m_isListAtPoint == true &&  m_NewListType != NOT_A_LIST)
	{
		getView()->changeListStyle(getAutoNum(),m_NewListType,m_iStartValue,(XML_Char *) m_pszDelim,(XML_Char *) m_pszDecimal, m_pszFont,m_fAlign,m_fIndent);
		if(getAutoNum() != NULL)
		{
			getAutoNum()->update(0);
		}
		clearDirty();
		return;
	}
/*!
 * This code stops the list at the current point.
 */
	if ( m_isListAtPoint == true &&  m_NewListType == NOT_A_LIST)
	{
		for(i=0;i < count; i++)
		{
			fl_BlockLayout * pBlock = (fl_BlockLayout *) vBlock.getNthItem(i);
			if(pBlock->isListItem() == true)
			{
				getView()->getDocument()->StopList(pBlock->getStruxDocHandle());
			}
		}
		clearDirty();
		return;
	}
/*!
 * Start new list. 4 Possibilities.
 * 1. If there is a list at the current point and the user choose no list, stop
 *    the list the current point.
 *
 * 2. start a new list with the properties given if there is not a
 * list at the current point.
 *
 * 3. Start a sublist at the current point if a list already exists there and
 *    contains two or more items.
 *
 * 4. Change the list to the requested value if a list already eists but only
 *    has one item in it.
 */
	if(m_bStartNewList == true)
	{
		for(i=0;i < count; i++)
		{
			fl_BlockLayout * pBlock = (fl_BlockLayout *) vBlock.getNthItem(i);
			if(pBlock->isListItem() == true && m_NewListType == NOT_A_LIST)
			{
//
// This stops the current list.
//
				if(pBlock->isListItem() == true)
				{
					getView()->getDocument()->StopList(pBlock->getStruxDocHandle());
				}
			}
			else if ( pBlock->isListItem() == false && m_NewListType != NOT_A_LIST )
			{
//
// This starts the new list
//
				pBlock->getDocument()->disableListUpdates();
				if(i == 0)
				{
					pBlock->StartList(m_NewListType,m_iStartValue,m_pszDelim,m_pszDecimal,m_pszFont,m_fAlign,m_fIndent, 0,1);
					pBlock->getDocument()->enableListUpdates();
					pBlock->getDocument()->updateDirtyLists();
				}
				else
				{
					fl_BlockLayout * pBlock = (fl_BlockLayout *) vBlock.getNthItem(i);
					fl_BlockLayout * rBlock = (fl_BlockLayout *) pBlock->getPrev();
					if(rBlock != NULL)
					{
						pBlock->resumeList(rBlock);
						pBlock->getDocument()->enableListUpdates();
					}
				}

			}
			else if( pBlock->getAutoNum() && (pBlock->getAutoNum()->getNumLabels() > 1) && m_NewListType != NOT_A_LIST )
			{
//
// This starts a sublist.
//
				UT_uint32 curlevel = pBlock->getLevel();
				UT_uint32 currID = pBlock->getAutoNum()->getID();
				curlevel++;
				pBlock->getDocument()->disableListUpdates();
//
// Need to update m_fAlign and m_fIndent to reflect the higher level of indentation.
//
				if(i == 0)
				{
					m_fAlign = m_fAlign + (float) LIST_DEFAULT_INDENT;
					pBlock->StartList(m_NewListType,m_iStartValue,m_pszDelim,m_pszDecimal,m_pszFont,m_fAlign,m_fIndent, currID,curlevel);
					pBlock->getDocument()->enableListUpdates();
					pBlock->getDocument()->updateDirtyLists();
				}
				else
				{
					fl_BlockLayout * pBlock = (fl_BlockLayout *) vBlock.getNthItem(i);
					fl_BlockLayout * rBlock = (fl_BlockLayout *) pBlock->getPrev();
					if(rBlock != NULL)
					{
						pBlock->resumeList(rBlock);
						pBlock->getDocument()->enableListUpdates();
					}
				}
			}
			else if( pBlock->getAutoNum() && (pBlock->getAutoNum()->getNumLabels() <= 1) && m_NewListType != NOT_A_LIST )
			{
//
// The list at the current point only has one item which is the current paragraph.
// We can't share an sdh amongst two autonum's so we can't start a sublist.
// We'll change the list style instead.
//
				getView()->changeListStyle(pBlock->getAutoNum(),m_NewListType,m_iStartValue,(XML_Char *) m_pszDelim,(XML_Char *) m_pszDecimal, m_pszFont,m_fAlign,m_fIndent);
				if(pBlock->getAutoNum() != NULL)
				{
					pBlock->getAutoNum()->update(0);
				}

			}
		}
		clearDirty();
		return;
	}
/*!
 * OK Attach the block at this point to the previous list of the same margin.
 */
	if(m_bResumeList == true &&  m_isListAtPoint != true )
	{
		for(i=0;i < count; i++)
		{
			fl_BlockLayout * pBlock = (fl_BlockLayout *) vBlock.getNthItem(i);
			fl_BlockLayout * rBlock = pBlock->getPreviousListOfSameMargin();
			if(rBlock != NULL)
			{
				pBlock->resumeList(rBlock);
				pBlock->getDocument()->enableListUpdates();
				pBlock->getDocument()->updateDirtyLists();
			}
		}
	}
	clearDirty();
}

/*!
 *
 * This function loads the standard values into Delim, decimal, format
 * m_fAlign, m_iLevel and m_iStarValue based on m_NewListType
 *
 * m_fAlign and m_fIndent should be in inches
 */
void  AP_Dialog_Lists::fillUncustomizedValues(void)
{
	// if we can get the current font, we will use it where appropriate
	// the "NULL" string does not work too well on Windows in numbered lists
	const XML_Char** props_in = NULL;
	const XML_Char * font_family;
	if (getView()->getCharFormat(&props_in))
		font_family = UT_getAttribute("font-family", props_in);
	else
		font_family =(const XML_Char *) "NULL";

	if(m_NewListType == NOT_A_LIST)
	{
		UT_XML_strncpy( (XML_Char *) m_pszDelim, 80, (const XML_Char *) "%L");
		m_fAlign = 0.0;
		m_fIndent = 0.0;
		m_iLevel = 0;
		UT_XML_strncpy( (XML_Char *) m_pszFont, 80, (const XML_Char *) "NULL");
		UT_XML_strncpy( (XML_Char *) m_pszDecimal, 80, (const XML_Char *) ".");
		m_iStartValue = 1;
	}

	if(m_iLevel <= 0)
	{
		m_iLevel = 1;
	}

	UT_XML_strncpy( (XML_Char *) m_pszDelim, 80, (const XML_Char *) "%L");
	m_fAlign =  (float)(LIST_DEFAULT_INDENT * m_iLevel);
	m_fIndent = (float)-LIST_DEFAULT_INDENT_LABEL;

	if( m_NewListType == NUMBERED_LIST)
	{
		UT_XML_strncpy( (XML_Char *) m_pszFont, 80, font_family);
		UT_XML_strncpy( (XML_Char *) m_pszDecimal, 80, (const XML_Char *) ".");
		m_iStartValue = 1;
		UT_XML_strncpy( (XML_Char *) m_pszDelim, 80, (const XML_Char *) "%L.");
	}
	else if( m_NewListType == LOWERCASE_LIST)
	{
		UT_XML_strncpy( (XML_Char *) m_pszFont, 80, font_family);
		UT_XML_strncpy( (XML_Char *) m_pszDecimal, 80, (const XML_Char *) ".");
		m_iStartValue = 1;
		UT_XML_strncpy( (XML_Char *) m_pszDelim, 80, (const XML_Char *) "%L)");
	}
	else if( m_NewListType == UPPERCASE_LIST)
	{
		UT_XML_strncpy( (XML_Char *) m_pszFont, 80, font_family);
		UT_XML_strncpy( (XML_Char *) m_pszDecimal, 80, (const XML_Char *) ".");
		m_iStartValue = 1;
		UT_XML_strncpy( (XML_Char *) m_pszDelim, 80, (const XML_Char *) "%L)");
	}
	else if( m_NewListType == HEBREW_LIST)
	{
		UT_XML_strncpy( (XML_Char *) m_pszFont, 80, font_family);
		UT_XML_strncpy( (XML_Char *) m_pszDecimal, 80, (const XML_Char *) "");
		m_iStartValue = 1;
		UT_XML_strncpy( (XML_Char *) m_pszDelim, 80, (const XML_Char *) "%L");
	}
	else if( m_NewListType < BULLETED_LIST)
	{
		UT_XML_strncpy( (XML_Char *) m_pszFont, 80, (const XML_Char *) "NULL");
		UT_XML_strncpy( (XML_Char *) m_pszDecimal, 80, (const XML_Char *) ".");
		m_iStartValue = 1;
		UT_XML_strncpy( (XML_Char *) m_pszDelim, 80, (const XML_Char *) "%L");
	}
	else
	{
		UT_XML_strncpy( (XML_Char *) m_pszFont, 80, (const XML_Char *) "NULL");
		UT_XML_strncpy( (XML_Char *) m_pszDecimal, 80, (const XML_Char *) ".");
		m_iStartValue = 0;
	}
	if(m_NewListType == BULLETED_LIST || m_NewListType == IMPLIES_LIST)
	{
		UT_XML_strncpy( (XML_Char *) m_pszFont, 80, (const XML_Char *) "Symbol");
	}
	else if (m_NewListType == NOT_A_LIST)
	{
		UT_XML_strncpy( (XML_Char *) m_pszFont, 80, (const XML_Char *) "NULL");
	}
	else if(m_NewListType > DASHED_LIST && m_NewListType < OTHER_NUMBERED_LISTS)
	{
		UT_XML_strncpy( (XML_Char *) m_pszFont, 80, _getDingbatsFontName());
	}

	if(props_in)
		free(props_in);
}

/*!
 * This method sets the parameters of the "Fake" list shown in the preview.
 * The values display are theones the user should expect to get in their document
 * should they press "Apply"
 */
void  AP_Dialog_Lists::fillFakeLabels(void)
{
/*!
 * m_bisCustomized is true if the user has changed anything in the dialog without
 * pressing "Apply". If this variable is false we should just display what is
 * in the document at the list point.
 */
	if(m_bisCustomized == false && !isModal())
	{
		m_iLevel = getBlock()->getLevel();
		if(m_iLevel == 0 )
		{
			m_iLevel++;
		}
/*!
 * This method loads the list info from the document at the current point
 * into the XP member variables.
 */
		PopulateDialogData();
//
// We may not need this. Will check. Sevior 18/7/2001
//
		if(m_bguiChanged == false)
			m_NewListType = m_DocListType;
		m_bguiChanged = false;
	}
/*!
 * For Bullet type lists we don't allow the user to set either the delimimiter
 * or font from their specified values. Here we override whatever was in them
 * to the values they SHOULD be given the list type.
 */
	if(  m_NewListType == BULLETED_LIST ||
		 m_NewListType == IMPLIES_LIST  ||
		 m_NewListType == DASHED_LIST )
	{
		UT_XML_strncpy( (XML_Char *) m_pszFont, 80, (const XML_Char *) "Symbol");
		UT_XML_strncpy( (XML_Char *) m_pszDelim, 80, (const XML_Char *) "%L");
	}
	else if (m_NewListType == NOT_A_LIST)
	{
		UT_XML_strncpy( (XML_Char *) m_pszFont, 80, (const XML_Char *) "NULL");
		UT_XML_strncpy( (XML_Char *) m_pszDelim, 80, (const XML_Char *) "%L");
	}
	else if(m_NewListType > DASHED_LIST && m_NewListType < OTHER_NUMBERED_LISTS)
	{
		UT_XML_strncpy( (XML_Char *) m_pszFont, 80, _getDingbatsFontName());
		UT_XML_strncpy( (XML_Char *) m_pszDelim, 80, (const XML_Char *) "%L");
	}
/*!
 * OK fill the preview variables with what they need and load them into
 * the preview class.
 */
	m_pFakeAuto->setListType(m_NewListType);
	m_pFakeAuto->setDelim(m_pszDelim);
	m_pFakeAuto->setDecimal(m_pszDecimal);
	m_pFakeAuto->setStartValue(m_iStartValue);
	m_pListsPreview->setData(m_pszFont,m_fAlign,m_fIndent);
}

/*!
 *
 * This routine generates it's own AutoNum's and Layout pointers
 * for use in the preview
 */
void  AP_Dialog_Lists::generateFakeLabels(void)
{
	UT_uint32 i;
	//
	// Start by generating 4 fake (PL_StruxDocHandle and fl_Layout pointers
	//
	// Jeeze gotta generate a fake void * pointer!! Try this hack.
	//
	XAP_App * fakeApp = getApp();
	for(i=0; i<4; i++)
	{
		DELETEP(m_pFakeLayout[i]);
		m_pFakeSdh[i] = (PL_StruxDocHandle) fakeApp++;
		m_pFakeLayout[i] = new fl_Layout((PTStruxType) 0 , (PL_StruxDocHandle) m_pFakeSdh[i] );
	}
	//
	// Now generate the AutoNum
	//
	DELETEP(m_pFakeAuto);
	UNREFP(m_pFakeDoc);
	m_pFakeDoc = new PD_Document(fakeApp);
	m_pFakeAuto = new fl_AutoNum(m_iID, 0, m_NewListType, m_newStartValue, m_pszDelim, m_pszDecimal, (PD_Document *) m_pFakeDoc);
	m_pFakeAuto->insertFirstItem(m_pFakeSdh[0], NULL,1,false);
	m_pFakeLayout[0]->setAutoNum(m_pFakeAuto);
	for(i=1; i<4; i++)
	{
		m_pFakeAuto->insertItem(m_pFakeSdh[i],m_pFakeSdh[i-1],false);
		m_pFakeLayout[i]->setAutoNum(m_pFakeAuto);
	}
}

/*!
 * Little convienence method to get the List label from the FakeAutoNum used in the
 * Preview.
 */
UT_UCSChar * AP_Dialog_Lists::getListLabel(UT_sint32 itemNo)
{
	UT_ASSERT(itemNo < 4);
	static UT_UCSChar lab[80];
	const UT_UCSChar * tmp  = m_pFakeAuto->getLabel(m_pFakeSdh[itemNo]);
	if(tmp == NULL)
	{
		return NULL;
	}
	UT_sint32 cnt = UT_MIN(UT_UCS4_strlen(tmp),80);
	UT_sint32 i;
	for(i =0; i<= cnt; i++)
		lab[i] = *tmp++;
	return lab;
}

/*!
 * The vector vp contains all the properties we need to fill our dialog variables.
 * Fill our variables from this vector.
 * This is used by the Modal dialog and is filled from the styles dialog.
 */
void AP_Dialog_Lists::fillDialogFromVector( UT_Vector * vp)
{
	UT_sint32 i;
	if(vp->getItemCount() > 0)
	{
		i = findVecItem(vp,"start-value");
		if(i >= 0)
		{
			m_iStartValue = atoi( (const XML_Char *) vp->getNthItem(i+1));
		}
		else
		{
			m_iStartValue = 1;
		}

		i = findVecItem(vp,"margin-left");
		if(i>=0)
		{
			m_fAlign = (float)UT_convertToInches((const XML_Char *) vp->getNthItem(i+1));
		}
		else
		{
			m_fAlign = (float)LIST_DEFAULT_INDENT;
		}

		i = findVecItem(vp,"text-indent");
		if(i >= 0)
		{
			m_fIndent = (float)UT_convertToInches((const XML_Char *) vp->getNthItem(i+1));
		}
		else
		{
			m_fIndent = (float)-LIST_DEFAULT_INDENT_LABEL;
		}

		i = findVecItem(vp,"list-delim");
		if( i>= 0)
		{
			UT_XML_strncpy( (XML_Char *) m_pszDelim, 80, (const XML_Char *) vp->getNthItem(i+1));
		}
		else
		{
			UT_XML_strncpy( (XML_Char *) m_pszDelim, 80, (const XML_Char *) "%L");
		}
		i = findVecItem(vp,"list-decimal");
		if( i>= 0)
		{
			UT_XML_strncpy( (XML_Char *) m_pszDecimal, 80, (const XML_Char *) vp->getNthItem(i+1));
		}
		else
		{
			UT_XML_strncpy( (XML_Char *) m_pszDecimal, 80, (const XML_Char *) ".");
		}

		i = findVecItem(vp,"field-font");
		if( i>= 0)
		{
			UT_XML_strncpy( (XML_Char *) m_pszFont, 80, (const XML_Char *) vp->getNthItem(i+1));
		}
		else
		{
			UT_XML_strncpy( (XML_Char *) m_pszFont, 80, (const XML_Char *) "NULL");
		}
		i = findVecItem(vp,"list-style");
		if( i>= 0)
		{
			m_DocListType = getBlock()->getListTypeFromStyle( (const XML_Char *) vp->getNthItem(i+1));
			m_NewListType = m_DocListType;
		}
		else
		{
			m_DocListType = NOT_A_LIST;
			m_NewListType = m_DocListType;
		}
	}
}

/*!
 * This method reads all the List info from the document at the curent point
 * and loads it into the dialog member variables.
 */
void AP_Dialog_Lists::fillDialogFromBlock(void)
{
	UT_Vector va,vp;

	if (getBlock()->getPreviousList() != NULL)
	{
		m_previousListExistsAtPoint = true;
	}
	else
	{
		m_previousListExistsAtPoint = false;
	}
	getBlock()->getListAttributesVector( &va);
	getBlock()->getListPropertyVector( &vp);
	//
	// First do properties.
	//
	UT_sint32 i;
	if(vp.getItemCount() > 0)
	{
		i = findVecItem(&vp,"start-value");
		if(i >= 0)
		{
			m_iStartValue = atoi( (const XML_Char *) vp.getNthItem(i+1));
		}
		else
		{
			m_iStartValue = 1;
		}

		i = findVecItem(&vp,"margin-left");
		if(i>=0)
		{
			m_fAlign = (float)UT_convertToInches((const XML_Char *) vp.getNthItem(i+1));
		}
		else
		{
			m_fAlign = (float)LIST_DEFAULT_INDENT;
		}

		i = findVecItem(&vp,"text-indent");
		if(i >= 0)
		{
			m_fIndent = (float)UT_convertToInches((const XML_Char *) vp.getNthItem(i+1));
		}
		else
		{
			m_fIndent = (float)-LIST_DEFAULT_INDENT_LABEL;
		}

//
// OK for list-delim it is better to use what is in fl_AutoNum first then the
// the paraprops then the defaults. The value for fl_AutoNum is what ends up in
// the users doc.
//
		i = findVecItem(&vp,"list-delim");
		if(getAutoNum())
		{
			UT_XML_strncpy( (XML_Char *) m_pszDelim, 80, getAutoNum()->getDelim());
		}
		else if(i >=0 )
		{
			UT_XML_strncpy( (XML_Char *) m_pszDelim, 80, (const XML_Char *) vp.getNthItem(i+1));
		}
		else
		{
			UT_XML_strncpy( (XML_Char *) m_pszDelim, 80, (const XML_Char *) "%L");
		}


//
// OK for list-delim it is better to use what is in fl_AutoNum first then the
// the paraprops then the defaults. The value for fl_AutoNum is what ends up in
// the users doc.
//
		i = findVecItem(&vp,"list-decimal");
		if(getAutoNum())
		{
			UT_XML_strncpy( (XML_Char *) m_pszDecimal, 80, getAutoNum()->getDecimal());
		}
		else if( i>= 0)
		{
			UT_XML_strncpy( (XML_Char *) m_pszDecimal, 80, (const XML_Char *) vp.getNthItem(i+1));
		}
		else
		{
			UT_XML_strncpy( (XML_Char *) m_pszDecimal, 80, (const XML_Char *) ".");
		}

		i = findVecItem(&vp,"field-font");
		if( i>= 0)
		{
			UT_XML_strncpy( (XML_Char *) m_pszFont, 80, (const XML_Char *) vp.getNthItem(i+1));
		}
		else
		{
			UT_XML_strncpy( (XML_Char *) m_pszFont, 80, (const XML_Char *) "NULL");
		}
		i = findVecItem(&vp,"list-style");
		if( i>= 0)
		{
			m_DocListType = getBlock()->getListTypeFromStyle( (const XML_Char *) vp.getNthItem(i+1));
		}
		else
		{
			m_DocListType = NUMBERED_LIST;
		}
	}
	//
	// Now Do the Attributes first
	//
	if(va.getItemCount()>0)
	{
//  		i = findVecItem(&va,PT_STYLE_ATTRIBUTE_NAME);
//  		if( i>= 0)
//  		{
//  			m_DocListType = getBlock()->getListTypeFromStyle( (const XML_Char *) va.getNthItem(i+1));
//  		}
//  		else
//  		{
//  			m_DocListType = NUMBERED_LIST;
//  		}

		i = findVecItem(&va,"level");
		if( i>= 0)
		{
			m_iLevel = atoi((const XML_Char *) va.getNthItem(i+1));
		}
		else
		{
			m_iLevel = 1;
		}
	}
	if(getAutoNum() != NULL)
	{
		m_iID = getAutoNum()->getID();
		m_DocListType = getAutoNum()->getType();
		UT_XML_strncpy( (XML_Char *) m_pszDecimal, 80, (const XML_Char *) getAutoNum()->getDecimal());

	}
	else
	{
		m_iID = 0;
		m_DocListType = NOT_A_LIST;
	}
}

/*!
 * This method looks to see if there is a list at the current point. If so
 * fill the dialog with that stuff, otherwise fill the dialog with the uncustomized
 * values corresponding to m_NewListType.
 */
void AP_Dialog_Lists::PopulateDialogData(void)
{
	m_isListAtPoint = getBlock()->isListItem();
	if(m_isListAtPoint == true)
	{
		fillDialogFromBlock();
	}
	else
	{
		//	m_NewListType = NOT_A_LIST;
		fillUncustomizedValues();
	}
	if(m_isListAtPoint == true)
	{
		const UT_UCSChar * tmp1 =  getBlock()->getListLabel();
		if(tmp1 != NULL)
		{
			UT_sint32 cnt = UT_MIN(UT_UCS4_strlen(tmp1),80);
			UT_sint32 i;
			for(i =0; i<=cnt; i++)
				m_curListLabel[i] = *tmp1++;
		}
		m_curListLevel = getBlock()->getLevel();
		m_curStartValue = getAutoNum()->getStartValue32();
		m_iStartValue = getAutoNum()->getStartValue32();
		m_DocListType = getAutoNum()->getType();
	}
	else
	{
		m_DocListType = NOT_A_LIST;
		m_curStartValue = 1;
	}
}

UT_uint32 AP_Dialog_Lists::getID(void)
{
       if(getBlock()->isListItem() == false)
       {
	       return 0;
       }
       else
       {
	       return getAutoNum()->getID();
       }
}

/*!
 * This method returns the index to the value corresponding to the
 * key in this props vector
 */
UT_sint32  AP_Dialog_Lists::findVecItem(UT_Vector * v, char * key)
{
	const char* const_key = key;
	return findVecItem( v, const_key);
}

/*!
 * This method returns the index to the value corresponding to the
 * key in this props vector
 */
UT_sint32  AP_Dialog_Lists::findVecItem(UT_Vector * v, const char * key)
{
	UT_sint32 i = v->getItemCount();
	if(i < 0)
		return i;
	UT_sint32 j;
	const char * pszV = NULL;
	for(j= 0; j<i ;j=j+2)
	{
		pszV = (char *) v->getNthItem(j);
		if( (pszV != NULL) && (strcmp( pszV,key) == 0))
			break;
	}
	if( j < i && pszV)
		return j;
	else
		return -1;
}

bool AP_Dialog_Lists::isLastOnLevel(void)
{
	return getAutoNum()->isLastOnLevel(getBlock()->getStruxDocHandle());
}



// --------------------------- Generic Functions -----------------------------


void AP_Dialog_Lists::ConstructWindowName(void)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	XML_Char * tmp = NULL;
	UT_uint32 title_width = 33;
	UT_XML_cloneNoAmpersands(tmp, pSS->getValueUTF8(AP_STRING_ID_DLG_Lists_Title).utf8_str());
	BuildWindowName((char *) m_WindowName,(char*)tmp,title_width);
	FREEP(tmp);
}

void AP_Dialog_Lists::setActiveFrame(XAP_Frame *pFrame)
{
	setView(getView());
	notifyActiveFrame(getActiveFrame());
}

bool AP_Dialog_Lists::setView(FV_View * view)
{
	m_pView = (FV_View *) getActiveFrame()->getCurrentView();
	return true;
}

FV_View * AP_Dialog_Lists::getView(void) const
{
	XAP_Frame * pFrame = getActiveFrame();
	return (FV_View *) pFrame->getCurrentView();
}

AV_View * AP_Dialog_Lists::getAvView(void)
{
	XAP_Frame * pFrame = getActiveFrame();
	return pFrame->getCurrentView();
}

const XML_Char* AP_Dialog_Lists::_getDingbatsFontName() const
{
	return "Dingbats";
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

AP_Lists_preview::AP_Lists_preview(GR_Graphics * gc, AP_Dialog_Lists * pLists)
:	XAP_Preview(gc),
	m_pLists(pLists),
	m_pFont(NULL),
	m_fAlign(0.0f),
	m_fIndent(0.0f),
	m_iLine_height(0),
	m_bFirst(true)
{
	m_iLine_pos[0] = 0;
}

AP_Lists_preview::~AP_Lists_preview()
{
	DELETEP(m_pFont);
}

AP_Dialog_Lists * AP_Lists_preview::getLists(void)
{
	return m_pLists;
}

void AP_Lists_preview::setData(XML_Char * pszFont,float fAlign,float fIndent)
{
	//
	// we draw at 16 points in this preview
	//
	if(!pszFont || strcmp(pszFont,"NULL")== 0)
	{
		m_pFont = m_gc->findFont("Times New Roman", "normal", "", "normal", "", "16pt");
	}
	else
	{
		m_pFont = m_gc->findFont((char *)pszFont, "normal", "", "normal", "", "16pt");
	}	
	UT_ASSERT(m_pFont);
	
	m_fAlign = fAlign;
	m_fIndent = fIndent;
}


void AP_Lists_preview::draw(void)
{
	UT_return_if_fail(m_pFont);
	m_gc->setFont(m_pFont);
	
	UT_RGBColor clrGrey = UT_RGBColor(128,128,128);
	UT_RGBColor clrBlack = UT_RGBColor(0,0,0);
	UT_sint32 iWidth = m_gc->tlu(getWindowWidth());
	UT_sint32 iHeight = m_gc->tlu(getWindowHeight());
	UT_UCSChar ucs_label[50];

	UT_sint32 iDescent = m_gc->getFontDescent();
	UT_sint32 iAscent = m_gc->getFontAscent();
	UT_sint32 iFont = iDescent + iAscent;
	m_iLine_height = iFont;
	//
	// clear our screen
	//
	if (m_bFirst == true)
	{
		m_gc->clearArea(0, 0, iWidth, iHeight);
	}
	m_gc->setColor(clrBlack);
	UT_sint32 yoff = m_gc->tlu(5) ;
	UT_sint32 xoff = m_gc->tlu(5) ;
	UT_sint32 i,ii,yloc,awidth,aheight,maxw;
	UT_sint32 twidth =0;
	UT_sint32 j,xy;
	float z,fwidth;
	// todo 6.5 should be the page width in inches
	float pagew = 2.0;
	aheight = m_gc->tlu(16);
	fwidth = static_cast<float>(m_gc->tdu(iWidth));

	z = (float)((fwidth - 2.0*static_cast<float>(m_gc->tdu(xoff))) /pagew);
  UT_sint32 indent = m_gc->tlu(static_cast<UT_sint32>( z*(m_fAlign+m_fIndent)));

	if(indent < 0)
		indent = 0;
	maxw = 0;
	for(i=0; i<4; i++)
	{
		UT_UCSChar * lv = getLists()->getListLabel(i);
		UT_sint32 len =0;

		if(lv != NULL)
		{
			//
			// This code is here because UT_UCS_copy_char is broken
			//
			len = UT_MIN(UT_UCS4_strlen(lv),51);
			for(j=0; j<=len;j++)
			{
				ucs_label[j] = *lv++;
			}

			ucs_label[len] = 0;

			len = UT_UCS4_strlen(ucs_label);
			yloc = yoff + iAscent + (iHeight - 2*yoff -iFont)*i/4;
			//    m_gc->drawChars(ucs_label,0,len,xoff+indent,yloc);
			twidth = m_gc->measureString(ucs_label,0,len,NULL);
			if(twidth > maxw)
				maxw = twidth;
		}
	}
	//
	// Work out where to put grey areas to represent text
	//
	UT_sint32 xx,yy;
	if(maxw > 0)
		maxw++;

        // UT_sint32 vspace = (iHeight - 2*yoff -iFont)*i/16;
	z = (float)((fwidth - 2.0*static_cast<float>(m_gc->tdu(xoff))) /(float)pagew);
	UT_sint32 ialign = m_gc->tlu(static_cast<UT_sint32>( z*m_fAlign));

	xx = xoff + ialign;
	xy = xoff + ialign;

	if(xx < (xoff + maxw + indent))
		xy = xoff + maxw + indent + m_gc->tlu(1);
	ii = 0;

	for(i=0; i<4; i++)
	{
		yloc = yoff + iAscent + (iHeight - 2*yoff -iFont)*i/4;
		for(j=0; j< 2; j++)
		{
			yy = yloc + m_gc->tlu(5) + j*m_gc->tlu(21);
			m_iLine_pos[ii++] = yy;
		}
	}
	//
	// Now finally draw the preview
	//

	FriBidiCharType iDirection = getLists()->getBlock()->getDominantDirection();

	for(i=0; i<8; i++)
	{
		//
		// First clear the line
		//
		m_gc->clearArea(0, m_iLine_pos[i], iWidth, iHeight);
		if((i & 1) == 0)
		{
			//
			// Draw the text
			//
			UT_UCSChar * lv = getLists()->getListLabel(i/2);
			UT_sint32 len =0;

			if(lv != NULL)
			{
				len = UT_MIN(UT_UCS4_strlen(lv),51);

				if(len > 1 && XAP_App::getApp()->theOSHasBidiSupport() == XAP_App::BIDI_SUPPORT_GUI)
				{
					FriBidiChar * fLogStr = new FriBidiChar[len+1];
					FriBidiChar * fVisStr = new FriBidiChar[len+1];
					UT_ASSERT(fLogStr && fVisStr);

					for(j=0; j<=len;j++)
						fLogStr[j] = lv[j];

					fribidi_log2vis(/* input */
			    		 fLogStr,
					     len,
					     &iDirection,
					     /* output */
					     fVisStr,
					     NULL,
					     NULL,
					     NULL
					     );

					for(j=0; j<=len;j++)
						ucs_label[j] = (UT_UCSChar)fVisStr[j];

					delete [] fLogStr;
					delete [] fVisStr;
				}
				else
				{
					for(j=0; j<=len;j++)
						ucs_label[j] = *lv++;
				}

				ucs_label[len] = 0;
				len = UT_UCS4_strlen(ucs_label);
				yloc = yoff + iAscent + (iHeight - 2*yoff -iFont)*i/8;

				if(iDirection == FRIBIDI_TYPE_RTL)
					m_gc->drawChars(ucs_label,0,len,iWidth - xoff - indent - maxw,yloc);
				else
					m_gc->drawChars(ucs_label,0,len,xoff+indent,yloc);

				yy = m_iLine_pos[i];
				awidth = iWidth - 2*xoff - xy;

				if(iDirection == FRIBIDI_TYPE_RTL)
					m_gc->fillRect(clrGrey,xoff,yy,awidth,aheight);
				else
					m_gc->fillRect(clrGrey,xy,yy,awidth,aheight);
			}
			else
			{
				yy = m_iLine_pos[i];
				awidth = iWidth - 2*xoff - xy;

				if(iDirection == FRIBIDI_TYPE_RTL)
					m_gc->fillRect(clrGrey,xoff,yy,awidth,aheight);
				else
					m_gc->fillRect(clrGrey,xy,yy,awidth,aheight);
			}
		}
		else
		{
			yy = m_iLine_pos[i];
			awidth = iWidth - 2*xoff - xx;

			if(iDirection == FRIBIDI_TYPE_RTL)
				m_gc->fillRect(clrGrey,xoff,yy,awidth,aheight);
			else
				m_gc->fillRect(clrGrey,xy,yy,awidth,aheight);
		}
	}
}
