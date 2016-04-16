/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/* AbiSource
 * 
 * Copyright (C) 2002 Dom Lachowicz <cinamod@hotmail.com>
 * Copyright (C) 2004 Robert Staudinger <robsta@stereolyzer.net>
 * Copyright (C) 2005 Daniel d'Andrada T. de Carvalho
 * <daniel.carvalho@indt.org.br>
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
 
// Class definition include.
#include "ODi_Table_ListenerState.h"

// Internal includes
#include "ODi_ElementStack.h"
#include "ODi_Office_Styles.h"
#include "ODi_Style_Style.h"
#include "ODi_ListenerStateAction.h"

// AbiWord includes
#include <pd_Document.h>
#include <ut_std_string.h>


/**
 * Constructor
 */
ODi_Table_ListenerState::ODi_Table_ListenerState (PD_Document* pDocument,
                                 ODi_Office_Styles* pStyles,
                                 ODi_ElementStack& rElementStack) :
                                 ODi_ListenerState("Table", rElementStack),
                                 m_onFirstPass(true),
                                 m_elementLevel(0),
                                 m_pAbiDocument(pDocument),
                                 m_pStyles(pStyles),
                                 m_row(0),
                                 m_col(0),
                                 m_rowsLeftToRepeat(0),
                                 m_gotAllColumnWidths(true)
{
    if (m_rElementStack.hasElement("office:document-content")) {
        m_onContentStream = true;
    } else {
        m_onContentStream = false;
    }
}


/**
 * Called when the XML parser finds a start element tag.
 * 
 * @param pName The name of the element.
 * @param ppAtts The attributes of the parsed start tag.
 */
void ODi_Table_ListenerState::startElement (const gchar* pName,
                                           const gchar** ppAtts,
                                           ODi_ListenerStateAction& rAction)
{
    UT_ASSERT(m_elementLevel >= 0);
    
    
    if (!m_waitingEndElement.empty()) {
        // Do nothing.
        
    } else if (!strcmp(pName, "table:table")) {

        _parseTableStart(ppAtts, rAction);
        
    } else if (!strcmp(pName, "table:table-column")) {

        _parseColumnStart(ppAtts, rAction);

    } else if (!strcmp(pName, "table:table-row")) {
      
       _parseRowStart(ppAtts, rAction);
       
    } else if (!strcmp(pName, "table:table-cell")) {
        _parseCellStart (ppAtts, rAction);
    }
    else if(!strcmp(pName,"table:covered-table-cell")){
      m_col++;
    }

    m_elementLevel++;
}


/**
 * Called when an "end of element" tag is parsed (like <myElementName/>)
 * 
 * @param pName The name of the element
 */
void ODi_Table_ListenerState::endElement (const gchar* pName,
                                         ODi_ListenerStateAction& rAction)
{
    
    UT_ASSERT(m_elementLevel > 0);
    
    if (!m_waitingEndElement.empty()) {
        
        if ( !strcmp(m_waitingEndElement.c_str(), pName) ) {
            // Found it. No more wait.
            m_waitingEndElement.clear();
        }
        
    } else if (!strcmp(pName, "table:table")) {
        
        if (m_elementLevel == 1) {
            
            if (m_onFirstPass) {
                m_onFirstPass = false;
            } else {
                m_pAbiDocument->appendStrux(PTX_EndTable, PP_NOPROPS);
                rAction.popState();
            }
        } else {
            // It's the end of a nested table.
            // Do nothing.
        }
        
    } else if (!strcmp(pName, "table:table-cell")) {
    
      UT_DEBUGMSG(("ODi endCell \n"));
        if (m_onFirstPass) {
            // Do nothing.
        } else{
            m_pAbiDocument->appendStrux(PTX_EndCell, PP_NOPROPS);
        }
    }
    
    m_elementLevel--;
}


/**
 * Used to parse a <table:table> start element.
 */
