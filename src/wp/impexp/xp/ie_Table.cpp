/* AbiWord
 * Copyright (C) 2002 Martin Sevior
 *               msevior@physics.unimelb.edu.au
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

/******************************************************************

These are convience classes designed to make the export of table easier.
In particular ie_Table makes it easy to track nested tables and moving between
cells. To use, call the methods OpenTable(api)  and OpenCell(api) on encountering a 
PTX_SectionTable or PTX_SectionCell strux.

Call the methods CloseTable() and CloseCell() on encountering a PTX_EndTable
and PTX_EndCell strux. 

You can access all the properties of the current cell in the
current table via ie_Table::get* methods.
******************************************************************/

#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <locale.h>

#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_string.h"
#include "ut_units.h"
#include "xap_EncodingManager.h"
#include "ut_string_class.h"
#include "ie_Table.h"
#include "pp_AttrProp.h"

class ABI_EXPORT ie_PartTable
{
 public:
	ie_PartTable(PD_Document * pDoc);
	virtual ~ie_PartTable(void);
	void             setDoc(PD_Document * pDoc);
	void             setTableApi(PL_StruxDocHandle sdh,PT_AttrPropIndex iApi);
	void             setCellApi(PT_AttrPropIndex iApi);
	UT_sint32        getLeft(void);
	UT_sint32        getRight(void);
	UT_sint32        getTop(void);
	UT_sint32        getBot(void);
	const char *     getTableProp(const char * pPropName);
	const char *     getCellProp(const char * pPropName);
	UT_sint32        getNumRows(void);
	UT_sint32        getNumCols(void);
 private:
	void                  _setRowsCols(void);
	void                  _clearAll(void);
	void                  _clearAllCell(void);
	PD_Document *         m_pDoc;
	PT_AttrPropIndex      m_apiTable;			
	PT_AttrPropIndex      m_apiCell;
	const PP_AttrProp *   m_TableAttProp;
	const PP_AttrProp *   m_CellAttProp;
	UT_sint32             m_iNumRows;
	UT_sint32             m_iNumCols;
	UT_sint32             m_iLeft;
	UT_sint32             m_iRight;
	UT_sint32             m_iTop;
	UT_sint32             m_iBot;
	PL_StruxDocHandle     m_TableSDH;
};			

/*!
 * Class to hold a particular table and cell.
 */
ie_PartTable::ie_PartTable(PD_Document * pDoc) :
	m_pDoc(pDoc),
	m_apiTable(0), 
	m_apiCell(0), 
	m_TableAttProp(NULL),
	m_CellAttProp(NULL),
	m_iNumRows(0),
	m_iNumCols(0),
	m_iLeft(-1),
	m_iRight(-1),
	m_iTop(-1),
	m_iBot(-1),
	m_TableSDH(NULL)
{
}

ie_PartTable::~ie_PartTable(void)
{
}

/*!
 * Clears just the cell properties stored in the class.
 */
void ie_PartTable::_clearAllCell(void)
{
	m_apiCell = 0;
	m_CellAttProp = NULL;
	m_iLeft = -1;
	m_iRight = -1;
	m_iTop = -1;
	m_iBot = -1;
}

/*!
 * Clears both the Table and Cell properties stored in the class.
 */
void ie_PartTable::_clearAll(void)
{
	_clearAllCell();
	m_apiTable = 0;
	m_TableAttProp = NULL;
	m_iNumRows = 0;
	m_iNumCols = 0;
	m_TableSDH = NULL;
}

/*!
 * Sets the document pointer
 */
void ie_PartTable::setDoc(PD_Document * pDoc)
{
	_clearAll();
	m_pDoc = pDoc;
}

/*!
 * Sets the Attribute/Property index of the Table in the class.
 * This is used to find a pointer to the pp_AttrProp class associated with the index.
 */
void ie_PartTable::setTableApi(PL_StruxDocHandle sdh, PT_AttrPropIndex iApi)
{
	_clearAll();
	m_apiTable = iApi;
	UT_return_if_fail(m_pDoc);
	m_pDoc->getAttrProp(iApi, &m_TableAttProp);
	m_TableSDH = sdh;
	_setRowsCols();
}

/*!
 * Number of rows in the table.
 */
UT_sint32 ie_PartTable::getNumRows(void)
{
	return m_iNumRows;
}

/*!
 * Number of columns in the Table.
 */
UT_sint32 ie_PartTable::getNumCols(void)
{
	return m_iNumCols;
}

/*!
 * The left attach column of the current cell in the current Table.
 */
UT_sint32 ie_PartTable::getLeft(void)
{
	return m_iLeft;
}

/*!
 * The right attach column of the current cell in the current Table.
 */
UT_sint32 ie_PartTable::getRight(void)
{
	return m_iRight;
}


