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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */


#ifndef IE_EXP_PALMDOC_H
#define IE_EXP_PALMDOC_H

#include <stdio.h>
#include "ie_exp.h"
#include "ie_impexp_Palm.h"
#include "ie_exp_Text.h"

// The exporter/writer for PalmDoc file format.

/*****************************************************************/
/*****************************************************************/

class IE_Exp_PalmDoc_Sniffer : public IE_ExpSniffer
{
	friend class IE_Exp;

public:
	IE_Exp_PalmDoc_Sniffer () {}
	virtual ~IE_Exp_PalmDoc_Sniffer () {}

	virtual bool recognizeSuffix (const char * szSuffix);
	virtual bool getDlgLabels (const char ** szDesc,
							   const char ** szSuffixList,
							   IEFileType * ft);
	virtual UT_Error constructExporter (PD_Document * pDocument,
										IE_Exp ** ppie);
};

class IE_Exp_PalmDoc : public IE_Exp_Text
{
public:
	IE_Exp_PalmDoc(PD_Document * pDocument);
	virtual ~IE_Exp_PalmDoc();

protected:
	bool				_openFile(const char * szFilename);
	UT_uint32			_writeBytes(const UT_Byte * pBytes, UT_uint32 length);
	bool				_writeBytes(const UT_Byte * sz);
	bool				_closeFile(void);

        void				_selectSwap();
	void				_compress( buffer* );
	Byte*				_mem_find( Byte *t, int t_len, Byte *m, int m_len );
	Word				_swap_Word( Word );
	DWord				_swap_DWord( DWord );
//	void				_uncompress( buffer* );
	void				_zero_fill( char*, int len );

private:
    
        FILE *				m_pdfp;
	pdb_header			m_header;
	doc_record0			m_rec0;
	unsigned long		m_index;
	DWord				m_recOffset;
        UT_uint32			m_numRecords;
	DWord				m_fileSize;
	buffer *			m_buf;
	UT_uint32			m_bufLen;
        UT_uint32			m_bufPosition;
	bool				m_littlendian;	

};

#endif /* IE_EXP_PALMDOC */