void ODi_Table_ListenerState::_parseTableStart(const gchar** ppAtts,
                                              ODi_ListenerStateAction& rAction) {
                                                
    if (m_elementLevel == 0) {
        if (m_onFirstPass) {
            rAction.repeatElement();
        } else {
            std::string props;
            const gchar* pVal;
            const ODi_Style_Style* pStyle = NULL;
            
            pVal = UT_getAttribute("table:style-name", ppAtts);
            if (pVal) {
                pStyle = m_pStyles->getTableStyle(pVal, m_onContentStream);
                UT_ASSERT(pStyle);
            }
            
            // Background color
            if (pStyle != NULL) {
                if (!pStyle->getBackgroundColor()->empty()) {
                    props += "background-color:";
                    props += pStyle->getBackgroundColor()->c_str();
                }
            }
            // Left table pos
            if (pStyle != NULL) {
                if (!pStyle->getTableMarginLeft()->empty()) {
		    if (!props.empty()) {
		        props += "; ";
		    }
		    props += "table-column-leftpos:";
		    props += pStyle->getTableMarginLeft()->c_str();
		    
                }
            }

            // table width
            if (pStyle != NULL) {
                if (!pStyle->getTableWidth()->empty()) {
		    if (!props.empty()) {
		        props += "; ";
		    }
		    props += "table-width:";
		    props += pStyle->getTableWidth()->c_str();
		    
                }
            }


            // table relative width 
            if (pStyle != NULL) {
                if (!pStyle->getTableRelWidth()->empty()) {
		    if (!props.empty()) {
		        props += "; ";
		    }
		    props += "table-rel-width:";
		    props += pStyle->getTableRelWidth()->c_str();
		    
                }
            }
  
            // Column widths
            if (m_gotAllColumnWidths) {
                if (!props.empty()) {
                    props += "; ";
                }
                props += "table-column-props:";
                props += m_columnWidths;
            }
 
           // Column Rel widths
            if (m_gotAllColumnWidths && !m_columnRelWidths.empty()) 
	    {
                if (!props.empty()) 
		{
                    props += "; ";
                }
                props += "table-rel-column-props:";
                props += m_columnRelWidths;
            }

            // Row heights
            if (!props.empty()) {
                props += "; ";
            }
            props += "table-row-heights:";
            props += m_rowHeights;

            if (!props.empty()) {
                PP_PropertyVector ppAttribs = {
                    "props", props
                };

                m_pAbiDocument->appendStrux(PTX_SectionTable, ppAttribs);
            } else {
                m_pAbiDocument->appendStrux(PTX_SectionTable, PP_NOPROPS);
            }

            // Initialize cell variables.
            m_row = 0;
            m_col = 0;
        }
    } else {
        // It's a nested table
        if (m_onFirstPass) {
            m_waitingEndElement = "table:table";
        } else {
            rAction.pushState("Table");
        }
    }
}



/**
 * Used to parse a <table:table-row start element.
 */
void ODi_Table_ListenerState::_parseRowStart (const gchar** ppAtts,
                                                 ODi_ListenerStateAction& rAction)
{
    if (m_onFirstPass) 
    {
        const gchar* pStyleName = UT_getAttribute("table:style-name", ppAtts);
        const ODi_Style_Style* pStyle;
        
        const gchar* pNumberRowsRepeated = UT_getAttribute("table:number-rows-repeated", ppAtts);           
        UT_sint32 nRowsRepeated = !pNumberRowsRepeated ? 1 : atoi(pNumberRowsRepeated);
        UT_ASSERT_HARMLESS(nRowsRepeated > 0);

        std::string rowHeight = "";

        if (pStyleName != NULL) 
        {
            pStyle = m_pStyles->getTableRowStyle(pStyleName,
                        m_onContentStream);
            UT_ASSERT(pStyle != NULL);

            if (pStyle)
            {
                if (!pStyle->getRowHeight()->empty()) 
                {
                    rowHeight = *(pStyle->getRowHeight());
                } 
                else if (!pStyle->getMinRowHeight()->empty()) 
                {
                    rowHeight = *(pStyle->getMinRowHeight());
                }
            }
        }

        // AbiWord supports unspecified row heights mixed among specified ones.
        // e.g.: "table-row-heights:2.37cm//3.62cm/"
        for (UT_sint32 i = 0; i < nRowsRepeated; i++) {
            m_rowHeights += rowHeight + "/";
        }
    } 
    else
    {
        if (m_rowsLeftToRepeat == 0) {
            const gchar* pNumberRowsRepeated = UT_getAttribute("table:number-rows-repeated", ppAtts);           
            m_rowsLeftToRepeat = !pNumberRowsRepeated ? 1 : atoi(pNumberRowsRepeated);
            UT_ASSERT_HARMLESS(m_rowsLeftToRepeat > 0);
        }

        m_row++;
        m_col = 0;
        m_rowsLeftToRepeat--;
        if (m_rowsLeftToRepeat > 0) {
            // We want another pass over this table:table-row element
            rAction.repeatElement();
        }
    }
}