/*!
 * The top attach row of the current cell in the current Table.
 */
UT_sint32 ie_PartTable::getTop(void)
{
	return m_iTop;
}


/*!
 * The bot attach row of the current cell in the current Table.
 */
UT_sint32 ie_PartTable::getBot(void)
{
	return m_iBot;
}

/*!
 * Sets the api of the current cell and all the class cell properties derived from
 * it.
 */
void ie_PartTable::setCellApi(PT_AttrPropIndex iApi)
{
	_clearAllCell();
	m_apiCell = iApi;
	UT_return_if_fail(m_pDoc);
	m_pDoc->getAttrProp(iApi, &m_CellAttProp);
	const char * szVal = NULL;
	szVal = getCellProp("left-attach");
	if(szVal && *szVal)
	{
		m_iLeft = atoi(szVal);
	}
	szVal = getCellProp("right-attach");
	if(szVal && *szVal)
	{
		m_iRight = atoi(szVal);
	}
	szVal = getCellProp("top-attach");
	if(szVal && *szVal)
	{
		m_iTop = atoi(szVal);
	}
	szVal = getCellProp("bot-attach");
	if(szVal && *szVal)
	{
		m_iBot = atoi(szVal);
	}
	if(m_iBot > m_iNumRows)
	{
		m_iNumRows = m_iBot;
	}
	if(m_iRight > m_iNumCols)
	{
		m_iNumCols = m_iRight;
	}
}

/*!
 * Return the value of the property named *pProp of the current Table.
 */
const char * ie_PartTable::getTableProp(const char * pProp)
{
	const char * szVal = NULL;
	if(m_TableAttProp == NULL)
	{
		return NULL;
	}
	m_TableAttProp->getProperty(pProp,szVal);
	return szVal;
}



/*!
 * Return the value of the property named *pProp of the current cell.
 */
const char * ie_PartTable::getCellProp(const char * pProp)
{
	const char * szVal = NULL;
	if(m_CellAttProp == NULL)
	{
		return NULL;
	}
	m_CellAttProp->getProperty(pProp,szVal);
	return szVal;
}

/*!
 * Calculate the number of rows and columns in this table
 * Do this by scanning through the table struxes in the Piece Table 
*/
void ie_PartTable::_setRowsCols(void)
{
	PT_DocPosition posTable = m_pDoc->getStruxPosition(m_TableSDH) + 1;
	PT_DocPosition posEndTable,posCell,posEndCell;
	UT_sint32 Right,Bot;
	PL_StruxDocHandle cellSDH,endTableSDH,endCellSDH;
	bool  bRes = m_pDoc->getNextStruxOfType(m_TableSDH,PTX_EndTable,&endTableSDH);
	if(!bRes)
	{
		//
		// Disaster! the table structure in the piecetable is screwed.
		// we're totally stuffed now.
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}
	posEndTable = m_pDoc->getStruxPosition(endTableSDH);
	bool bEnd = false;
	posCell = posTable +1;
	bRes = m_pDoc->getStruxOfTypeFromPosition(posCell,PTX_SectionCell,&cellSDH);
	posCell = m_pDoc->getStruxPosition(cellSDH);
//
// Do the scan
//
	while(!bEnd)
	{
		bRes = m_pDoc->getNextStruxOfType(cellSDH,PTX_EndCell,&endCellSDH);
		if(!bRes)
		{
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			bEnd = true;
			break;
		}
		posEndCell =  m_pDoc->getStruxPosition(endCellSDH);
		if(posEndCell+1 >= posEndTable)
		{
			bEnd = true;
		}
		posCell =  m_pDoc->getStruxPosition(cellSDH);
		const char * szRight = NULL;
		bool bres = m_pDoc->getPropertyFromSDH(cellSDH,"right-attach",&szRight);
		if(szRight && *szRight)
		{
			Right = atoi(szRight);
		}
		const char * szBot = NULL;
		bres = m_pDoc->getPropertyFromSDH(cellSDH,"bot-attach",&szBot);
		if(szBot && *szBot)
		{
			Bot = atoi(szBot);
		}

		if(m_iNumCols < Right)
		{
			m_iNumCols = Right;
		}
		if(m_iNumRows < Bot)
		{
			m_iNumRows = Bot;
		}
		if(!bEnd)
		{
			bRes = m_pDoc->getNextStruxOfType(cellSDH,PTX_SectionCell,&cellSDH);
			if(!bRes)
			{
				bEnd = true;
				break;
			}
		}
	}
}
/*--------------------------------------------------------------------------------*/

ie_Table::ie_Table(PD_Document * pDoc) :
	m_pDoc(pDoc)
{
	m_sLastTable.push(NULL);
}

