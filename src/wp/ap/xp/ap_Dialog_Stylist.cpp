/* AbiWord
 * Copyright (C) 2003 Dom Lachowicz
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ut_assert.h"
#include "ut_string.h"
#include "ut_debugmsg.h"

#include "ap_Dialog_Stylist.h"
#include "ap_Strings.h"

#include "xap_App.h"
#include "xap_Dialog_Id.h"
#include "xap_DialogFactory.h"
#include "xap_Dlg_MessageBox.h"
#include "fv_View.h"
#include "pd_Document.h"
#include "pd_Style.h"
#include "pt_Types.h"
#include "fp_Line.h"
#include "fp_Run.h"
#include "fp_ContainerObject.h"
#include "fp_TableContainer.h"
#include "fl_TableLayout.h"
#include "fl_BlockLayout.h"
#include "fl_DocLayout.h"
#include "ut_timer.h"
#include "pd_Document.h"

AP_Dialog_Stylist::AP_Dialog_Stylist(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: XAP_Dialog_Modeless(pDlgFactory,id),
	  m_bIsModal(false),
	  m_pDoc(NULL),
	  m_pAutoUpdater(0),
	  m_iTick(0),
	  m_sCurStyle(""),
	  m_pStyleTree(NULL),
	  m_bStyleTreeChanged(true),
	  m_bStyleChanged(true)
{
}

AP_Dialog_Stylist::~AP_Dialog_Stylist(void)
{
	stopUpdater();
	DELETEP(m_pStyleTree);
}

void AP_Dialog_Stylist::setActiveFrame(XAP_Frame * /*pFrame*/)
{
	updateDialog();
	notifyActiveFrame(getActiveFrame());
}

void AP_Dialog_Stylist::startUpdater(void)
{
	m_pAutoUpdater =  UT_Timer::static_constructor(autoUpdate,this);
	m_pAutoUpdater->set(500);
	m_pAutoUpdater->start();
}

/*!
 * Apply current style to the current selection in the current view
 */
void AP_Dialog_Stylist::Apply(void)
{
	if(!getActiveFrame())
		return;
	FV_View * pView = static_cast<FV_View *>(getActiveFrame()->getCurrentView());
	if(pView->getPoint() == 0)
	{
		return;
	}
	pView->setStyle(getCurStyle()->utf8_str());
	pView->notifyListeners(AV_CHG_MOTION | AV_CHG_HDRFTR);
}

void AP_Dialog_Stylist::stopUpdater(void)
{
	if(m_pAutoUpdater == NULL)
	{
		return;
	}
	m_pAutoUpdater->stop();
	DELETEP(m_pAutoUpdater);
	m_pAutoUpdater = NULL;
}

UT_sint32 AP_Dialog_Stylist::getNumStyles(void) const
{
	if(m_pStyleTree == NULL)
	{
		return 0;
	}
	return m_pStyleTree->getNumStyles();
}


/*!
 * Autoupdater of the dialog.
 */
void AP_Dialog_Stylist::autoUpdate(UT_Worker * pTimer)
{

	UT_return_if_fail (pTimer);
	
// this is a static callback method and does not have a 'this' pointer

	AP_Dialog_Stylist * pDialog = static_cast<AP_Dialog_Stylist *>(pTimer->getInstanceData());
	pDialog->updateDialog();
}

/*!
 * This method actually updates the dialog, in particular the style Tree and 
 * the current style.
 */
void AP_Dialog_Stylist::updateDialog(void)
{
	XAP_Frame * frame = getActiveFrame();
	if (frame) {
		// Handshaking code
		FV_View * pView = static_cast<FV_View *>(frame->getCurrentView());
		if(pView->getPoint() == 0)
		{
			return;
		}
		PD_Document * pDoc = pView->getDocument();
		if(m_pStyleTree == NULL)
		{
			m_pStyleTree = new Stylist_tree(pDoc);
		}
		if((m_iTick != pView->getTick()) || (m_pDoc != pDoc))
		{
			m_iTick = pView->getTick();
			if((pDoc != m_pDoc) || (static_cast<UT_sint32>(pDoc->getStyleCount()) != getNumStyles()))
			{
				m_pDoc = pDoc;
				m_pStyleTree->buildStyles(pDoc);
				if(!m_bIsModal) // fill the current style if Modeless
				{
					const char * pszStyle;
					pView->getStyle(&pszStyle);
					m_sCurStyle =  pszStyle;
				}
				m_bStyleTreeChanged =true;
				setStyleInGUI();
				return;
			}
			const char * pszStyle;
			pView->getStyle(&pszStyle);
			UT_UTF8String sCurViewStyle;
			if(!m_bIsModal)
			{
				sCurViewStyle = pszStyle;
			}
			else
			{
				m_bStyleChanged =true;
				setStyleInGUI();
				return;
			}

			if((sCurViewStyle.size ()) > 0 && m_sCurStyle.size() == 0)
			{
				m_sCurStyle = sCurViewStyle;
				m_bStyleChanged =true;
				setStyleInGUI();
				return;
			}
			if(sCurViewStyle != m_sCurStyle)
			{
				m_sCurStyle = sCurViewStyle;
				m_bStyleChanged =true;
				setStyleInGUI();
				return;
			}
		}
	}
	setAllSensitivities();
}			

