 
/*
** The contents of this file are subject to the AbiSource Public
** License Version 1.0 (the "License"); you may not use this file
** except in compliance with the License. You may obtain a copy
** of the License at http://www.abisource.com/LICENSE/ 
** 
** Software distributed under the License is distributed on an
** "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
** implied. See the License for the specific language governing
** rights and limitations under the License. 
** 
** The Original Code is AbiWord.
** 
** The Initial Developer of the Original Code is AbiSource, Inc.
** Portions created by AbiSource, Inc. are Copyright (C) 1998 AbiSource, Inc. 
** All Rights Reserved. 
** 
** Contributor(s):
**  
*/

#ifndef IE_TYPES_H
#define IE_TYPES_H

typedef enum _IEStatus { IES_OK,
						 IES_Error,
						 IES_FileNotFound,
						 IES_NoMemory,
						 IES_UnknownType,
						 IES_BogusDocument,
						 IES_CouldNotOpenForWriting,
						 IES_CouldNotWriteToFile } IEStatus;

typedef enum _IEFileType {	IEFT_Unknown,
							IEFT_AbiWord_1 } IEFileType;


#endif /* IE_TYPES_H */