ie_Table::ie_Table(void) :
	m_pDoc(NULL)
{
	m_sLastTable.push(NULL);
}

/*!
 * Clean up the stack if needed.
 */
ie_Table::~ie_Table(void)
{
	while(m_sLastTable.getDepth() > 1)
	{
		ie_PartTable * pPT = NULL;
		m_sLastTable.pop((void **)&pPT);
		delete pPT;
	}
}

/*!
 * Set pointer to the document. Clear out all previous table info
 */
void ie_Table::setDoc(PD_Document * pDoc)
{
	m_pDoc = pDoc;
	while(m_sLastTable.getDepth() > 1)
	{
		ie_PartTable * pPT = NULL;
		m_sLastTable.pop((void **) &pPT);
		delete pPT;
	}
}

/*!
 * a table strux has been been found. Push it and it's api onto the stack.
 */
void ie_Table::OpenTable(PL_StruxDocHandle tableSDH, PT_AttrPropIndex iApi)
{
	ie_PartTable * pPT = new ie_PartTable(m_pDoc);
	m_sLastTable.push((void *) pPT);
	pPT->setTableApi(tableSDH,iApi);
}

/*!
 * A cell strux has been found. Update all info with this api.
 */ 
void ie_Table::OpenCell(PT_AttrPropIndex iApi)
{
	ie_PartTable * pPT = NULL;
	m_sLastTable.viewTop((void **) &pPT);
	pPT->setCellApi(iApi);
}


/*!
 * Signal clase of cell from endCell strux
 */
void ie_Table::CloseCell(void)
{
	ie_PartTable * pPT = NULL;
	m_sLastTable.viewTop((void **) &pPT);
	pPT->setCellApi(0);
}


/*!
 * pop the stack on this endTable strux.
 */
void ie_Table::CloseTable(void)
{
	ie_PartTable * pPT = NULL;
	m_sLastTable.pop((void **) &pPT);
	delete pPT;
}

/*!
 * Convience function to get the left attach of the current cell.
 */
UT_sint32 ie_Table::getLeft(void)
{
	ie_PartTable * pPT = NULL;
	m_sLastTable.viewTop((void **) &pPT);
	UT_return_val_if_fail(pPT,0);
	return pPT->getLeft();
}


/*!
 * Convience function to get the right attach of the current cell.
 */
UT_sint32 ie_Table::getRight(void)
{
	ie_PartTable * pPT = NULL;
	m_sLastTable.viewTop((void **) &pPT);
	UT_return_val_if_fail(pPT,0);
	return pPT->getRight();
}


/*!
 * Convience function to get the top attach of the current cell.
 */
UT_sint32 ie_Table::getTop(void)
{
	ie_PartTable * pPT = NULL;
	m_sLastTable.viewTop((void **) &pPT);
	UT_return_val_if_fail(pPT,0);
	return pPT->getTop();
}

/*!
 * Convience function to get the bottom attach of the current cell.
 */
UT_sint32 ie_Table::getBot(void)
{
	ie_PartTable * pPT = NULL;
	m_sLastTable.viewTop((void **) &pPT);
	UT_return_val_if_fail(pPT,0);
	return pPT->getBot();
}

/*!
 * Convience function to get the current number of rows in table.
 */
UT_sint32 ie_Table::getNumRows(void)
{
	ie_PartTable * pPT = NULL;
	m_sLastTable.viewTop((void **) &pPT);
	UT_return_val_if_fail(pPT,0);
	return pPT->getNumRows();
}



/*!
 * Convience function to get the current number of columns in table.
 */
UT_sint32 ie_Table::getNumCols(void)
{
	ie_PartTable * pPT = NULL;
	m_sLastTable.viewTop((void **) &pPT);
	UT_return_val_if_fail(pPT,0);
	return pPT->getNumCols();
}


/*!
 * RTF expects an unnested table to have a nest depth of 1. Since we push NULL
 * at the start this matches nicely witht he getDepth method.
 */
UT_sint32 ie_Table::getNestDepth(void)
{
	return m_sLastTable.getDepth();
}


/*!
 * Return the value of the property named *pProp of the current Table.
 */
const char * ie_Table::getTableProp(const char * pProp)
{
	ie_PartTable * pPT = NULL;
	m_sLastTable.viewTop((void **) &pPT);
	UT_return_val_if_fail(pPT,NULL);
	return pPT->getTableProp(pProp);
}


/*!
 * Return the value of the property named *pProp of the current Cell.
 */
const char * ie_Table::getCellProp(const char * pProp)
{
	ie_PartTable * pPT = NULL;
	m_sLastTable.viewTop((void **) &pPT);
	UT_return_val_if_fail(pPT,NULL);
	return pPT->getCellProp(pProp);
}