/*!
 * Finalize the dialog.
 */
void  AP_Dialog_Stylist::finalize(void)
{
	stopUpdater();
	modeless_cleanup();
}

void AP_Dialog_Stylist::setAllSensitivities()
{
	XAP_Frame *frame = XAP_App::getApp()->getLastFocussedFrame();
	if (frame) {
		setSensitivity(true);
	}
	else {
		setSensitivity(false);
	}

}


///////////////////////////////////////////////////////////////////////////
/*!
 * This class holds the current style tree and provides a useful interface
 * to the tree for the GUI.
 */
Stylist_tree::Stylist_tree(PD_Document *pDoc)
{
	m_vecAllStyles.clear();
	m_vecStyleRows.clear();
	buildStyles(pDoc);
}

Stylist_tree::~Stylist_tree(void)
{
	UT_DEBUGMSG(("Deleteing Stylist_tree %p \n",this));
	UT_VECTOR_PURGEALL(Stylist_row *, m_vecStyleRows);
}

/*!
 * This method builds the tree of styles from the Document given.
 */
void Stylist_tree::buildStyles(PD_Document * pDoc)
{
	UT_sint32 numStyles = static_cast<UT_sint32>(pDoc->getStyleCount());
	UT_sint32 i = 0;
	m_vecAllStyles.clear();
	UT_VECTOR_PURGEALL(Stylist_row *, m_vecStyleRows);
	m_vecStyleRows.clear();
	UT_GenericVector<const PD_Style *> vecStyles;
	const PD_Style * pStyle = NULL;
	UT_DEBUGMSG(("In Build styles num styles in doc %d \n",numStyles));

	UT_GenericVector<PD_Style*> * pStyles = NULL;
	pDoc->enumStyles(pStyles);
	UT_return_if_fail( pStyles );

	for(i=0; i < numStyles; i++)
	{
		pStyle = pStyles->getNthItem(i);
		m_vecAllStyles.addItem(pStyle);
		vecStyles.addItem(pStyle);
	}

	delete pStyles;
//
// OK now build the tree of Styles
//
// Start with the heading styles.
//
	const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet ();
	Stylist_row * pStyleRow = new Stylist_row();
	std::string sTmp;
	pSS->getValueUTF8(AP_STRING_ID_DLG_Stylist_HeadingStyles, sTmp);
	pStyleRow->setRowName(sTmp);
	m_vecStyleRows.addItem(pStyleRow);
	for(i=0; i< numStyles; i++)
	{
		pStyle = vecStyles.getNthItem(i);
		if(isHeading(pStyle))
		{
			sTmp = pStyle->getName();
			pStyleRow->addStyle(sTmp);
			vecStyles.setNthItem(i,NULL,NULL);
			UT_DEBUGMSG(("Adding heading style %s \n",sTmp.c_str()));
		}
	}
//
// Next the list styles.
//
	pStyleRow = new Stylist_row();
	pSS->getValueUTF8(AP_STRING_ID_DLG_Stylist_ListStyles, sTmp);
	pStyleRow->setRowName(sTmp);
	m_vecStyleRows.addItem(pStyleRow);
	for(i=0; i< numStyles; i++)
	{
		pStyle = vecStyles.getNthItem(i);
		if(pStyle && isList(pStyle))
		{
			sTmp = pStyle->getName();
			pStyleRow->addStyle(sTmp);
			vecStyles.setNthItem(i,NULL,NULL);
			UT_DEBUGMSG(("Adding List style %s \n",sTmp.c_str()));
		}
	}
//
// Now the Footnote/Endnote.
//
	pStyleRow = new Stylist_row();
	pSS->getValueUTF8(AP_STRING_ID_DLG_Stylist_FootnoteStyles, sTmp);
	pStyleRow->setRowName(sTmp);
	m_vecStyleRows.addItem(pStyleRow);
	for(i=0; i< numStyles; i++)
	{
		pStyle = vecStyles.getNthItem(i);
		if(pStyle && isFootnote(pStyle))
		{
			sTmp = pStyle->getName();
			pStyleRow->addStyle(sTmp);
			vecStyles.setNthItem(i,NULL,NULL);
			UT_DEBUGMSG(("Adding Footnote style %s \n",sTmp.c_str()));
		}
	}
//
// Now the user-defined
//
	pStyleRow = new Stylist_row();
	pSS->getValueUTF8(AP_STRING_ID_DLG_Stylist_UserStyles, sTmp);
	pStyleRow->setRowName(sTmp);
	UT_sint32 iCount = 0;
	for(i=0; i< numStyles; i++)
	{
		pStyle = vecStyles.getNthItem(i);
		if(pStyle && isUser(pStyle))
		{
			sTmp = pStyle->getName();
			pStyleRow->addStyle(sTmp);
			vecStyles.setNthItem(i,NULL,NULL);
			iCount++;
			UT_DEBUGMSG(("Adding User-defined style %s \n",sTmp.c_str()));
		}
	}
	if(iCount > 0)
	{
		m_vecStyleRows.addItem(pStyleRow);
	}
	else
	{
		DELETEP(pStyleRow);
	}
//
// Now everything else
//
	pSS->getValueUTF8(AP_STRING_ID_DLG_Stylist_MiscStyles, sTmp);
	pStyleRow = new Stylist_row();
	pStyleRow->setRowName(sTmp);
	m_vecStyleRows.addItem(pStyleRow);
	for(i=0; i< numStyles; i++)
	{
		pStyle = vecStyles.getNthItem(i);
		if(pStyle)
		{
			sTmp = pStyle->getName();
			pStyleRow->addStyle(sTmp);
			vecStyles.setNthItem(i,NULL,NULL);
			UT_DEBUGMSG(("Adding style %s \n",sTmp.c_str()));
		}
	}
}