/**
 * Used to parse a <table:table-column> start element.
 */
void ODi_Table_ListenerState::_parseColumnStart (const gchar** ppAtts,
                                                 ODi_ListenerStateAction& /*rAction*/)
{
    if (m_onFirstPass) 
    {
        const gchar* pStyleName=NULL;
        const ODi_Style_Style* pStyle=NULL;
        const gchar* pNumberColumnsRepeated = NULL;
        int nColsRepeated= 0;
        UT_sint32 i=0;
        
        pStyleName = UT_getAttribute("table:style-name", ppAtts);
        if (pStyleName != NULL) 
        {
            pStyle = m_pStyles->getTableColumnStyle(pStyleName,
                                                    m_onContentStream);
            UT_ASSERT_HARMLESS(pStyle != NULL);
            UT_DEBUGMSG(("style-name %s pstyle = %p \n",pStyleName,pStyle));
            if (pStyle && (pStyle->getColumnWidth()->empty() && pStyle->getColumnRelWidth()->empty())) 
            {
                m_gotAllColumnWidths = false;
            } 
            else if (pStyle) 
            {
                pNumberColumnsRepeated = UT_getAttribute("table:number-columns-repeated", ppAtts);
                if (pNumberColumnsRepeated != NULL) 
                {
                    nColsRepeated = atoi(pNumberColumnsRepeated);
                    UT_ASSERT(nColsRepeated > 0);
                } 
                else 
                {
                    nColsRepeated = 1;
                }
                if(!pStyle->getColumnWidth()->empty())
                {
                    for (i=0; i<nColsRepeated; i++) 
                    {
                        m_columnWidths += *(pStyle->getColumnWidth());
                        m_columnWidths += "/";
                    }
                }
                if(!pStyle->getColumnRelWidth()->empty())
                {
                    for (i=0; i<nColsRepeated; i++) 
                    {
		        m_columnRelWidths += *(pStyle->getColumnRelWidth());
                        m_columnRelWidths += "/";
		    }
                }
                UT_DEBUGMSG(("m_columnRelWidths %s \n",m_columnRelWidths.c_str()));
            }
        } 
        else 
        {
            m_gotAllColumnWidths = false;
        }
    }
}


/**
 * 
 */
