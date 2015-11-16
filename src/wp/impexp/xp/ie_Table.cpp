/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 2002 Martin Sevior <msevior@physics.unimelb.edu.au>
 * Copyright (C) 2003 Francis James Franklin <fjf@alinameridon.com>
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

/******************************************************************

These are convience classes designed to make the export of table
easier.  In particular ie_Table makes it easy to track nested tables
and moving between cells. To use, call the methods openTable(api) and
openCell(api) on encountering a PTX_SectionTable or PTX_SectionCell
strux.

Call the methods closeTable() and closeCell() on encountering a PTX_EndTable
and PTX_EndCell strux.

You can access all the properties of the current cell in the
current table via ie_Table::get* methods.
******************************************************************/

#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <locale.h>

#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_std_string.h"
#include "ut_units.h"

#include "pd_Document.h"

#include "pf_Frag_Strux.h"

#include "pp_AttrProp.h"

#include "ie_Table.h"

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
	m_iPrevLeft(-1),
	m_iPrevRight(-1),
	m_iPrevTop(-1),
	m_iPrevBot(-1),
	m_TableSDH(NULL),
	m_bIsCellJustOpenned(false),
	m_iCurRow(-1)
{
	xxx_UT_DEBUGMSG(("ie_PartTable created %x \n",this));
}

ie_PartTable::~ie_PartTable(void)
{
	xxx_UT_DEBUGMSG(("ie_PartTable deleted %x \n",this));
}

/*!
 * Clears just the cell properties stored in the class.
 */
