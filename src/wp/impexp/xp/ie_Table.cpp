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
	m_pDoc->getRowsColsFromTableSDH(m_TableSDH, &m_iNumRows, &m_iNumCols);
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
 * Signal close of cell from endCell strux
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
 * RTF expects an unnested table to have a nest depth of 0. Since we push NULL
 * at the start we have to subtract this from the depth calculation.
 */
UT_sint32 ie_Table::getNestDepth(void)
{
	return m_sLastTable.getDepth() -1;
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
/*!
 * Set the cell api on the top of the stack to that at location (row,col)
 * If there is no cell at the (row,col) the cellApi is not changed. 
*/
void ie_Table::setCellRowCol(UT_sint32 row, UT_sint32 col)
{
	ie_PartTable * pPT = NULL;
	m_sLastTable.viewTop((void **) &pPT);
	UT_return_if_fail(pPT);
	PL_StruxDocHandle cellSDH = m_pDoc->getCellSDHFromRowCol(pPT->getTableSDH(),row,col);
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
	m_bFirstVertical(false)
{
	m_sCellProps.clear();
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
UT_sint32 ie_imp_cell::getCellX(void)
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
	setProp("left-attach", UT_String_sprintf("%d",iLeft));
}

/*!
 * Get the left attach for the cell
 */
UT_sint32 ie_imp_cell::getLeft(void)
{
	return m_iLeft;
}

/*!
 * set Right attach for this this cell..
 */
void ie_imp_cell::setRight(UT_sint32 iRight)
{
	m_iRight = iRight;
	UT_String spRight("right-attach");
	setProp("right-attach", UT_String_sprintf("%d",iRight));
}

/*!
 * Get the right attach for the cell
 */
UT_sint32 ie_imp_cell::getRight(void)
{
	return m_iRight;
}

/*!
 * set top attach for this this cell..
 */
void ie_imp_cell::setTop(UT_sint32 iTop)
{
	m_iTop = iTop;
	setProp("top-attach", UT_String_sprintf("%d",iTop));
}

/*!
 * Get the top attach for the cell
 */
UT_sint32 ie_imp_cell::getTop(void)
{
	return m_iTop;
}

/*!
 * set bottom attach for this this cell..
 */
void ie_imp_cell::setBot(UT_sint32 iBot)
{
	m_iBot = iBot;
	setProp("bot-attach", UT_String_sprintf("%d",iBot));
}

/*!
 * Get the bottom attach for the cell
 */
UT_sint32 ie_imp_cell::getBot(void)
{
	return m_iBot;
}

/*!
 * Get the cell SDH for this cell.
 */
PL_StruxDocHandle ie_imp_cell::getCellSDH(void)
{
	return m_cellSDH;
}

/*!
 * Set Cell SDH 
 */
void ie_imp_cell::setCellSDH(PL_StruxDocHandle cellSDH)
{
	m_cellSDH = cellSDH;
}

/*!
 * Write all the properties of this cell to the piecetable without throwing a changerecord
 */
void ie_imp_cell::writeCellPropsInDoc(void)
{
	UT_return_if_fail(m_cellSDH);
	m_pDoc->changeStruxAttsNoUpdate(m_cellSDH,"props",m_sCellProps.c_str());
}

/*!
 * Return a pointer to the import cell class above this one.
 */
ie_imp_cell * ie_imp_cell::getCellAbove(void)
{
	return NULL;
}

/*!
 * Return a pointer to the import cell class below this one.
 */
ie_imp_cell * ie_imp_cell::getCellBelow(void)
{
	return NULL;
}

/*!
 * Return a pointer to the import cell class right of this one.
 */
ie_imp_cell * ie_imp_cell::getCellRight(void)
{
	return NULL;
}

/*!
 * Return a pointer to the import cell class left of this one.
 */
ie_imp_cell * ie_imp_cell::getCellLeft(void)
{
	return m_pCellLeft;
}

/*!
 * set a property of this cell.
 */
void ie_imp_cell::setProp(const UT_String & psProp, const UT_String & psVal)
{
	UT_String_setProperty(m_sCellProps, psProp, psVal);
}

/*!
 * set a property of this cell.
 */
void ie_imp_cell::setProp(const char * szProp, const char * szVal)
{
	UT_String psProp = szProp;
	UT_String psVal = szVal;
	UT_String_setProperty(m_sCellProps, psProp, psVal);
}

/*!
 * Return the value of a property of this cell. This should be deleted when you've finished with it.
 */
UT_String ie_imp_cell::getPropVal(const UT_String & psProp)
{
	return UT_String_getPropVal(m_sCellProps, psProp);
}


/*!
 * Return the value of a property of this cell. This should be deleted when you've finished with it.
 */
UT_String ie_imp_cell::getPropVal(const char * szProp)
{
	UT_String psProp = szProp;
	return UT_String_getPropVal(m_sCellProps, psProp);
}

/*!
 * Class for handling import of tables. Built for RTF but might be useful elsewhere.
 */
ie_imp_table::ie_imp_table(PD_Document * pDoc):
	m_pDoc(pDoc),
	m_tableSDH(NULL),
	m_pCurImpCell(NULL),
	m_iRowCounter(0),
	m_bAutoFit(true),
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
	UT_DEBUGMSG(("SEVIOR: deleteing table %x table used %d \n",this,m_bTableUsed));
	if(!m_bTableUsed)
	{
		_removeAllStruxes();
	}
	UT_VECTOR_PURGEALL(ie_imp_cell *,m_vecCells);
}

/*!
 * Open a new cell.
 */
void ie_imp_table::OpenCell(void)
{
	ie_imp_cell * pNewCell = new ie_imp_cell(this, m_pDoc,m_pCurImpCell,m_iRowCounter);
	m_pCurImpCell = pNewCell;
	m_vecCells.addItem((void *) pNewCell);
	m_bNewRow = false;
}

/*!
 * Start a new row.
 */
void ie_imp_table::NewRow(void)
{
	m_pCurImpCell = NULL;
	m_iRowCounter++;
	m_iPosOnRow = 0;
	m_iCellXOnRow = 0;
	m_bNewRow = true;
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
	for(i=0; i < (UT_sint32) m_vecCells.getItemCount(); i++)
	{
		pCell = (ie_imp_cell *) m_vecCells.getNthItem(i);
		if(pCell->getRow() == row)
		{
			if(col == ColCount)
			{
				break;
			}
			ColCount++;
		}
	}
	if(i == (UT_sint32) m_vecCells.getItemCount())
	{
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
PL_StruxDocHandle ie_imp_table::getTableSDH(void)
{
	return m_tableSDH;
}

/*!
 * Set the SDH for this table
 */
void ie_imp_table::setTableSDH(PL_StruxDocHandle sdh)
{
	m_tableSDH = sdh;
	UT_DEBUGMSG(("SEVIOR: Table sdh set to %x \n",sdh));
}

/*!
 * Write out all the properties in the properties string to the tableSDH
 */
void ie_imp_table::writeTablePropsInDoc(void)
{
	UT_return_if_fail(m_tableSDH);
	UT_String colwidths;
	UT_sint32 i=0;
	if(!m_bAutoFit)
	{
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
		UT_String sColSpace = getPropVal("table-col-spacing");
		if(sColSpace.size() == 0)
		{
			sColSpace = "0.05in";
		}
		UT_String sLeftPos = getPropVal("table-column-leftpos");
		if(sLeftPos.size()==0)
		{
			sLeftPos = "0.0in";
		}
		double dLeftPos = UT_convertToInches(sLeftPos.c_str());
		double dColSpace = UT_convertToInches(sColSpace.c_str());
		_buildCellXVector();
		UT_sint32 iPrev = 0;
//
// Now build the table-col-width string.
//
		UT_String sColWidth;
		sColWidth.clear();
		for(i=0; i< (UT_sint32) m_vecCellX.getItemCount(); i++)
		{
			UT_sint32 iCellx = (UT_sint32) m_vecCellX.getNthItem(i);
			iCellx -= iPrev;
			double dCellx = ((double) iCellx)/1440.0 - dLeftPos - dColSpace;
			iPrev = iCellx;
			sColWidth += UT_formatDimensionString(DIM_IN,dCellx,NULL);
			sColWidth += "/";
		}
		setProp("table-col-props",sColWidth.c_str());
	}
	m_pDoc->changeStruxAttsNoUpdate(m_tableSDH,"props",m_sTableProps.c_str());
}

/*!
 * Write out all the properties in all the cells to the PieceTable
 */
void ie_imp_table::writeAllCellPropsInDoc(void)
{
	UT_sint32 i =0;
	ie_imp_cell * pCell = NULL;
	for(i=0; i< (UT_sint32) m_vecCells.getItemCount();i++)
	{
		pCell = (ie_imp_cell *) m_vecCells.getNthItem(i);
		if(!pCell->isMergedAbove() && !pCell->isMergedRight())
		{
			UT_DEBUGMSG(("SEVIOR: pCell %d row %d left %d right %d top %d bot %d sdh %x \n",i,pCell->getRow(),pCell->getLeft(),pCell->getRight(),pCell->getTop(),pCell->getBot(),pCell->getCellSDH())); 
			pCell->writeCellPropsInDoc();
		}
		if(pCell->isMergedAbove() && (pCell->getCellSDH() != NULL))
		{
			UT_DEBUGMSG(("BUG!BUG! found a sdh is merged above cell! removing it \n"));
			m_pDoc->deleteStruxNoUpdate(pCell->getCellSDH());
		}
	}
}

/*!
 * Set a property in the table properties string.
 */
void ie_imp_table::setProp(const UT_String & psProp, const UT_String & psVal)
{
	UT_String_setProperty(m_sTableProps, psProp, psVal);
}

/*!
 * Return the value of a property of this table. 
 * This should be deleted when you've finished with it.
 */
UT_String ie_imp_table::getPropVal(const UT_String & psProp)
{
	return UT_String_getPropVal(m_sTableProps, psProp);
}


/*!
 * Set a property in the table properties string.
 */
void ie_imp_table::setProp(const char * szProp, const char * szVal)
{
	UT_String psProp = szProp;
	UT_String psVal = szVal;
	UT_String_setProperty(m_sTableProps, psProp, psVal);
}

/*!
 * Return the value of a property of this table. 
 * This should be deleted when you've finished with it.
 */
UT_String ie_imp_table::getPropVal(const char * szProp)
{
	UT_String psProp = szProp; 
	return UT_String_getPropVal(m_sTableProps, psProp);
}

/*!
 * Set a property in the current cell properties string.
 */
void ie_imp_table::setCellProp(const UT_String & psProp, const UT_String & psVal)
{
	UT_return_if_fail(m_pCurImpCell);
	m_pCurImpCell->setProp(psProp, psVal);
}

/*!
 * Return the value of a property of the current cell. 
 * This should be deleted when you've finished with it.
 */
UT_String ie_imp_table::getCellPropVal(const UT_String & psProp)
{
	UT_return_val_if_fail(m_pCurImpCell,"");
	return m_pCurImpCell->getPropVal(psProp);
}

/*!
 * Return a pointer to the current cell
 */
ie_imp_cell * ie_imp_table::getCurCell(void)
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
	UT_sint32 x1 = *((UT_sint32 *) vX1);
	UT_sint32 x2 = *((UT_sint32 *) vX2);
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
	for(i=0; i< (UT_sint32) m_vecCells.getItemCount(); i++)
	{
		pCell = (ie_imp_cell *) m_vecCells.getNthItem(i);
		UT_sint32 cellx = pCell->getCellX();
		if(m_vecCellX.findItem((void *) cellx) < 0)
		{
			m_vecCellX.addItem((void *) cellx);
		}
	}
	m_vecCellX.qsort(compareCellX);
}

/*!
 * Returns column number plus 1 of the cell.
 */
UT_sint32 ie_imp_table::getColNumber(ie_imp_cell * pImpCell)
{
	UT_sint32 cellx = pImpCell->getCellX();
	UT_sint32 col = m_vecCellX.findItem((void *) cellx);
	UT_return_val_if_fail((col>=0), -1)
	return col + 1;
}

ie_imp_cell *  ie_imp_table::getCellAtRowColX(UT_sint32 iRow,UT_sint32 cellX)
{
	UT_sint32 i = 0;
	ie_imp_cell * pCell = NULL;
	bool bfound = false;
	for(i=0; i< (UT_sint32) m_vecCells.getItemCount(); i++)
	{
		pCell = (ie_imp_cell *) m_vecCells.getNthItem(i);
		UT_sint32 icellx = pCell->getCellX();
		if(icellx == cellX && (pCell->getRow() == iRow))
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
	UT_sint32 cellx = 0;
	UT_sint32 curRow = 0;
	UT_sint32 iLeft =0;
	UT_sint32 iRight=0;
	UT_sint32 iTop=0;
	UT_sint32 iBot=0;

	for(i=0; i< (UT_sint32) m_vecCells.getItemCount(); i++)
	{
		bool bSkipThis = false;
		pCell = (ie_imp_cell *) m_vecCells.getNthItem(i);
		cellx = pCell->getCellX();
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
			UT_DEBUGMSG(("SEVIOR: This cell is meregd above!!!!!!!!! cellx %d \n",cellx));
			iLeft = getColNumber(pCell);
			bSkipThis = true;
		}
		iRight = getColNumber(pCell);
		iTop = curRow;
		if(pCell->isFirstVerticalMerged())
		{
			//
			// The cells below this are vertically merged with this. go hunting for the last one.
			//
			UT_sint32 newRow = curRow+1;
			ie_imp_cell * pNewCell = getCellAtRowColX(newRow,pCell->getCellX());
			UT_DEBUGMSG(("SEVIOR: This cell is first vertical mereged cell class %x cellx %d \n",pNewCell,cellx));
			if(pNewCell)
			{
				UT_DEBUGMSG(("SEVIOR: this cellx %d, found cellx %d, found row %d \n",cellx,pNewCell->getCellX(),pNewCell->getRow()));
			}
			while(pNewCell && (pNewCell->isMergedAbove()) )
			{
				newRow++;
				pNewCell = getCellAtRowColX(newRow,pCell->getCellX());
			}
			iBot = newRow;
			UT_DEBUGMSG(("SEVIOR: This cell bottom is %d \n",iBot));
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
			pCell->setLeft(iLeft);
			pCell->setRight(iRight);
			pCell->setTop(iTop);
			pCell->setBot(iBot);
			UT_DEBUGMSG(("SEVIOR: Left %d Right %d top %d bot %d \n",iLeft,iRight,iTop,iBot));
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
UT_sint32  ie_imp_table::getNumRows(void)
{
	UT_sint32 numrows = 0;
	UT_sint32 i =0;
	ie_imp_cell * pCell = NULL;
	for(i= (UT_sint32) m_vecCells.getItemCount() -1; i >=0 ; i--)
	{
		pCell = (ie_imp_cell *) m_vecCells.getNthItem(i);
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
 * This method scans the vector of cells and removes cells and their sdh's if they
 * do no have cellc defined.
 */
void ie_imp_table::removeExtraneousCells(void)
{
	UT_sint32 i =0;
	ie_imp_cell * pCell = NULL;
	for(i= (UT_sint32) m_vecCells.getItemCount() -1; i >=0 ; i--)
	{
		pCell = (ie_imp_cell *) m_vecCells.getNthItem(i);
		if(pCell->getCellX() == 0)
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
	for(i= (UT_sint32) m_vecCells.getItemCount() -1; i >=0 ; i--)
	{
		pCell = (ie_imp_cell *) m_vecCells.getNthItem(i);
		if(pCell->getCellSDH())
		{
			UT_DEBUGMSG(("SEVIOR: Removing cell strux %x from PT \n",pCell->getCellSDH())); 
			m_pDoc->deleteStruxNoUpdate(pCell->getCellSDH());
		}
	}
	if(m_tableSDH)
	{
		UT_DEBUGMSG(("SEVIOR: Removing table strux %x from PT \n",m_tableSDH)); 
		m_pDoc->deleteStruxNoUpdate(m_tableSDH);
	}
}

/*!
 * This method scans the vector of cell looking for the nth cell on the current row
 * Return null if cell is not present.
*/
ie_imp_cell * ie_imp_table::getNthCellOnRow(UT_sint32 iCell)
{
	ie_imp_cell * pFoundCell = NULL;
	ie_imp_cell * pCell = NULL;
	UT_sint32 iCellOnRow =0;
	UT_sint32 i=0;
	bool bFound = false;
	for(i=0; !bFound &&  (i< m_vecCells.getItemCount()); i++)
	{
		pCell = (ie_imp_cell *) m_vecCells.getNthItem(i);
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
	while(m_sLastTable.getDepth() > 1)
	{
		ie_imp_table * pT = NULL;
		m_sLastTable.pop((void **)&pT);
		if(pT->wasTableUsed())
		{
			pT->buildTableStructure();
			pT->writeTablePropsInDoc();
			pT->writeAllCellPropsInDoc();
		}			
		UT_DEBUGMSG(("SEVIOR: Deleting table %x \n",pT));
		delete pT;
	}
}

UT_sint32 ie_imp_table_control::getNestDepth(void)
{
	return m_sLastTable.getDepth() -1;
}

void ie_imp_table_control::OpenTable(void)
{
	ie_imp_table * pT = new ie_imp_table(m_pDoc);
	m_sLastTable.push((void *) pT);
}


void ie_imp_table_control::OpenCell(void)
{
	ie_imp_table * pT = NULL;
	m_sLastTable.viewTop((void **) &pT);
	pT->OpenCell();
}

void ie_imp_table_control::CloseTable(void)
{
	ie_imp_table * pT = NULL;
	m_sLastTable.pop((void **) &pT);
	if(pT->wasTableUsed())
	{
		pT->buildTableStructure();
		pT->writeTablePropsInDoc();
		pT->writeAllCellPropsInDoc();
	}
	delete pT;
}


void ie_imp_table_control::CloseCell(void)
{
	ie_imp_table * pT = NULL;
	m_sLastTable.viewTop((void **) &pT);
	pT->CloseCell();
}

ie_imp_table *  ie_imp_table_control::getTable(void)
{
	ie_imp_table * pT = NULL;
	m_sLastTable.viewTop((void **) &pT);
	return pT;
}