/*!
 * Returns true if the style is withinthe "Heading" type of styles.
 */
bool Stylist_tree::isHeading(const PD_Style * pStyle, UT_sint32 iDepth) const
{
	if(pStyle == NULL)
	{
		return false;
	}
	if(strstr(pStyle->getName(),"Heading") != 0)
	{
		return true;
	}
	PD_Style * pUpStyle = pStyle->getBasedOn();
	if(pUpStyle != NULL && iDepth > 0)
	{
		return isHeading(pUpStyle,iDepth-1);
	}
	return false;
}


/*!
 * Returns true if the style is withinthe "List" type of styles.
 */
bool Stylist_tree::isList(const PD_Style * pStyle, UT_sint32 iDepth) const
{
	if(pStyle == NULL)
	{
		return false;
	}
	if(strstr(pStyle->getName(),"List") != 0)
	{
		return true;
	}
	PD_Style * pUpStyle = pStyle->getBasedOn();
	if(pUpStyle != NULL && iDepth > 0)
	{
		return isList(pUpStyle,iDepth-1);
	}
	return false;
}


/*!
 * Returns true if the style is withinthe "Footnote/Endnote" type of styles.
 */
bool Stylist_tree::isFootnote(const PD_Style * pStyle, UT_sint32 iDepth) const
{
	if(pStyle == NULL)
	{
		return false;
	}
	if((strstr(pStyle->getName(),"Footnote") != 0) || (strstr(pStyle->getName(),"Endnote") != 0)) 
	{
		return true;
	}
	PD_Style * pUpStyle = pStyle->getBasedOn();
	if(pUpStyle != NULL && iDepth > 0)
	{
		return isFootnote(pUpStyle,iDepth-1);
	}
	return false;
}

/*!
 * Returns true if the style is a userdefined style.
 */
bool Stylist_tree::isUser(const PD_Style * pStyle) const
{
	if(pStyle == NULL)
	{
		return false;
	}
	return pStyle->isUserDefined();
}

