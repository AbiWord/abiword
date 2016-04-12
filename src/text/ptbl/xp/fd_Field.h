/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
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
#ifndef FIELD_H
#define FIELD_H

#include "ut_types.h"
#include "ut_xml.h"
#include "pf_Frag_Object.h"
#include "pf_Frag_Text.h"
#include "pt_Types.h"

class fl_BlockLayout;
class pf_Frag_Object;

/*!
 \note This class will eventually have subclasses to implement the different
 types of fields.
*/

class ABI_EXPORT fd_Field
{
 public:
    // TBD: convention for naming
    enum FieldType
    {
        FD_None,
        FD_Test,
        FD_MartinTest,
        FD_Time,
        FD_PageNumber,
        FD_PageCount,
        FD_ListLabel,
        FD_FileName,
        FD_Date,

        FD_Date_MMDDYY,
        FD_Date_DDMMYY,
        FD_Date_MDY,
        FD_Date_MthDY,
        FD_Date_DFL,
        FD_Date_NTDFL,
        FD_Date_Wkday,
        FD_Date_DOY,

        FD_Time_MilTime,
        FD_Time_AMPM,
        FD_Time_Zone,
        FD_Time_Epoch,

        FD_Table_sum_rows,
        FD_Table_sum_cols,

        FD_DateTime_Custom,

        FD_Doc_WordCount,
        FD_Doc_CharCount,
        FD_Doc_LineCount,
        FD_Doc_ParaCount,
        FD_Doc_NbspCount,

        FD_App_Version,
        FD_App_ID,
        FD_App_Options,
        FD_App_Target,
        FD_App_CompileTime,
        FD_App_CompileDate,

        FD_Endnote_Ref,
        FD_Endnote_Anchor,

        FD_Footnote_Ref,
        FD_Footnote_Anchor,

        FD_PageReference,
        FD_MailMerge,

        FD_Meta_Title,
        FD_Meta_Creator,
        FD_Meta_Subject,
        FD_Meta_Publisher,
        FD_Meta_Date,
        FD_Meta_Date_Last_Changed,
        FD_Meta_Type,
        FD_Meta_Language,
        FD_Meta_Rights,
        FD_Meta_Keywords,
        FD_Meta_Contributor,
        FD_Meta_Coverage,
        FD_Meta_Description,

        __last_field_dont_use__
    };

    fd_Field(pf_Frag_Object& fO, pt_PieceTable * pt, FieldType fieldType, const gchar *pParam);
    virtual							~fd_Field(void);
    bool							update(void);
    void							setBlock(fl_BlockLayout * pBlock);
    fl_BlockLayout *				getBlock( void) const;
	FieldType						getFieldType(void) const;
	gchar *						getValue(void) const;
	void							setValue(const gchar * szValue);
	const gchar * 				getParameter(void) const {return static_cast<const gchar *>(m_pParameter);};
    // probably need different types of update
    // which are overridden in the appropriate subclass
    // eg positionChangeUpdate
    //    referenceChangeUpdate
 protected:
    bool							_deleteSpan(void);
    void							_throwChangeRec(PT_DocPosition docPos);
    fl_BlockLayout * m_pBlock;
    // will need some more helper functions in here eg. to test
    // whether text has changed to avoid unnecessary updates
 private:
    pf_Frag_Object& m_fragObject;
    pt_PieceTable *	m_pPieceTable;
    UT_uint32 m_updateCount;
    FieldType m_iFieldType;
    gchar * m_szValue;
    gchar * m_pParameter;
};

#endif