void ODi_Table_ListenerState::_parseCellStart (const gchar** ppAtts,
                                              ODi_ListenerStateAction& rAction)
{
    if (m_onFirstPass) {
        // Do nothing.
    } else {    

        const gchar* xmlid = UT_getAttribute("xml:id", ppAtts);
        UT_DEBUGMSG(("ODi_Table_ListenerState::_parseCellStart() xmlid:%s\n",
                     xmlid ? xmlid : "_undefined_" ));
        
        std::string props;
        const gchar* pVal;
        const ODi_Style_Style* pStyle = NULL;
        UT_sint32 colSpan;
        UT_sint32 rowSpan;
        m_col++;
        std::string dataID;
        
        pVal = UT_getAttribute("table:number-columns-spanned", ppAtts);
        if (pVal) {
            colSpan = atoi(pVal);
            if (colSpan < 1) {
                colSpan = 1;
            }
        } else {
            colSpan = 1;
        }
        pVal = UT_getAttribute("table:number-rows-spanned", ppAtts);
        if (pVal) {
            rowSpan = atoi(pVal);
            if (rowSpan < 1) {
                rowSpan = 1;
            }
        } else {
            rowSpan = 1;
        }
        

        props = UT_std_string_sprintf(
            "top-attach: %d; bot-attach: %d; left-attach: %d; right-attach: %d",
            m_row-1, m_row+(rowSpan-1), m_col-1, m_col + (colSpan-1));

        pVal = UT_getAttribute("table:style-name", ppAtts);
        if (pVal) {
            pStyle = m_pStyles->getTableCellStyle(pVal, m_onContentStream);
            UT_ASSERT(pStyle);
        }

        if (pStyle) {

            ////
            // Top border
            if (pStyle->hasTopBorder() == ODi_Style_Style::HAVE_BORDER_YES) {

                props += "; top-style:solid";

                if (!pStyle->getBorderTop_thickness()->empty()) {
                    props += "; top-thickness:";
                    props += *(pStyle->getBorderTop_thickness());
                }
                
                if (!pStyle->getBorderTop_color()->empty()) {
                    props += "; top-color:";
                    props += *(pStyle->getBorderTop_color());
                }
                
	    }
            else if (pStyle->hasTopBorder() == ODi_Style_Style::HAVE_BORDER_NO) {
                    props += "; top-style:none";
            }
            
            
            ////
            // Bottom border
            if (pStyle->hasBottomBorder() == ODi_Style_Style::HAVE_BORDER_YES) {
            
                props += "; bot-style:solid";
            
                if (!pStyle->getBorderBottom_thickness()->empty()) {
                    props += "; bot-thickness:";
                    props += *(pStyle->getBorderBottom_thickness());
                }
                
                if (!pStyle->getBorderBottom_color()->empty()) {
                    props += "; bot-color:";
                    props += *(pStyle->getBorderBottom_color());
                }
            
            } else if (pStyle->hasBottomBorder() == ODi_Style_Style::HAVE_BORDER_NO) {
                    props += "; bot-style:none";
	    }
            
            
            ////
            // Left border
            if (pStyle->hasLeftBorder() == ODi_Style_Style::HAVE_BORDER_YES) {
            
                props += "; left-style:solid";
                
                if (!pStyle->getBorderLeft_thickness()->empty()) {
                    props += "; left-thickness:";
                    props += *(pStyle->getBorderLeft_thickness());
                }
                
                if (!pStyle->getBorderLeft_color()->empty()) {
                    props += "; left-color:";
                    props += *(pStyle->getBorderLeft_color());
                }
                
            } else if (pStyle->hasLeftBorder() == ODi_Style_Style::HAVE_BORDER_NO) {
                props += "; left-style:none";
            }
            

            ////
            // Right border
            if (pStyle->hasRightBorder() == ODi_Style_Style::HAVE_BORDER_YES) {
            
                props += "; right-style:solid";
                
                if (!pStyle->getBorderRight_thickness()->empty()) {
                    props += "; right-thickness:";
                    props += *(pStyle->getBorderRight_thickness());
                }
                
                if (!pStyle->getBorderRight_color()->empty()) {
                    props += "; right-color:";
                    props += *(pStyle->getBorderRight_color());
                }
                
            } else if (pStyle->hasRightBorder() == ODi_Style_Style::HAVE_BORDER_NO) {
                props += "; right-style:none";
            }
            
            
            // background color
            if (!pStyle->getBackgroundColor()->empty()) {
                props += "; background-color:";
                props += pStyle->getBackgroundColor()->c_str();
            }

            // background-image
            if(!pStyle->getBackgroundImageID()->empty())
            {
                dataID = pStyle->getBackgroundImageID()->c_str();
            }

            // Vertical align
            if (!pStyle->getVerticalAlign()->empty())
            {
                if (!strcmp(pStyle->getVerticalAlign()->c_str(), "top"))
                    props += "; vert-align:0";
                else if (!strcmp(pStyle->getVerticalAlign()->c_str(), "middle"))
                    props += "; vert-align:50";
                else if (!strcmp(pStyle->getVerticalAlign()->c_str(), "bottom"))
                    props += "; vert-align:100";
            }
        }

        PP_PropertyVector cell_props;
        if( xmlid )
        {
            cell_props.push_back(PT_XMLID);
            cell_props.push_back(xmlid);
            props += "; xmlid:";
            props += xmlid;
            UT_DEBUGMSG(("ODi_Table_ListenerState::_parseCellStart() adding xmlid:%s\n", xmlid ));
        }
        cell_props.push_back("props");
        cell_props.push_back(props);
        UT_DEBUGMSG(("ODi_Table_ListenerState::_parseCellStart() props:%s\n", props.c_str() ));
        if(dataID.length() > 0)
        {
            cell_props.push_back("strux-image-dataid");
            cell_props.push_back(dataID);
        }
        m_pAbiDocument->appendStrux(PTX_SectionCell, cell_props);

        // Now parse the cell text content.
        rAction.pushState("TextContent");
    }
}
