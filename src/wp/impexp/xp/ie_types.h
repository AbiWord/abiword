
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
