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

#ifndef AP_CONVERT_H
#define AP_CONVERT_H

#include "ie_types.h"
#include "pd_Document.h"

//////////////////////////////////////////////////////////////////

class AP_Convert
{
 public:
	AP_Convert(void);
	~AP_Convert(void);

    void convertTo(const char * szSourceFilename,
				   IEFileType sourceFormat,
				   const char * szTargetFilename,
				   IEFileType targetFormat);
	
	void convertTo(const char * szSourceFilename,
				   const char * szTargetSuffix);

	void convertTo(const char * szSourceFilename,
				   const char * szSourceSuffix,
				   const char * szTargetFormat);

	void setVerbose(int level);

 private:
	int m_iVerbose;
};

#endif /* AP_CONVERT_H */