/*!
 * Return the number of rows in the tree.
 */
UT_sint32 Stylist_tree::getNumRows(void) const
{
	return m_vecStyleRows.getItemCount();
}


/*!
 * Return the row and column address of the style given. If not found return 
 * false.
 */
bool Stylist_tree::findStyle(UT_UTF8String & sStyleName,UT_sint32 & row, UT_sint32 & col)
{
	UT_sint32 i =0;
	UT_sint32 numRows = getNumRows();
	bool bFound = false;
	for(i=0; (i<numRows) && !bFound;i++)
	{
		Stylist_row * pStyleRow = static_cast<Stylist_row *>(m_vecStyleRows.getNthItem(i));
		bFound = pStyleRow->findStyle(sStyleName,col);
		if(bFound)
		{
			row = i;
			return true;
		}
	}
	row = -1;
	col = -1;
	return false;
}


/*!
 * Return the style at the row and column given. If the (row,col) address is
 * valid, return false.
 */
bool  Stylist_tree::getStyleAtRowCol(UT_UTF8String & sStyle,UT_sint32 row, UT_sint32 col)
{
	if(row > getNumRows() || (row < 0))
	{
		return false;
	}
	Stylist_row * pStyleRow = static_cast<Stylist_row *>(m_vecStyleRows.getNthItem(row));
	bool bFound = pStyleRow->getStyle(sStyle,col);
	return bFound;
}

/*!
 * return the number of columns at the row given.
 * If the row is invalid return 0.  
 */
UT_sint32 Stylist_tree::getNumCols(UT_sint32 row) const
{
	if(row > getNumRows() || (row < 0))
	{
		return 0;
	}
	Stylist_row * pStyleRow = static_cast<Stylist_row *>(m_vecStyleRows.getNthItem(row));
	return pStyleRow->getNumCols();
}

/*!
 * Return the total number of styles in the tree.
 */
UT_sint32 Stylist_tree::getNumStyles(void) const
{
	return m_vecAllStyles.getItemCount();
}

/*!
 * return the name of the row given. If the row is invalid return false;
 */
bool Stylist_tree::getNameOfRow(std::string &sName, UT_sint32 row) const
{
	if(row > getNumRows() || (row<0) )
	{
		return false;
	}
	Stylist_row * pStyleRow = static_cast<Stylist_row *>(m_vecStyleRows.getNthItem(row));
	pStyleRow->getRowName(sName);
	return true;
}

//////////////////////////////////////////////////////////////////////////
/*!
 * This class holds a row of style names and a useful API to access them.
 */
Stylist_row::Stylist_row(void):
	m_sRowName("")
{
	m_vecStyles.clear();
	UT_DEBUGMSG(("Creating Stylist_row %p \n",this));
}

Stylist_row::~Stylist_row(void)
{
	UT_DEBUGMSG(("Deleteing Stylist_row %p num styles %d\n",this,m_vecStyles.getItemCount()));
	UT_VECTOR_PURGEALL(UT_UTF8String *, m_vecStyles);
}

void Stylist_row::addStyle(const std::string & sStyle)
{
	UT_UTF8String * psStyle = new UT_UTF8String(sStyle);
	m_vecStyles.addItem(psStyle);
}

void Stylist_row::setRowName(const std::string & sRowName)
{
	m_sRowName = sRowName;
}

void Stylist_row::getRowName(std::string & sRowName) const
{
	sRowName = m_sRowName;
}

UT_sint32 Stylist_row::getNumCols(void) const
{
	return m_vecStyles.getItemCount();
}

bool Stylist_row::findStyle(UT_UTF8String & sStyleName, UT_sint32 & col)
{
	UT_sint32 i = 0;
	UT_sint32 numCols = getNumCols();
	bool bFound = false;
	for(i=0; (i<numCols) && !bFound;i++)
	{
		UT_UTF8String * psStyle = m_vecStyles.getNthItem(i);
		if(*psStyle == sStyleName)
		{
			col = i;
			bFound = true;
			return bFound;
		}
	}
	col = -1;
	return false;
}


bool Stylist_row::getStyle(UT_UTF8String & sStyleName, UT_sint32 col)
{
	if((col > getNumCols()) || (col < 0))
	{
		return false;
	}
	UT_UTF8String * psStyle = m_vecStyles.getNthItem(col);
	sStyleName = *psStyle;
	return true;
}