void ie_PartTable::_clearAllCell(void)
{
	xxx_UT_DEBUGMSG(("Clearing cell now \n"));
	m_apiCell = 0;
	m_CellAttProp = NULL;
	m_iLeft = -1;
	m_iRight = -1;
	m_iTop = -1;
	m_iBot = -1;
	m_iPrevLeft = -1;
	m_iPrevRight = -1;
	m_iPrevTop = -1;
	m_iPrevBot = -1;
	m_bIsCellJustOpenned = false;
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

void ie_PartTable::setCellJustOpenned(bool b)
{
	m_bIsCellJustOpenned = b;
}

bool ie_PartTable::isCellJustOpenned(void) const
{
	return m_bIsCellJustOpenned;
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
void ie_PartTable::setTableApi(pf_Frag_Strux* sdh, PT_AttrPropIndex iApi)
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
UT_sint32 ie_PartTable::getNumRows(void) const
{
	return m_iNumRows;
}

/*!
 * Number of columns in the Table.
 */
UT_sint32 ie_PartTable::getNumCols(void) const
{
	return m_iNumCols;
}

/*!
 * The left attach column of the current cell in the current Table.
 */
UT_sint32 ie_PartTable::getLeft(void) const
{
	return m_iLeft;
}

/*!
 * The right attach column of the current cell in the current Table.
 */
UT_sint32 ie_PartTable::getRight(void) const
{
	return m_iRight;
}


/*!
 * The top attach row of the current cell in the current Table.
 */
UT_sint32 ie_PartTable::getTop(void) const
{
	return m_iTop;
}


/*!
 * The bot attach row of the current cell in the current Table.
 */
UT_sint32 ie_PartTable::getBot(void) const
{
	return m_iBot;
}

/*!
 * Sets the api of the current cell and all the class cell properties derived from
 * it.
 */
void ie_PartTable::setCellApi(PT_AttrPropIndex iApi)
{
	if(iApi == 0)
	{
		return;
	}
	UT_sint32 iL,iR,iT,iB;
	xxx_UT_DEBUGMSG(("setCellApi to %d \n",iApi));
	xxx_UT_DEBUGMSG(("Old Right was %d \n",m_iRight));
	if(iApi != 	m_apiCell)
	{ 
		iL = m_iLeft;
		iR = m_iRight;
	    iT = m_iTop;
		iB = m_iBot;
	}
	else
	{
		iL = m_iPrevLeft;
		iR = m_iPrevRight;
	    iT = m_iPrevTop;
		iB = m_iPrevBot;
	}
	_clearAllCell();
	m_iPrevLeft = iL;
	m_iPrevRight = iR;
	xxx_UT_DEBUGMSG(("New prevRight is %d \n",m_iPrevRight));
	m_iPrevTop = iT;
	m_iPrevBot = iB;
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
	xxx_UT_DEBUGMSG(("New Right set to %s \n",szVal));
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
const char * ie_PartTable::getTableProp(const char * pProp) const
{
	const gchar * szVal = NULL;
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
const char * ie_PartTable::getCellProp(const char * pProp) const
{
	const gchar * szVal = NULL;
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
	m_pDoc->getRowsColsFromTableSDH(m_TableSDH, true, PD_MAX_REVISION, &m_iNumRows, &m_iNumCols);
}

/*--------------------------------------------------------------------------------*/

ie_Table::ie_Table(PD_Document * pDoc) :
	m_pDoc(pDoc),
	m_bNewRow(false),
	m_sdhLastCell(NULL)
{
	m_sLastTable.push(NULL);
}

ie_Table::ie_Table(void) :
	m_pDoc(NULL),
	m_bNewRow(false),
	m_sdhLastCell(NULL)
{
	m_sLastTable.push(NULL);
}

/*!
 * Clean up the stack if needed.
 */
ie_Table::~ie_Table(void)
{
	_clearLastTables();
}

/*!
 * Clean up the stack
 * Must be safe to call from the dtor
 */
void ie_Table::_clearLastTables()
{
	while(!m_sLastTable.empty())
	{
		ie_PartTable * pPT = m_sLastTable.top();
		m_sLastTable.pop();
		delete pPT;
	}
}
/*!
 * Set pointer to the document. Clear out all previous table info
 */
void ie_Table::setDoc(PD_Document * pDoc)
{
	m_pDoc = pDoc;
	m_sdhLastCell = NULL;
	_clearLastTables();
}

/*!
 * a table strux has been been found. Push it and it's api onto the stack.
 */
void ie_Table::openTable(pf_Frag_Strux* tableSDH, PT_AttrPropIndex iApi)
{
	ie_PartTable * pPT = new ie_PartTable(m_pDoc);
	m_sdhLastCell = NULL;
	m_sLastTable.push(pPT);
	pPT->setTableApi(tableSDH,iApi);
}

/*!
 * A cell strux has been found. Update all info with this api.
 */ 
void ie_Table::openCell(PT_AttrPropIndex iApi)
{
	ie_PartTable * pPT = m_sLastTable.top();
	UT_return_if_fail(pPT != NULL);

	UT_sint32 iOldTop = pPT->getTop();
	pPT->setCellApi(iApi);
	pPT->setCellJustOpenned(true);
	if(pPT->getTop() > iOldTop)
	{
		m_bNewRow = true;
	}
	else
	{
		m_bNewRow = false;
	}
}

bool ie_Table::isNewRow(void) const
{
	return m_bNewRow;
}

bool ie_Table::isCellJustOpenned(void) const
{
	ie_PartTable * pPT = m_sLastTable.top();
	return pPT->isCellJustOpenned();
}

void ie_Table::setCellJustOpenned(bool b)
{
	ie_PartTable * pPT = m_sLastTable.top();
	pPT->setCellJustOpenned(b);
}

/*!
 * Return the current table SDH for debugging purposes.
 */
pf_Frag_Strux* ie_Table::getTableSDH(void)
{
	ie_PartTable * pPT = m_sLastTable.top();
	if(pPT)
	{
		return pPT->getTableSDH();
	}
	return NULL;
}

/*!
 * Signal close of cell from endCell strux
 */
void ie_Table::closeCell(void)
{
	ie_PartTable * pPT = m_sLastTable.top();
	pPT->setCellApi(0);
}


/*!
 * pop the stack on this endTable strux.
 */
void ie_Table::closeTable(void)
{
	ie_PartTable * pPT = m_sLastTable.top();
	m_sLastTable.pop();
	delete pPT;
	m_sdhLastCell = NULL;
}

/*!
 * The returns the Right attached column of the previous cell. We
 * needs to get vertically merged cells at the right edge of a table.
 */
UT_sint32 ie_Table::getPrevNumRightMostVMerged(void) const
{
	ie_PartTable * pPT = m_sLastTable.top();
	xxx_UT_DEBUGMSG(("PrevRight %d curRight %d \n",pPT->getPrevRight(),pPT->getRight()));
	UT_sint32 num = pPT->getNumCols() - pPT->getPrevRight();
	return num;
}


/*!
 * This returns the current row counter
 */
UT_sint32 ie_Table::getCurRow(void) const
{
	ie_PartTable * pPT = m_sLastTable.top();
	UT_return_val_if_fail(pPT != NULL, 0);

	return pPT->getCurRow();
}

/*!
 * This increments the current row counter
 */
void ie_Table::incCurRow(void)
{
	ie_PartTable * pPT = m_sLastTable.top();
	pPT->incCurRow();
}


/*!
 * Convience function to get the left attach of the current cell.
 */
UT_sint32 ie_Table::getLeft(void) const
{
	ie_PartTable * pPT = m_sLastTable.top();
	UT_return_val_if_fail(pPT,0);
	return pPT->getLeft();
}


/*!
 * Convience function to get the right attach of the current cell.
 */
UT_sint32 ie_Table::getRight(void) const
{
	ie_PartTable * pPT = m_sLastTable.top();
	UT_return_val_if_fail(pPT,0);
	return pPT->getRight();
}


/*!
 * Convience function to get the top attach of the current cell.
 */
UT_sint32 ie_Table::getTop(void) const
{
	ie_PartTable * pPT = m_sLastTable.top();
	UT_return_val_if_fail(pPT,0);
	return pPT->getTop();
}

/*!
 * Convience function to get the bottom attach of the current cell.
 */
UT_sint32 ie_Table::getBot(void) const
{
	ie_PartTable * pPT = m_sLastTable.top();
	UT_return_val_if_fail(pPT,0);
	return pPT->getBot();
}

/*!
 * Convience function to get the current number of rows in table.
 */
UT_sint32 ie_Table::getNumRows(void) const
{
	ie_PartTable * pPT = m_sLastTable.top();
	UT_return_val_if_fail(pPT,0);
	return pPT->getNumRows();
}



/*!
 * Convience function to get the current number of columns in table.
 */
UT_sint32 ie_Table::getNumCols(void) const
{
	ie_PartTable * pPT = m_sLastTable.top();
	UT_return_val_if_fail(pPT,0);
	return pPT->getNumCols();
}


/*!
 * RTF expects an unnested table to have a nest depth of 0. Since we push NULL
 * at the start we have to subtract this from the depth calculation.
 */
UT_sint32 ie_Table::getNestDepth(void) const
{
	return m_sLastTable.size() - 1;
}


/*!
 * Return the api of the current Table.
 */
 PT_AttrPropIndex ie_Table::getTableAPI(void) const
{
	ie_PartTable * pPT = m_sLastTable.top();
	UT_return_val_if_fail(pPT,0);
	return pPT->getTableAPI();
}

/*!
 * Return the api of the current Cell.
 */
 PT_AttrPropIndex ie_Table::getCellAPI(void) const
{
	ie_PartTable * pPT = m_sLastTable.top();
	UT_return_val_if_fail(pPT,0);
	return pPT->getCellAPI();
}


/*!
 * Return the value of the property named *pProp of the current Table.
 */
const char * ie_Table::getTableProp(const char * pProp) const
{
	ie_PartTable * pPT = m_sLastTable.top();
	UT_return_val_if_fail(pPT,NULL);
	return pPT->getTableProp(pProp);
}


/*!
 * Return the value of the property named *pProp of the current Cell.
 */
const char * ie_Table::getCellProp(const char * pProp) const
{
	ie_PartTable * pPT = m_sLastTable.top();
	UT_return_val_if_fail(pPT,NULL);
	return pPT->getCellProp(pProp);
}
/*!
 * Set the cell api on the top of the stack to that at location (row,col)
 * If there is no cell at the (row,col) the cellApi is not changed. 
*/
void ie_Table::setCellRowCol(UT_sint32 row, UT_sint32 col)
{
	ie_PartTable * pPT = m_sLastTable.top();
	UT_return_if_fail(pPT);
	pf_Frag_Strux* sdhStart = m_sdhLastCell;
	if(sdhStart == NULL)
	{
		sdhStart = pPT->getTableSDH();
	}
	pf_Frag_Strux* cellSDH = m_pDoc->getCellSDHFromRowCol(sdhStart,true,PD_MAX_REVISION,row,col);
	if(cellSDH == NULL)
	{
		sdhStart = pPT->getTableSDH();
		cellSDH = m_pDoc->getCellSDHFromRowCol(sdhStart,true,PD_MAX_REVISION,row,col);
	}
	m_sdhLastCell = cellSDH;
	if(cellSDH != NULL)
	{
		PT_AttrPropIndex api = m_pDoc->getAPIFromSDH(cellSDH);
		pPT->setCellApi(api);
	}
}

/*---------------------------------------------------------------------------------------------*/

/*!
 * These classes aid the import of table information. They we designed to import RTF but they might
 * useful for other classes too.
 */
ie_imp_cell::ie_imp_cell(ie_imp_table * pImpTable, PD_Document * pDoc, 
						 ie_imp_cell * pLeftImpCell, UT_sint32 iRow):
	m_pDoc(pDoc),
	m_iCellX(-1),
	m_iLeft(-1),
	m_iRight(-1),
	m_iTop(-1),
	m_iBot(-1),
	m_cellSDH(NULL),
	m_pImpTable(pImpTable),
	m_pCellLeft(pLeftImpCell),
	m_iRow(iRow),
	m_bMergeAbove(false),
	m_bMergeRight(false),
	m_bMergeLeft(false),
	m_bFirstVertical(false),
	m_bFirstHori(false)
{
	m_sCellProps.clear();
	xxx_UT_DEBUGMSG(("Cell %x created \n",this));
}


ie_imp_cell::~ie_imp_cell(void)
{
}

/*!
 * Set the cellX value for the cell. rtf uses this to distinguish between cells. All the cells with the
 * same cellx have in the same column.
 * The value of cellX is the right most-edge of the cell including 0.5 of the spacing to the next cell 
 * in uints of twips.
 */
void ie_imp_cell::setCellX(UT_sint32 cellx)
{
	m_iCellX = cellx;
}

/*!
 * Get the cellX value for the cell.
 */
UT_sint32 ie_imp_cell::getCellX(void) const
{
	return m_iCellX;
}

/*!
 * set a pointer to the cell immedidately left of this one.
 */
void ie_imp_cell::setCellLeft(ie_imp_cell * pImpCell)
{
	m_pCellLeft = pImpCell;
}

/*!
 * set Left attach for this this cell..
 */
void ie_imp_cell::setLeft(UT_sint32 iLeft)
{
	m_iLeft = iLeft;
	setProp("left-attach", UT_std_string_sprintf("%d",iLeft));
}

/*!
 * Get the left attach for the cell
 */
UT_sint32 ie_imp_cell::getLeft(void) const
{
	return m_iLeft;
}

/*!
 * set Right attach for this this cell..
 */
void ie_imp_cell::setRight(UT_sint32 iRight)
{
	m_iRight = iRight;
	setProp("right-attach", UT_std_string_sprintf("%d",iRight));
}

/*!
 * Get the right attach for the cell
 */
UT_sint32 ie_imp_cell::getRight(void) const
{
	return m_iRight;
}

/*!
 * set top attach for this this cell..
 */
void ie_imp_cell::setTop(UT_sint32 iTop)
{
	m_iTop = iTop;
	setProp("top-attach", UT_std_string_sprintf("%d",iTop));
}

/*!
 * Get the top attach for the cell
 */
UT_sint32 ie_imp_cell::getTop(void) const
{
	return m_iTop;
}

/*!
 * set bottom attach for this this cell..
 */
void ie_imp_cell::setBot(UT_sint32 iBot)
{
	m_iBot = iBot;
	setProp("bot-attach", UT_std_string_sprintf("%d",iBot));
}

/*!
 * Get the bottom attach for the cell
 */
UT_sint32 ie_imp_cell::getBot(void) const
{
	return m_iBot;
}

/*!
 * Get the cell SDH for this cell.
 */
pf_Frag_Strux* ie_imp_cell::getCellSDH(void) const
{
	return m_cellSDH;
}

/*!
 * Set Cell SDH 
 */
void ie_imp_cell::setCellSDH(pf_Frag_Strux* cellSDH)
{
	m_cellSDH = cellSDH;
}

/*!
 * Write all the properties of this cell to the piecetable without throwing a changerecord
 * return false if no cellSDH is present.
 * true otherwise
 */
bool ie_imp_cell::writeCellPropsInDoc(void) const
{
	if(m_cellSDH == NULL)
	{
		return false;
	}
	xxx_UT_DEBUGMSG(("Cell props are %s \n",m_sCellProps.c_str()));
	m_pDoc->changeStruxAttsNoUpdate(m_cellSDH,"props",m_sCellProps.c_str());
	return true;
}

/*!
 * Return a pointer to the import cell class above this one.
 */
ie_imp_cell * ie_imp_cell::getCellAbove(void) const
{
	return NULL;
}

/*!
 * Return a pointer to the import cell class below this one.
 */
ie_imp_cell * ie_imp_cell::getCellBelow(void) const
{
	return NULL;
}

/*!
 * Return a pointer to the import cell class right of this one.
 */
ie_imp_cell * ie_imp_cell::getCellRight(void) const
{
	return NULL;
}

/*!
 * Return a pointer to the import cell class left of this one.
 */
ie_imp_cell * ie_imp_cell::getCellLeft(void) const
{
	return m_pCellLeft;
}

/*!
 * set a property of this cell.
 */
void ie_imp_cell::setProp(const std::string & psProp, const std::string & psVal)
{
	UT_std_string_setProperty(m_sCellProps, psProp, psVal);
}

/*!
 * Add a list of properties to the cell definition. The definition is the
 * standard prop:value; pair
 */
void ie_imp_cell::addPropString(const std::string & sPropString)
{
	UT_std_string_addPropertyString(m_sCellProps, sPropString);
}

/*!
 * set a property of this cell.
 */
void ie_imp_cell::setProp(const char * szProp, const char * szVal)
{
	std::string psProp = szProp;
	std::string psVal = szVal;
	UT_std_string_setProperty(m_sCellProps, psProp, psVal);
}

/*!
 * Return the value of a property of this cell. This should be deleted when you've finished with it.
 */
std::string ie_imp_cell::getPropVal(const std::string & psProp) const
{
	return UT_std_string_getPropVal(m_sCellProps, psProp);
}

/*!
 * Copy the relevant contents of one cell to another. Useful for rows of
 * of cells with properties identical to the previous row.
 */
void ie_imp_cell::copyCell(ie_imp_cell * pCell)
{
	m_iCellX = pCell->m_iCellX;
	m_bMergeAbove = pCell->m_bMergeAbove;
	m_bMergeRight = pCell->m_bMergeRight;
	m_sCellProps = pCell->m_sCellProps;
	m_bMergeLeft = pCell->m_bMergeLeft;
	m_bFirstHori = pCell->m_bFirstHori;
}

/*!
 * Return the value of a property of this cell. This should be deleted when you've finished with it.
 */
std::string ie_imp_cell::getPropVal(const char * szProp) const
{
	std::string psProp = szProp;
	return UT_std_string_getPropVal(m_sCellProps, psProp);
}

/*!
 * Class for handling import of tables. Built for RTF but might be useful elsewhere.
 */
ie_imp_table::ie_imp_table(PD_Document * pDoc):
	m_pDoc(pDoc),
	m_tableSDH(NULL),
	m_pCurImpCell(NULL),
	m_iRowCounter(0),
	m_bAutoFit(false),
	m_bNewRow(true),
	m_bTableUsed(false),
	m_iPosOnRow(0),
    m_iCellXOnRow(0)
{
	m_sTableProps.clear();
	m_vecCells.clear();
	m_vecCellX.clear();
}

ie_imp_table::~ie_imp_table(void)
{
	xxx_UT_DEBUGMSG(("SEVIOR: deleteing table %x table used %d \n",this,m_bTableUsed));
	if(!m_bTableUsed)
	{
		_removeAllStruxes();
	}
	UT_VECTOR_PURGEALL(ie_imp_cell *,m_vecCells);
}

/*!
 * Open a new cell.
 */
UT_sint32 ie_imp_table::OpenCell(void)
{
	ie_imp_cell * pNewCell = new ie_imp_cell(this, m_pDoc,m_pCurImpCell,m_iRowCounter);
	m_pCurImpCell = pNewCell;
	m_vecCells.addItem(pNewCell);
	UT_sint32 count =0;
	UT_sint32 i = m_vecCells.getItemCount() - 1;
	while((pNewCell->getRow() == m_iRowCounter) && (i>= 0))
	{
		pNewCell = m_vecCells.getNthItem(i);
		if(pNewCell->getRow() == m_iRowCounter)
		{
			count++;
		}
		i--;
	}
	m_bNewRow = false;
	return count -1;
}

/*!
 * Returns a vector of pointers to cells on the requested row.
 * pVec should be empty initially.
 */
bool ie_imp_table::getVecOfCellsOnRow(UT_sint32 row, UT_GenericVector<ie_imp_cell*> * pVec) const
{
	UT_sint32 i = 0;
	ie_imp_cell * pCell = NULL;
	bool bFound = false;
	UT_sint32 iFound = 0;
	for(i=0; !bFound && (i < m_vecCells.getItemCount()); i++)
	{
		pCell = m_vecCells.getNthItem(i);
		if(pCell->getRow() == row)
		{
			bFound = true;
			iFound = i;
		}
	}
	if(!bFound)
	{
		return bFound;
	}
	bool bEnd = false;
	for(i=iFound; !bEnd && (i<m_vecCells.getItemCount()); i++)
	{
		pCell = m_vecCells.getNthItem(i);
		if(pCell->getRow() != row)
		{
			bEnd = true;
		}
		else
		{
			pVec->addItem(pCell);
			xxx_UT_DEBUGMSG(("SEVIOR: Adding cell %d with cellx %d to row vec \n",i-iFound,pCell->getCellX()));
		}
	}
	return true;
}


bool ie_imp_table::doCellXMatch(UT_sint32 iCellX1, UT_sint32 iCellX2, bool bLast  /* = false  */)
{
	UT_sint32 fuz = 20; // CellXs within 20 TWIPS are assumed to be the same
	if(bLast)
	{
		fuz = 300;
	}
	if(iCellX1 > iCellX2)
	{
		if((iCellX1 - iCellX2) < fuz)
		{
			return true;
		}
		return false;
	}
	else if( iCellX2 > iCellX1)
	{
		if( (iCellX2 - iCellX1) < fuz)
		{
			return true;
		}
		return false;
	}
	return true;
}


/*!
 * Start a new row. 
\returns This returns -1 on error.
\returns 0 on Normal.
\returns +1 if the row should be the first row of a new Table. 
(This is decided is the number cellx valus inthe row don't match.)
  */
UT_sint32 ie_imp_table::NewRow(void)
{
	UT_DEBUGMSG(("Doing NewRow in ie_imp_table rowcounter %d \n",m_iRowCounter));
	if(m_iRowCounter > 0)
	{
		ie_imp_cell * pCell = getNthCellOnRow(0);
		ie_imp_cell * pPrevCell = NULL;
		UT_GenericVector<ie_imp_cell*> vecPrev;
		UT_GenericVector<ie_imp_cell*> vecCur;
		vecPrev.clear();
		vecCur.clear();
		getVecOfCellsOnRow(m_iRowCounter-1, &vecPrev);
		getVecOfCellsOnRow(m_iRowCounter, &vecCur);
		UT_sint32 szPrevRow = vecPrev.getItemCount();
		UT_sint32 szCurRow = vecCur.getItemCount();
//
// Look if this row is just a copy of the previous. We decide this if there
// are no values of cellX set.
//
		UT_sint32 i =0;
		for(i=0; i < szCurRow; i++)
		{
			pCell = vecCur.getNthItem(i);
			if(pCell->getCellX() == -1)
			{
				if(i >= szPrevRow)
				{
					//
					// Might have more cells on this row than the previous.
					// In which case we should just start a new table.
					//
					return 1;
				}
				else
				{
					pPrevCell = vecPrev.getNthItem(i);
					pCell->copyCell(pPrevCell);
				}
			}
		}
//
// Now look for numbers of matching cellx between rows. If the new row has a
// wholely different cellx structure we start a new table.
//
		UT_sint32 iMatch = 0;
		for(i=0; i < szCurRow; i++)
		{
			pCell = vecCur.getNthItem(i);
			UT_sint32 curX = pCell->getCellX();
			UT_DEBUGMSG(("Cur cell %d cellx %d \n",i,curX));
			bool bMatch = false;
			UT_sint32 j = 0;
			for(j=0; !bMatch && (j < m_vecCellX.getItemCount()); j++)
			{
				UT_sint32 prevX = m_vecCellX.getNthItem(j);
				UT_DEBUGMSG(("Prev cell %d cellx %d \n",j,prevX));
				bool bLast = ((j-1) == szCurRow);
				bMatch =  doCellXMatch(prevX,curX,bLast);
			}
			if(bMatch)
			{
				iMatch++;
			}
		}
		UT_DEBUGMSG(("SEVIOR: iMatch = %d \n",iMatch));
		if(iMatch == 0)
		{
			return +1;
		}
		double dMatch = static_cast<double>(iMatch);
		double dPrev = static_cast<double>(szCurRow);
		if(dMatch/dPrev < 0.6)
		{
			return +1;
		}
#if 0
		if(dMatch/dPrev > 1.1)
		{
			return +1;
		}
		if(szCurRow != szPrevRow)
		{
			return +1;
		}
#endif
	}
	m_pCurImpCell = NULL;
	m_iRowCounter++;
	m_iPosOnRow = 0;
	m_iCellXOnRow = 0;
	m_bNewRow = true;
	_buildCellXVector();
	return 0;
}

/*!
 * Set the current cell to that at row row and at position col past the first cell on
 * this row.
 */
void ie_imp_table::setCellRowNthCell(UT_sint32 row, UT_sint32 col)
{
	UT_sint32 i =0;
	ie_imp_cell * pCell = NULL;
	UT_sint32 ColCount = 0;
	bool bFound = false;
	for(i=0; !bFound && (i < m_vecCells.getItemCount()); i++)
	{
		pCell = m_vecCells.getNthItem(i);
		if(pCell->getRow() == row)
		{
			xxx_UT_DEBUGMSG(("SEVIOR: col %d colcount %d \n",col,ColCount));
			if(col == ColCount)
			{
				bFound = true;
			}
			ColCount++;
		}
	}
	if(!bFound)
	{
		UT_ASSERT_HARMLESS(0);
		m_pCurImpCell = NULL;
	}
	else
	{
		m_pCurImpCell = pCell;
	}
}

/*!
 * Set this cell to have the cellx value given.
 */
void ie_imp_table::setCellX(UT_sint32 cellx)
{
	UT_return_if_fail(m_pCurImpCell);
	m_pCurImpCell->setCellX(cellx);
}

/*!
 * Return this tables SDH
 */
pf_Frag_Strux* ie_imp_table::getTableSDH(void) const
{
	return m_tableSDH;
}

/*!
 * Set the SDH for this table
 */
void ie_imp_table::setTableSDH(pf_Frag_Strux* sdh)
{
	m_tableSDH = sdh;
	xxx_UT_DEBUGMSG(("SEVIOR: Table sdh set to %x \n",sdh));
}

/*!
 * Write out all the properties in the properties string to the tableSDH
 */
void ie_imp_table::writeTablePropsInDoc(void)
{
	UT_return_if_fail(m_tableSDH);
	std::string colwidths;
	UT_sint32 i=0;
/*
	table-column-props:1.2in/3.0in/1.3in/;

   So we read back in pszColumnProps
   1.2in/3.0in/1.3in/

   The "/" characters will be used to delineate different column entries.
   As new properties for each column are defined these will be delineated with "_"
   characters. But we'll cross that bridge later. Right now only column widths are implemented.

   To fill this we have to translate the cellx positions in twips (which are the right edge
   of the cell plus 0.5 os the gap between cells, to what abiword wants, which are the widths
   of the cell without the spacings. We have to subtract the left-col position plus the
   cell spacings from the cellx positions to get this.

   OK start by looking up table-col-spacing and table-column-leftpos. The defaults for
   these if undefined are 0.05in and 0.0in respectively.
*/
	std::string sColSpace = getPropVal("table-col-spacing");
	if(sColSpace.size() == 0)
	{
		sColSpace = "0.02in";
	}
	std::string sLeftPos = getPropVal("table-column-leftpos");
	if(sLeftPos.size()==0)
	{
		sLeftPos = "0.0in";
	}
	double dColSpace = UT_convertToInches(sColSpace.c_str());
	double dLeftPos = UT_convertToInches(sLeftPos.c_str());
	setProp("table-col-spacing",sColSpace.c_str());
	setProp("table-column-leftpos",sLeftPos.c_str());
	UT_sint32 iPrev = static_cast<UT_sint32>(dLeftPos*1440.0);
	if(!m_bAutoFit)
	{
//
// Now build the table-col-width string.
//
		std::string sColWidth;
		sColWidth.clear();
		for(i=0; i< m_vecCellX.getItemCount(); i++)
		{
			UT_sint32 iCellx = m_vecCellX.getNthItem(i);
			xxx_UT_DEBUGMSG(("final cellx import cellx %d iPrev %x \n",iCellx,iPrev));
			UT_sint32 iDiffCellx = iCellx - iPrev;
			double dCellx = static_cast<double>(iDiffCellx)/1440.0 -dColSpace;
			iPrev = iCellx;
			std::string sWidth = UT_formatDimensionString(DIM_IN,dCellx,NULL);
			sColWidth += sWidth;
			sColWidth += "/";
		}
		setProp("table-column-props",sColWidth.c_str());
	}
	xxx_UT_DEBUGMSG(("SEVIOR: props: %s \n",m_sTableProps.c_str()));
	m_pDoc->changeStruxAttsNoUpdate(m_tableSDH,"props",m_sTableProps.c_str());
}

/*!
 * Write out all the properties in all the cells to the PieceTable
 */
void ie_imp_table::writeAllCellPropsInDoc(void)
{
	UT_sint32 i =0;
	ie_imp_cell * pCell = NULL;
#if DEBUG
	ie_imp_cell * pOldCell = NULL;
#endif
	for(i=0; i< m_vecCells.getItemCount();i++)
	{
		pCell = m_vecCells.getNthItem(i);
		if(!pCell->isMergedAbove() && !pCell->isMergedRight() && !pCell->isMergedLeft())
		{
			bool bCellPresent = pCell->writeCellPropsInDoc();
			if(!bCellPresent)
			{
				//				removeOnThisCellRow(pCell);
				continue;
			}
			UT_DEBUGMSG(("writeallcellprops: pCell %d row %d left %d right %d top %d bot %d sdh %p \n",i,pCell->getRow(),pCell->getLeft(),pCell->getRight(),pCell->getTop(),pCell->getBot(),pCell->getCellSDH())); 
		}
		if(pCell->isMergedAbove() && (pCell->getCellSDH() != NULL))
		{
			UT_DEBUGMSG(("BUG!BUG! found a sdh is merged above cell! removing it \n"));
			pf_Frag_Strux* cellSDH = pCell->getCellSDH();
			UT_return_if_fail(cellSDH != NULL);
			pf_Frag_Strux* nextSDH = NULL;
			m_pDoc->getNextStrux(cellSDH,&nextSDH);
			bool bStop = (cellSDH == nextSDH);
			m_pDoc->deleteStruxNoUpdate(cellSDH);
			while(!bStop && (nextSDH != NULL) && (m_pDoc->getStruxType(nextSDH) != PTX_SectionCell))
			{
				if(	cellSDH == nextSDH)
				{
					break;
				}
				cellSDH = nextSDH;
				m_pDoc->getNextStrux(cellSDH,&nextSDH);
				m_pDoc->deleteStruxNoUpdate(cellSDH);
				if(	cellSDH == nextSDH)
				{
					break;
				}
			}
		}
		if(pCell->isMergedLeft() && (pCell->getCellSDH() != NULL))
		{
			UT_DEBUGMSG(("BUG!BUG! found a sdh is merged left cell! removing it \n"));
			pf_Frag_Strux* cellSDH = pCell->getCellSDH();
			UT_return_if_fail(cellSDH != NULL);
			pf_Frag_Strux* nextSDH = NULL;
			m_pDoc->getNextStrux(cellSDH,&nextSDH);
			m_pDoc->deleteStruxNoUpdate(cellSDH);
			while((nextSDH != NULL) && (m_pDoc->getStruxType(nextSDH) != PTX_SectionCell))
			{
				cellSDH = nextSDH;
				m_pDoc->getNextStrux(cellSDH,&nextSDH);
				m_pDoc->deleteStruxNoUpdate(cellSDH);
			}
		}
#if DEBUG
		if(pOldCell)
		{
			if((pOldCell->getTop() == pCell->getTop()) && (pCell->getLeft() != pOldCell->getRight()))
			{
				UT_DEBUGMSG(("Illegal cell structure!!\n"));
				UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			}
		}
		pOldCell = pCell;
#endif
	}
}

/*!
 * Set a property in the table properties string.
 */
void ie_imp_table::setProp(const std::string & psProp,
						   const std::string & psVal)
{
	UT_std_string_setProperty(m_sTableProps, psProp, psVal);
}

/*!
 * Return the value of a property of this table. 
 * This should be deleted when you've finished with it.
 */
std::string ie_imp_table::getPropVal(const std::string & psProp) const
{
	return UT_std_string_getPropVal(m_sTableProps, psProp);
}


/*!
 * Set a property in the table properties string.
 */
void ie_imp_table::setProp(const char * szProp, const char * szVal)
{
	std::string psProp = szProp;
	std::string psVal = szVal;
	UT_std_string_setProperty(m_sTableProps, psProp, psVal);
}

/*!
 * Return the value of a property of this table. 
 * This should be deleted when you've finished with it.
 */
std::string ie_imp_table::getPropVal(const char * szProp) const
{
	std::string psProp = szProp;
	return UT_std_string_getPropVal(m_sTableProps, psProp);
}

/*!
 * Set a property in the current cell properties string.
 */
void ie_imp_table::setCellProp(const std::string & psProp, const std::string & psVal)
{
	UT_return_if_fail(m_pCurImpCell);
	m_pCurImpCell->setProp(psProp, psVal);
}

/*!
 * Return the value of a property of the current cell. 
 * This should be deleted when you've finished with it.
 */
std::string ie_imp_table::getCellPropVal(const std::string & psProp) const
{
	UT_return_val_if_fail(m_pCurImpCell,"");
	return m_pCurImpCell->getPropVal(psProp);
}

/*!
 * Return a pointer to the current cell
 */
ie_imp_cell * ie_imp_table::getCurCell(void) const
{
	return m_pCurImpCell;
}

/*!
 * set the current cell to the nth (iCell) location on the current row.
 */
void ie_imp_table::setNthCellOnThisRow(UT_sint32 iCell)
{
	setCellRowNthCell(m_iRowCounter, iCell);
}

/*!
 * This static function is used to compare CellX's for the qsort method of UT_Vector
\param vX1 pointer to a CellX value.
\param vX2 pointer to a second CellX value
*/
static UT_sint32 compareCellX(const void * vX1, const void * vX2)
{
	UT_sint32 x1 = *static_cast<const UT_sint32 *>(vX1);
	UT_sint32 x2 = *static_cast<const UT_sint32 *>(vX2);
	return x1 - x2;
}


/*!
 * Build a vector of all the cellx's and sort them
 */
void ie_imp_table::_buildCellXVector(void)
{
	m_vecCellX.clear();
	UT_sint32 i =0;
	ie_imp_cell * pCell = NULL;
	for(i=0; i< m_vecCells.getItemCount(); i++)
	{
		pCell = m_vecCells.getNthItem(i);
		UT_sint32 cellx = pCell->getCellX();
		if(m_vecCellX.findItem(cellx) < 0)
		{
			m_vecCellX.addItem(cellx);
		}
	}
	m_vecCellX.qsort(compareCellX);
}

/*!
 * Returns column number plus 1 of the cell.
 */
UT_sint32 ie_imp_table::getColNumber(ie_imp_cell * pImpCell) const
{
	UT_sint32 cellx = pImpCell->getCellX();
	UT_sint32 i =0;
	bool bFound = false;
	UT_sint32 iFound = 0;
	UT_sint32 iSub = 0;
	for(i=0; !bFound && (i< m_vecCellX.getItemCount()); i++)
	{
		UT_sint32 icellx = m_vecCellX.getNthItem(i);
		if(icellx == -1)
		{
			iSub++;
		}
		if(doCellXMatch(icellx,cellx))
		{
			bFound = true;
			iFound = i -iSub;
		}
	}
	if(bFound)
	{
		xxx_UT_DEBUGMSG(("SEVIOR: looking for cellx %d found at %d \n",cellx,i));
		return iFound+1;
	}
	return -1;
}

ie_imp_cell *  ie_imp_table::getCellAtRowColX(UT_sint32 iRow,UT_sint32 cellX) const
{
	UT_sint32 i = 0;
	ie_imp_cell * pCell = NULL;
	bool bfound = false;
	for(i=0; i< m_vecCells.getItemCount(); i++)
	{
		pCell = m_vecCells.getNthItem(i);
		UT_sint32 icellx = pCell->getCellX();
		if(doCellXMatch(icellx,cellX) && (pCell->getRow() == iRow))
		{
			bfound = true;
			break;
		}
	}
	if(bfound)
	{
		return pCell;
	}
	else
	{
		return NULL;
	}
}

/*!
 * This method bulds the table structure as required by abiword. ie It sets all the
 * left-attach, right-attach etc.
 */
void ie_imp_table::buildTableStructure(void)
{
//
// Start by building a vector of cellX's
//
	_buildCellXVector();
	UT_DEBUGMSG(("Building table structure \n"));
//
// Now construct the table structure.
//
	UT_sint32 i = 0;
	ie_imp_cell * pCell = NULL;
	//UT_sint32 cellx = 0;
	UT_sint32 curRow = 0;
	UT_sint32 iLeft =0;
	UT_sint32 iRight=0;
	UT_sint32 iTop=0;
	UT_sint32 iBot=0;

	for(i=0; i< m_vecCells.getItemCount(); i++)
	{
		bool bSkipThis = false;
		pCell = m_vecCells.getNthItem(i);
		//cellx = pCell->getCellX();
		if(i==0 || (pCell->getRow() > curRow))
		{
			curRow = pCell->getRow();
			iLeft =0;
		}
		if(pCell->isMergedAbove())
		{
//
// This cell is vertically merged. Advance the left pointer to the position after this cell.
//
			iRight = getColNumber(pCell);
			//UT_DEBUGMSG(("SEVIOR: This cell is meregd above!!!!!!!!! cellx %d iLeft %d \n",cellx,iLeft));
			bSkipThis = true;
		}
		if(pCell->isMergedLeft())
		{
//
// This cell is Horizontally merged. Advance the left pointer to the position after this cell. Increment iRight
//
			//UT_DEBUGMSG(("SEVIOR: This cell is meregd Left!!!!!!!!! cellx %d \n",cellx));
			bSkipThis = true;
		}
		else
		{
			if(!bSkipThis)
			{
				iRight = getColNumber(pCell);
				if(iRight <= iLeft)
				{
					iRight = iLeft+1;
				}
			}
		}
		iTop = curRow;
		if(pCell->isFirstVerticalMerged()  && !bSkipThis)
		{
			//
			// The cells below this are vertically merged with this. go hunting for the last one.
			//
			UT_sint32 newRow = curRow+1;
			ie_imp_cell * pNewCell = getCellAtRowColX(newRow,pCell->getCellX());
			/*UT_DEBUGMSG(("SEVIOR: This cell is first vertical mereged cell class %x cellx %d \n",pNewCell,cellx));
			if(pNewCell)
			{
				xxx_UT_DEBUGMSG(("SEVIOR: this cellx %d, found cellx %d, found row %d \n",cellx,pNewCell->getCellX(),pNewCell->getRow()));
			}*/
			while(pNewCell && (pNewCell->isMergedAbove()) )
			{
				newRow++;
				pNewCell = getCellAtRowColX(newRow,pCell->getCellX());
			}
			iBot = newRow;
			xxx_UT_DEBUGMSG(("SEVIOR: This cell bottom is %d \n",iBot));
		}
		else
		{
			iBot = iTop + 1;
		}
		//	
		// OK got what we need, set the left,right,top,bot attach's for the cell
		//
		if(!bSkipThis)
		{
			UT_ASSERT_HARMLESS(iRight>iLeft);
			UT_ASSERT_HARMLESS(iBot>iTop);
			pCell->setLeft(iLeft);
			pCell->setRight(iRight);
			pCell->setTop(iTop);
			pCell->setBot(iBot);
			xxx_UT_DEBUGMSG(("SEVIOR: i%d cellx %d Left %d Right %d top %d bot %d \n",i,pCell->getCellX(),iLeft,iRight,iTop,iBot));
		}
//
// Advance left attach to the right most cell.
//
		iLeft = iRight;
	}
}

/*!
 * Return the number of rows in the table
 */
UT_sint32  ie_imp_table::getNumRows(void) const
{
	UT_sint32 numrows = 0;
	UT_sint32 i =0;
	ie_imp_cell * pCell = NULL;
	for(i= m_vecCells.getItemCount() -1; i >=0 ; i--)
	{
		pCell = m_vecCells.getNthItem(i);
		if(pCell->getRow() > numrows)
		{
			numrows = pCell->getRow();
		}
	}
	numrows++;
	return numrows;
}

void ie_imp_table::CloseCell(void)
{
	m_bTableUsed = true;
}

/*!
 * This method removes the cells in the row given.
 */
void ie_imp_table::deleteRow(UT_sint32 row)
{
	m_iPosOnRow = 0;
	m_iCellXOnRow = 0;
	m_bNewRow = true;
	UT_sint32 i = 0;
	ie_imp_cell * pCell = NULL;
	UT_DEBUGMSG(("Deleting row %d \n",row));
	m_iPosOnRow = 0;
	for(i= m_vecCells.getItemCount() -1; i>=0; i--)
	{
		pCell = m_vecCells.getNthItem(i);
		UT_DEBUGMSG(("Look at Cell %d row %d cellx %d \n",i,pCell->getRow(),pCell->getCellX()));
		if(pCell->getRow() == row)
		{
			UT_DEBUGMSG(("Delete Cell pos %d on row %d \n",pCell->getLeft(),row));
			if(pCell->getCellSDH() != NULL)
			{
				pf_Frag_Strux* cellSDH = pCell->getCellSDH();
				pf_Frag_Strux* endCellSDH = m_pDoc->getEndCellStruxFromCellSDH(cellSDH);
				if(endCellSDH == NULL)
				{
					m_pDoc->deleteStruxNoUpdate(pCell->getCellSDH());
				}
				else
				{
					pf_Frag_Strux* sdh = cellSDH;
					pf_Frag_Strux* nextsdh = cellSDH;
					bool bDone = false;
					while(!bDone)
					{
						bDone = (sdh == endCellSDH);
						m_pDoc->getNextStrux(sdh,&nextsdh);
						//						m_pDoc->miniDump(sdh,4);
						m_pDoc->deleteStruxNoUpdate(sdh);
						sdh = nextsdh;
					}
				}
			}
			delete pCell;
			m_vecCells.deleteNthItem(i);
		}
	}
	if( 0 == m_vecCells.getItemCount())
	{
		m_bTableUsed = false;
	}
	//
	// look for extraneous unmatched endcell strux and delete it.
	//
	pf_Frag_Strux* sdhCell = m_pDoc->getLastStruxOfType(PTX_SectionCell);
	pf_Frag_Strux* sdhEndCell = m_pDoc->getLastStruxOfType(PTX_EndCell);
	if((sdhCell != NULL) && (sdhEndCell != NULL))
	{
		pf_Frag_Strux* sdhMyEnd= m_pDoc->getEndCellStruxFromCellSDH(sdhCell);
		if((sdhMyEnd != NULL) && (sdhEndCell != sdhMyEnd))
		{
			UT_DEBUGMSG(("Delete extraneous endCell strux 1 sdhEndCell %p sdhMyEnd %p \n",sdhEndCell,sdhMyEnd));
			m_pDoc->deleteStruxNoUpdate(sdhEndCell);
			m_pDoc->appendStrux(PTX_Block,NULL);
		}
	}
	else if( sdhCell == NULL)
	{
// 		if(sdhEndCell != NULL)
// 		{
// 			UT_DEBUGMSG(("Delete extraneous endCell strux 2 \n"));
// 			m_pDoc->deleteStruxNoUpdate(sdhEndCell);
// 			m_pDoc->appendStrux(PTX_Block,NULL);
//		}
	}
}
/*!
 * This method removes all cells on the same row as this. Can happen if
 * a document inserts a well defined row but puts in no \cell's
 */
void ie_imp_table::removeOnThisCellRow(ie_imp_cell * pImpCell)
{
	UT_sint32 row = pImpCell->getRow();
	UT_DEBUGMSG(("Doing a delete on Row %d left %d top %d \n",row,pImpCell->getLeft(),pImpCell->getTop()));
	deleteRow(row);
	//	UT_ASSERT_HARMLESS(0);
}

/*!
 * This method removes the current row.
 */
void ie_imp_table::removeCurrentRow(void)
{
	UT_DEBUGMSG(("About to delete current row number %d \n",m_iRowCounter));
	deleteRow(m_iRowCounter);
}

/*!
 * This method scans the vector of cells and removes cells and their sdh's if they
 * do no have cellx defined.
 */
void ie_imp_table::removeExtraneousCells(void)
{
	UT_sint32 i =0;
	ie_imp_cell * pCell = NULL;
	for(i= m_vecCells.getItemCount() -1; i >=0 ; i--)
	{
		pCell = m_vecCells.getNthItem(i);
		if(pCell->getCellX() == -1 && (pCell->getCellSDH() != NULL))
		{
			m_pDoc->deleteStruxNoUpdate(pCell->getCellSDH());
			delete pCell;
			m_vecCells.deleteNthItem(i);
		}
	}
}


/*!
 * This method removes all the struxes placed in the document. It is called if the table
 * is never actually used. 
 */
void ie_imp_table::_removeAllStruxes(void)
{
	UT_sint32 i =0;
	ie_imp_cell * pCell = NULL;
	for(i= m_vecCells.getItemCount() -1; i >=0 ; i--)
	{
		pCell = m_vecCells.getNthItem(i);
		if(pCell->getCellSDH())
		{
			UT_DEBUGMSG(("SEVIOR: Removing cell strux %p from PT \n",pCell->getCellSDH())); 
			m_pDoc->deleteStruxNoUpdate(pCell->getCellSDH());
		}
	}
	if(m_tableSDH)
	{
		UT_DEBUGMSG(("SEVIOR: Removing table strux %p from PT \n",m_tableSDH)); 
		m_pDoc->deleteStruxNoUpdate(m_tableSDH);
	}
}

/*!
 * Remove all the cells in row identified by row from the table vector of
 * cells. Do not delete the cell classes, they will be usde later.
 */ 
bool ie_imp_table::removeRow(UT_sint32 row)
{
	UT_sint32 i=0;
	UT_sint32 iFound =0;
	bool bFound = false;
	ie_imp_cell * pCell = NULL;
	for(i=0; !bFound &&  (i< m_vecCells.getItemCount()); i++)
	{
		pCell = m_vecCells.getNthItem(i);
		bFound = (pCell->getRow() == row);
		iFound = i;
	}
	if(!bFound)
	{
		return false;
	}
	i = iFound;
	while(pCell != NULL && i < m_vecCells.getItemCount())
	{
		xxx_UT_DEBUGMSG(("SEVIOR: Removing cell %p from row %d \n",pCell,row));
		m_vecCells.deleteNthItem(i);
		if(i<m_vecCells.getItemCount())
		{
			pCell = m_vecCells.getNthItem(i);
			if(pCell->getRow() != row)
			{
				pCell = NULL;
			}
		}
	}
	return true;
}

/*!
 * Append the row of cells given by the vector pVecRowOfCells to the current
 * table, adjusting hte table pointer and row in the cell classes
 */
void ie_imp_table::appendRow(UT_GenericVector<ie_imp_cell*>* pVecRowOfCells)
{
	UT_sint32 iNew =0;
	if(m_iRowCounter > 0)
	{
		m_iRowCounter++;
		iNew = m_iRowCounter;
	}
	UT_sint32 i =0;
	ie_imp_cell * pCell = NULL;
	for(i=0; i <pVecRowOfCells->getItemCount(); i++)
	{
		pCell = pVecRowOfCells->getNthItem(i);
		pCell->setImpTable(this);
		pCell->setRow(iNew);
		m_vecCells.addItem(pCell);
	}
}

/*!
 * This method scans the vector of cell looking for the nth cell on the current row
 * Return null if cell is not present.
*/
ie_imp_cell * ie_imp_table::getNthCellOnRow(UT_sint32 iCell) const
{
	ie_imp_cell * pFoundCell = NULL;
	ie_imp_cell * pCell = NULL;
	UT_sint32 iCellOnRow =0;
	UT_sint32 i=0;
	bool bFound = false;
	for(i=0; !bFound &&  (i< m_vecCells.getItemCount()); i++)
	{
		pCell = m_vecCells.getNthItem(i);
		if(pCell->getRow() == m_iRowCounter)
		{
			if(iCellOnRow == iCell)
			{
				bFound = true;
				pFoundCell = pCell;
			}
			else
			{
				iCellOnRow++;
			}
		}
	}
	return pFoundCell;
}

//---------------------------------------------------------------------------------------//

/*!
 * Class to hold a stack of tables for nested tables.
 */
ie_imp_table_control::ie_imp_table_control(PD_Document * pDoc):
	m_pDoc(pDoc)
{
	m_sLastTable.push(NULL);
}


ie_imp_table_control::~ie_imp_table_control(void)
{
	while(m_sLastTable.size() > 1)
	{
		ie_imp_table * pT = m_sLastTable.top();
		m_sLastTable.pop();
		if(pT->wasTableUsed())
		{
//			pT->removeExtraneousCells();
			pT->buildTableStructure();
			pT->writeTablePropsInDoc();
			pT->writeAllCellPropsInDoc();
		}			
		UT_DEBUGMSG(("SEVIOR: Deleting table %p \n",pT));
		delete pT;
	}
}

UT_sint32 ie_imp_table_control::getNestDepth(void)
{
	return m_sLastTable.size() - 1;
}

void ie_imp_table_control::OpenTable(void)
{
	m_sLastTable.push(new ie_imp_table(m_pDoc));
}


UT_sint32 ie_imp_table_control::OpenCell(void)
{
	ie_imp_table * pT = m_sLastTable.top();
	return pT->OpenCell();
}

void ie_imp_table_control::CloseTable(void)
{
	ie_imp_table * pT = m_sLastTable.top();
	m_sLastTable.pop();
	if(pT->wasTableUsed())
	{
//		pT->removeExtraneousCells();
		pT->buildTableStructure();
		pT->writeTablePropsInDoc();
		pT->writeAllCellPropsInDoc();
	}
	delete pT;
}


void ie_imp_table_control::CloseCell(void)
{
	ie_imp_table * pT = m_sLastTable.top();
	pT->CloseCell();
}

ie_imp_table *  ie_imp_table_control::getTable(void) const
{
	ie_imp_table * pT = m_sLastTable.top();
	return pT;
}

bool ie_imp_table_control::NewRow(void)
{
	UT_sint32 val = getTable()->NewRow();
	if(val == 0)
	{
		return true;
	}
	if(val == -1)
	{
		return false;
	}
//
// If we're here the row of cells has totally different cellx structure
// to the previous. So slice off this row, close the table and open a new 
// table with this row as the first row.
//
	UT_GenericVector<ie_imp_cell*> vecRow;
	vecRow.clear();
	UT_sint32 row = getTable()->getRow();
    UT_ASSERT_HARMLESS(row>0);
	bool bres = true;
	bres = getTable()->getVecOfCellsOnRow(row, &vecRow);
	if(!bres)
	{
		return bres;
	}
	UT_DEBUGMSG(("Number of cells on row %d \n",vecRow.getItemCount()));
	//	UT_ASSERT(0);
//
// Got last row, now remove it.
//
	getTable()->removeRow(row);
//
// Close the old table.
//
	UT_sint32 i =0;
	pf_Frag_Strux* sdhCell = NULL;
	ie_imp_cell * pCell = NULL;
	bool bFound = false;
	bool bAuto = false;
	for(i=0; i < vecRow.getItemCount() && !bFound;i++)
	{
		pCell = vecRow.getNthItem(i);
		if(pCell->getCellSDH())
		{
			bFound = true;
			break;
		}
	}
	if(bFound)
	{
		sdhCell = pCell->getCellSDH();
		m_pDoc->insertStruxNoUpdateBefore(sdhCell,PTX_EndTable,NULL);
		bAuto = getTable()->isAutoFit();
		CloseTable();
	}
	else
	{
		UT_DEBUGMSG(("Not a single valid sdh found on last row!!!! \n"));
		UT_DEBUGMSG(("We're in deep shit!!!! \n"));
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		return false;
	}

//
// Now create a new table with the old last row and the first new row.
//
	m_pDoc->insertStruxNoUpdateBefore(sdhCell,PTX_SectionTable,NULL);
	OpenTable();
	getTable()->setAutoFit(bAuto);
	getTable()->appendRow(&vecRow);
	getTable()->NewRow();
	pf_Frag_Strux* sdh = m_pDoc->getLastStruxOfType(PTX_SectionTable);
	getTable()->setTableSDH(sdh);
	getTable()->CloseCell(); // This just sets the table used flag!
//	UT_ASSERT_HARMLESS(0);
	return true;
}

#ifdef USE_IE_IMP_TABLEHELPER

CellHelper::CellHelper () :
	m_style(""),
	m_pfsCell(0),
	m_bottom(0),
	m_left(0),
	m_right(0),
	m_top(0),
	m_rowspan(0),
	m_colspan(0),
	m_next(0),
	m_tzone(tz_body),
	m_sCellProps("")
{
	// 
}

void CellHelper::setProp(const char * szProp, const std::string & sVal)
{
	std::string psProp = szProp;
	UT_std_string_setProperty(m_sCellProps, psProp, sVal);
}


IE_Imp_TableHelper::IE_Imp_TableHelper (PD_Document * pDocument, pf_Frag_Strux * pfsInsertionPoint, const char * style) :
	m_pDocument(pDocument),
	m_style_table(style),
	m_style_tzone(""),
	m_style(""),
	m_pfsInsertionPoint(pfsInsertionPoint),
	m_pfsTableStart(0),
	m_pfsTableEnd(pfsInsertionPoint),
	m_pfsCellPoint(NULL),
	m_rows(0),
	m_rows_head(0),
	m_rows_head_max(0),
	m_rows_foot(0),
	m_rows_foot_max(0),
	m_rows_body(0),
	m_rows_body_max(0),
	m_cols(0),
	m_cols_max(0),
	m_col_next(0),
	m_row_next(0),
	m_current(0),
	m_tzone(tz_body),
	m_bBlockInsertedForCell(false),
	m_bCaptionOn(false)
{
	m_thead.clear();
	m_tfoot.clear();
	m_tbody.clear();
	UT_DEBUGMSG(("TableHelper created document = %p \n",m_pDocument)); 
}

IE_Imp_TableHelper::~IE_Imp_TableHelper ()
{
	if(m_thead.getItemCount() > 0)
	{
		UT_VECTOR_PURGEALL(CellHelper *, m_thead);
	}
	if(m_tfoot.getItemCount() > 0)
	{	
		UT_VECTOR_PURGEALL(CellHelper *, m_tfoot);
	}
	if(m_tbody.getItemCount() > 0)
	{	
		UT_VECTOR_PURGEALL(CellHelper *, m_tbody);
	}
}

bool IE_Imp_TableHelper::tableStart (void)
{
	if(m_pfsInsertionPoint == NULL)
	{
		if(m_style.size() == 0)
		{
			if (!getDoc()->appendStrux (PTX_SectionTable, 0))
				return false;
		}
		else
		{
			const gchar * atts[3] = {NULL,NULL,NULL};
			atts[0] = "props";
			atts[1] = m_style.c_str();
			if (!getDoc()->appendStrux (PTX_SectionTable,atts))
				return false;
		}
		m_pfsTableStart = static_cast<pf_Frag_Strux *>(getDoc()->getLastFrag());
		getDoc()->appendStrux(PTX_EndTable,NULL);
		m_pfsTableEnd = static_cast<pf_Frag_Strux *>(getDoc()->getLastFrag());
		m_pfsInsertionPoint = m_pfsTableEnd;
		m_pfsCellPoint = m_pfsInsertionPoint;
	}
	else
	{
		pf_Frag * pf = static_cast<pf_Frag *>(m_pfsInsertionPoint);
		if(m_style.size() == 0)
		{
			getDoc()->insertStruxBeforeFrag(pf,PTX_SectionTable,NULL);
		}
		else
		{
			const gchar * atts[3] = {NULL,NULL,NULL};
			atts[0] = "props";
			atts[1] = m_style.c_str();
			getDoc()->insertStruxBeforeFrag(pf,PTX_SectionTable,atts);
		}
		getDoc()->insertStruxBeforeFrag(pf,PTX_EndTable,NULL);
		pf_Frag_Strux* sdhEnd = NULL;
		getDoc()->getPrevStruxOfType(static_cast<pf_Frag_Strux *>(pf),PTX_EndTable,&sdhEnd);
		m_pfsTableEnd = sdhEnd;
		m_pfsInsertionPoint = m_pfsTableEnd;
		m_pfsCellPoint = m_pfsInsertionPoint;
	}
	return tbodyStart ();
}

bool IE_Imp_TableHelper::tableEnd ()
{
	UT_DEBUGMSG(("Doing end table \n"));
	if (!tdPending ())
		return false;

	// TODO: unset frag - & other clean-up?
	m_pfsTableEnd = NULL;
	m_pfsInsertionPoint = NULL;
	m_pfsCellPoint = NULL;
	return true;
}

bool IE_Imp_TableHelper::theadStart (const char * style)
{
	if (!tdPending ())
		return false;

	m_tzone = tz_head;
	m_rows_head = m_row_next;

	m_col_next = 0;
	if (style)
		m_style_tzone = style;
	else
		m_style_tzone = "";

	return true;
}

bool IE_Imp_TableHelper::tfootStart (const char * style)
{
	if (!tdPending ())
		return false;

	m_tzone = tz_foot;
	m_rows_foot = m_row_next;

	m_col_next = 0;
	if (style)
		m_style_tzone = style;
	else
		m_style_tzone = "";

	return true;
}

bool IE_Imp_TableHelper::tbodyStart (const char * style)
{
	if (!tdPending ())
		return false;

	m_tzone = tz_body;
	m_rows_body = m_row_next;

	m_col_next = 0;

	if (style)
		m_style_tzone = style;
	else
		m_style_tzone = "";

	return true;
}

bool IE_Imp_TableHelper::trStart (const char * style)
{
	if (m_current)
		if (!trEnd ())
			return false;
	if(m_bCaptionOn)
		{
			UT_DEBUGMSG(("Row start with caption on \n"));
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			m_bCaptionOn = false;
		}
	// TODO ??

	if (style)
		m_style = style;
	else
		m_style = "";

	return true;
}

bool IE_Imp_TableHelper::trEnd ()
{
	m_row_next++;
	if(m_row_next == 1)
		{
			m_cols_max = m_col_next;
		}
	else if(m_col_next > m_cols_max)
		{
			UT_sint32 extra = m_col_next - m_cols_max;
			padAllRowsWithCells(m_thead,extra);
			padAllRowsWithCells(m_tfoot,extra);
			padAllRowsWithCells(m_tbody,extra);
		}
	else if(m_col_next < m_cols_max)
		{
			UT_sint32 extra = m_cols_max - m_col_next;
			if(m_tzone == tz_head)
				{
					padRowWithCells(m_thead,m_row_next-1,extra);
				}
			else if(m_tzone == tz_foot)
				{
					padRowWithCells(m_tfoot,m_row_next-1,extra);
				}
			else if(m_tzone == tz_body)
				{
					padRowWithCells(m_tbody,m_row_next-1,extra);
				}
		}
	m_col_next = 0;
	CellHelper * pCell = NULL;
	switch (m_tzone)
		{
		case tz_head:
			m_rows_head_max = m_rows_head - m_row_next;
		    pCell = getCellAtRowCol(m_thead,m_row_next, m_col_next);
			break;
		case tz_foot:
			m_rows_foot_max = m_rows_foot - m_row_next;
		    pCell = getCellAtRowCol(m_tfoot,m_row_next, m_col_next);
			break;
		case tz_body:
			m_rows_body_max = m_rows_body - m_row_next;
		    pCell = getCellAtRowCol(m_tbody,m_row_next, m_col_next);
			break;
		}
	if(pCell != NULL)
		{
			m_col_next = pCell->m_right;
		}
	return true;
}

void IE_Imp_TableHelper::padAllRowsWithCells(UT_GenericVector<CellHelper *> & vecCells,UT_sint32 extra)
{
	UT_sint32 LastRow = 0;
	if(vecCells.getItemCount() == 0)
		{
			return;
		}
	CellHelper * pCell = vecCells.getNthItem(0);
	UT_sint32 FirstRow = pCell->m_top;
	pCell = static_cast<CellHelper *>(vecCells.getNthItem(vecCells.getItemCount()-1));
	LastRow = pCell->m_top;
	UT_sint32 i = 0;
	for(i=FirstRow; i<=LastRow; i++)
		{
			padRowWithCells(vecCells,i,extra);
		}
}

/*!
 * Pad out the supplied row with the requested number of cells at the end of 
 * the vector.
 */
void IE_Imp_TableHelper::padRowWithCells(UT_GenericVector<CellHelper *>& vecCells,UT_sint32 row, UT_sint32 extra)
{
	CellHelper * pCell = NULL;
	UT_sint32 i =0;
	bool bFoundRow = false;
	for(i= vecCells.getItemCount()-1; i>=0;i--)
		{
			pCell = vecCells.getNthItem(i);
			if(pCell->m_top == row)
				{
					bFoundRow = true;
					break;
				}
		}
	if(!bFoundRow)
		{
			return;
		}
	UT_sint32 j = 0;
	CellHelper * pNext = pCell->m_next;
	CellHelper * pOldCurrent = m_current;
	m_current = pCell;
	TableZone oldTz = m_tzone;
	m_tzone = pCell->m_tzone;
	pf_Frag_Strux * pfsIns = NULL;
	if(pNext == NULL)
		{
			pfsIns = m_pfsCellPoint;
		}
	else
		{
			pfsIns = pNext->m_pfsCell;
		}
	for(j=0; j < extra; j++)
		{
			//
			// Add the cell.
			//
			tdStart(1,1,NULL,pfsIns);
		}
	m_current = pOldCurrent;
	m_tzone = oldTz;
}

/*!
 * Get a cellHelper at the specified row and column. Return NULL if none found.
 * Optimized to find or not find cells near the end of the specifed vector.
 */
CellHelper * IE_Imp_TableHelper::getCellAtRowCol(UT_GenericVector<CellHelper *> & vecCells, UT_sint32 row, UT_sint32 col)
{
	CellHelper * pCell = NULL;
	UT_sint32 i =0;
	for(i=vecCells.getItemCount()-1; i>=0;i--)
		{
			pCell = vecCells.getNthItem(i);
			if((pCell->m_left <= col) && (pCell->m_right > col) && (pCell->m_top == row))
				{
					return pCell;
				}
			else if( (row > pCell->m_top) && (row < pCell->m_bottom) && (pCell->m_left <= col) && (pCell->m_right > col))
				{
					return pCell;
				}
			else if( (row > pCell->m_top) && (row > pCell->m_bottom) && (pCell->m_left <= col) && (pCell->m_right > col))
				{
					return NULL;
				}
		}
	return NULL;
}

/*!
 * Handle </td> tag. In there is no content, write a blank block
 */
bool IE_Imp_TableHelper::tdEnd(void)
{
	if(m_bBlockInsertedForCell)
		{
			return true;
		}
	pf_Frag * pf = static_cast<pf_Frag *>(m_pfsInsertionPoint);
	getDoc()->insertStruxBeforeFrag(pf,PTX_Block,NULL);
	return true;
}

/*!
 * Handle the <td> tag. Insert a cell.
 */
 bool IE_Imp_TableHelper::tdStart (UT_sint32 rowspan, UT_sint32 colspan, const char * style, pf_Frag_Strux * pfsThis)
{
	CellHelper * pCell = new CellHelper();
	CellHelper * pPrev = m_current;
	if(m_current)
		{
			m_current->m_next = pCell;
		}
	m_current = pCell;
    m_current->m_rowspan = rowspan;
	m_current->m_colspan = colspan;
	m_current->m_style = style;
	m_current->m_left = m_col_next;
	m_current->m_right = m_col_next+colspan;
	m_current->m_top = m_row_next;
	m_current->m_bottom = m_row_next+rowspan;
	m_current->m_sCellProps = "";
	m_current->m_tzone = m_tzone;
	UT_GenericVector<CellHelper *>* pVecCells = NULL;
	pCell = NULL;
	if(true)
		{
			if(m_tzone == tz_head)
				{
					if(pfsThis == NULL)
						pCell = getCellAtRowCol(m_thead,m_row_next,m_col_next+colspan);
					pVecCells = & m_thead;
				}
			else if(m_tzone == tz_foot)
				{
					if(pfsThis == NULL)
						pCell = getCellAtRowCol(m_tfoot,m_row_next,m_col_next+colspan);
					pVecCells = & m_tfoot;
				}
			else if(m_tzone == tz_body)
				{
					if(pfsThis == NULL)
						pCell = getCellAtRowCol(m_tbody,m_row_next,m_col_next+colspan);
					pVecCells = & m_tbody;
				}
		}
	if(pCell == NULL)
		{
			m_col_next += colspan;
		}
	else
		{
			m_col_next = pCell->m_right;
			
		}
		
	m_current->setProp("top-attach", UT_std_string_sprintf("%d",m_current->m_top));
	m_current->setProp("bot-attach", UT_std_string_sprintf("%d",m_current->m_bottom));
	m_current->setProp("left-attach", UT_std_string_sprintf("%d",m_current->m_left));
	m_current->setProp("right-attach", UT_std_string_sprintf("%d",m_current->m_right));

	const gchar * atts[3] = {"props",NULL,NULL};
	atts[1] = m_current->m_sCellProps.c_str();
	UT_DEBUGMSG(("Props for td are : %s \n",atts[1]));
	pf_Frag * pf = NULL;
	if(pfsThis == NULL)
		{
		   pf = static_cast<pf_Frag *>(m_pfsCellPoint);
		}
	else
		{
		   pf = static_cast<pf_Frag *>(pfsThis);
		}
	getDoc()->insertStruxBeforeFrag(pf,PTX_SectionCell,atts,NULL);
	pf_Frag_Strux* sdhCell = NULL;
	getDoc()->getPrevStruxOfType(static_cast<pf_Frag_Strux *>(pf),PTX_SectionCell,&sdhCell);
	m_current->m_pfsCell = sdhCell;
	if(pfsThis == NULL)
		{
			getDoc()->insertStruxBeforeFrag(pf,PTX_EndCell,NULL);
			m_bBlockInsertedForCell = false;
			pf_Frag_Strux* sdhIns = NULL;
			getDoc()->getPrevStruxOfType(static_cast<pf_Frag_Strux *>(pf),PTX_EndCell,&sdhIns);
			m_pfsInsertionPoint = sdhIns;
		}
	else
		{
			getDoc()->insertStruxBeforeFrag(pf,PTX_Block,NULL);
			getDoc()->insertStruxBeforeFrag(pf,PTX_EndCell,NULL);
			m_bBlockInsertedForCell = true;
		}
	if(pPrev == NULL)
		{
			pVecCells->addItem(m_current);
			return true;
		}
	UT_sint32 iPrev = pVecCells->findItem(pPrev);
	if(iPrev < 0)
		{
			pVecCells->addItem(m_current);
			return false;
		}
	if(iPrev == pVecCells->getItemCount())
		{
			pVecCells->addItem(m_current);
			return true;
		}
	pVecCells->insertItemAt(m_current, iPrev+1);
	return true;
}


bool IE_Imp_TableHelper::tdPending ()
{
	// create any cells that are still awaiting creation
	return true;
}


bool IE_Imp_TableHelper::Block (PTStruxType /*pts*/, const gchar ** attributes)
{
	pf_Frag * pf = NULL;
	if(m_bCaptionOn)
		{
			pf = static_cast<pf_Frag *>(m_pfsTableStart);
		}
	else
		{
			pf = static_cast<pf_Frag *>(m_pfsInsertionPoint);
		}
	getDoc()->insertStruxBeforeFrag(pf, PTX_Block, attributes);
	m_bBlockInsertedForCell = true;
	return true;
}

bool IE_Imp_TableHelper::BlockFormat (const gchar ** attributes)
{
	if(!m_bBlockInsertedForCell)
		{
			Block(PTX_Block,NULL);
		}
	pf_Frag_Strux * pfs = NULL;
	if(m_bCaptionOn)
		{
			pfs = m_pfsTableStart;
		}
	else
		{
			pfs = m_pfsInsertionPoint;
		}
	pf_Frag_Strux* sdh = pfs;
	getDoc()->getPrevStruxOfType(pfs,PTX_Block,&sdh);
	getDoc()->changeStruxFormatNoUpdate(PTC_AddFmt,sdh,attributes);
	return true;
}

bool IE_Imp_TableHelper::Inline (const UT_UCSChar * ucs4_str, UT_sint32 length)
{
	if(!m_bBlockInsertedForCell)
		{
			Block(PTX_Block,NULL);
		}
	pf_Frag * pf = NULL;
	if(m_bCaptionOn)
		{
			pf = static_cast<pf_Frag *>(m_pfsTableStart);
		}			
	else
		{
			pf = static_cast<pf_Frag *>(m_pfsInsertionPoint);
		}
#if DEBUG
#if 0
	UT_uint32 ii = 0;
	std::string sStr;
	for(ii=0; ii<(length);ii++)
	{
		sStr += static_cast<const char>(ucs4_str[ii]);
	}
	UT_DEBUGMSG(("Append span in cell %s \n",sStr.c_str()));
#endif
#endif

	UT_DEBUGMSG(("Insert Text of length %d in cell \n",length));
	getDoc()->insertSpanBeforeFrag(pf, ucs4_str, length);
	return true;
}

bool IE_Imp_TableHelper::setCaptionOn(void)
{
	if(m_bCaptionOn)
		{
			UT_DEBUGMSG(("Attempt to open a caption without closing the last \n"));
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			return false;
		}
	m_bCaptionOn = true;
	Block(PTX_Block,NULL);
	return true;
}


bool IE_Imp_TableHelper::setCaptionOff(void)
{
	if(!m_bCaptionOn)
		{
			UT_DEBUGMSG(("Attempt to close a caption without openning \n"));
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			return false;
		}
	m_bCaptionOn = false;
	return true;
}


bool IE_Imp_TableHelper::InlineFormat (const gchar ** attributes)
{
	if(!m_bBlockInsertedForCell)
		{
			Block(PTX_Block,NULL);
		}
	pf_Frag * pf = NULL;
	if(m_bCaptionOn)
		{
			pf = static_cast<pf_Frag *>(m_pfsTableStart);
		}			
	else
		{
			pf = static_cast<pf_Frag *>(m_pfsInsertionPoint);
		}
	getDoc()->insertFmtMarkBeforeFrag(pf, attributes);
	return true;
}

bool IE_Imp_TableHelper::Object (PTObjectType pto, const gchar ** attributes)
{
	if(!m_bBlockInsertedForCell)
		{
			Block(PTX_Block,NULL);
		}
	pf_Frag * pf = NULL;
	if(m_bCaptionOn)
		{
			pf = static_cast<pf_Frag *>(m_pfsTableStart);
		}			
	else
		{
			pf = static_cast<pf_Frag *>(m_pfsInsertionPoint);
		}
	getDoc()->insertObjectBeforeFrag(pf, pto,attributes);
	return true;
}

IE_Imp_TableHelperStack::IE_Imp_TableHelperStack (void) :
	m_pDocument(NULL),
	m_count(0),
	m_max(0),
	m_stack(0)
{
	UT_DEBUGMSG(("TableHelperStack created document = %p \n",m_pDocument)); 
}

IE_Imp_TableHelperStack::~IE_Imp_TableHelperStack ()
{
	if (m_stack)
		{
			clear ();
			g_free (m_stack);
		}
}

void IE_Imp_TableHelperStack::clear ()
{
	for (UT_sint32 i = 1; i <= m_count; i++)
		delete m_stack[i];

	m_count = 0;
}

bool IE_Imp_TableHelperStack::push (const char * style)
{
	if (m_stack == 0)
		{
			m_stack = reinterpret_cast<IE_Imp_TableHelper **>(g_try_malloc (16 * sizeof (IE_Imp_TableHelper *)));
			if (m_stack == 0)
				return false;
			m_count = 0;
			m_max = 16;
		}
	else if (m_count == m_max)
		{
			IE_Imp_TableHelper ** more = 0;
			more = reinterpret_cast<IE_Imp_TableHelper **>(g_try_realloc (m_stack, (m_max + 16) * sizeof (IE_Imp_TableHelper *)));
			if (more == 0)
				return false;
			m_max += 16;
			m_stack = more;
		}

	IE_Imp_TableHelper * th = 0;

	// TODO: not sure this needs to happen...
	try
		{
			IE_Imp_TableHelper * prev = top();
			pf_Frag_Strux * pfs = NULL;
			if(prev)
			{
				pfs = prev->getInsertionPoint ();
			}
			th = new IE_Imp_TableHelper(m_pDocument,pfs,style);
		}
    catch(...)
		{
			th = 0;
		}

	if (th == 0)
		return false;
	m_count++;
	m_stack[m_count] = th;

	return true;
}

bool IE_Imp_TableHelperStack::pop ()
{
	if (!m_count)
		return false;

	delete m_stack[m_count];
	m_count--;
	return true;
}

bool IE_Imp_TableHelperStack::tableStart (PD_Document * pDoc, const char * style)
{
	m_pDocument = pDoc;
	bool okay = push (style);
	IE_Imp_TableHelper * th = top ();
	th->tableStart();

	return okay;
}
IE_Imp_TableHelper * IE_Imp_TableHelperStack::top(void) const
{
	if(m_count == 0)
		{
			return NULL;
		}
	return m_stack[m_count];
}

bool IE_Imp_TableHelperStack::tableEnd ()
{
	IE_Imp_TableHelper * th = top ();
	if (th == 0)
		return false;

	bool okay = th->tableEnd ();

	pop ();

	// TODO ??

	return okay;
}

bool IE_Imp_TableHelperStack::theadStart (const char * style)
{
	IE_Imp_TableHelper * th = top ();
	if (th == 0)
		return false;

	return th->theadStart (style);
}

bool IE_Imp_TableHelperStack::tfootStart (const char * style)
{
	IE_Imp_TableHelper * th = top ();
	if (th == 0)
		return false;

	return th->tfootStart (style);
}

bool IE_Imp_TableHelperStack::tbodyStart (const char * style)
{
	IE_Imp_TableHelper * th = top ();
	if (th == 0)
		return false;

	return th->tbodyStart (style);
}

bool IE_Imp_TableHelperStack::trStart (const char * style)
{
	IE_Imp_TableHelper * th = top ();
	if (th == 0)
		return false;

	return th->trStart (style);
}

bool IE_Imp_TableHelperStack::tdStart (UT_sint32 rowspan, UT_sint32 colspan, const char * style)
{
	IE_Imp_TableHelper * th = top ();
	if (th == 0)
		return false;

	return th->tdStart (rowspan, colspan, style,NULL);
}


bool IE_Imp_TableHelperStack::tdEnd (void)
{
	IE_Imp_TableHelper * th = top ();
	if (th == 0)
		return false;

	return th->tdEnd();
}

bool IE_Imp_TableHelperStack::Block (PTStruxType pts, const gchar ** attributes)
{
	IE_Imp_TableHelper * th = top ();
	if (th)
		return th->Block (pts, attributes);

	UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);

	return false;
}

bool IE_Imp_TableHelperStack::BlockFormat (const gchar ** attributes)
{
	IE_Imp_TableHelper * th = top ();
	if (th)
		return th->BlockFormat (attributes);
	return false;
}

bool IE_Imp_TableHelperStack::Inline (const UT_UCSChar * ucs4_str, UT_sint32 length)
{
	IE_Imp_TableHelper * th = top ();
	if (th)
		return th->Inline (ucs4_str, length);


	UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);

	return false;
}

bool IE_Imp_TableHelperStack::InlineFormat (const gchar ** attributes)
{
	IE_Imp_TableHelper * th = top ();
	if (th)
		return th->InlineFormat (attributes);

	UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);

	return false;
}

bool IE_Imp_TableHelperStack::Object (PTObjectType pto, const gchar ** attributes)
{
	IE_Imp_TableHelper * th = top ();
	if (th)
		return th->Object (pto, attributes);

	UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);

	return false;
}


bool IE_Imp_TableHelperStack::setCaptionOn(void)
{
	IE_Imp_TableHelper * th = top ();
	if (th)
		return th->setCaptionOn();

	UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);

	return false;
}


bool IE_Imp_TableHelperStack::setCaptionOff(void)
{
	IE_Imp_TableHelper * th = top ();
	if (th)
		return th->setCaptionOff();

	UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);

	return false;
}

#endif /* USE_IE_IMP_TABLEHELPER */
