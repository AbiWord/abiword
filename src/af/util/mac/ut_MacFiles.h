/*
**	Apple Macintosh Developer Technical Support
**
**	Routines for dealing with full pathnames... if you really must.
**
**	by Jim Luther, Apple Developer Technical Support Emeritus
**
**	File:		FullPath.h
**
**	Copyright © 1995-1999 Apple Computer, Inc.
**	All rights reserved.
**
**	You may incorporate this sample code into your applications without
**	restriction, though the sample code has been provided "AS IS" and the
**	responsibility for its operation is 100% yours.  However, what you are
**	not permitted to do is to redistribute the source as "DSC Sample Code"
**	after having made changes. If you're going to re-distribute the source,
**	we require that you make it clear in the source that the code was
**	descended from Apple Sample Code, but that you've made changes.
*/



#ifndef _UT_MAC_FILE_H_
#define _UT_MAC_FILE_H_

/* From More Files */
pascal	OSErr	FSpGetFullPath(const FSSpec *spec,
							   short *fullPathLength,
							   Handle *fullPath);
/*	¦ Get a full pathname to a volume, directory or file.
	The GetFullPath function builds a full pathname to the specified
	object. The full pathname is returned in the newly created handle
	fullPath and the length of the full pathname is returned in
	fullPathLength. Your program is responsible for disposing of the
	fullPath handle.
	
	Note that a full pathname can be made to a file/directory that does not
	yet exist if all directories up to that file/directory exist. In this case,
	FSpGetFullPath will return a fnfErr.
	
	spec			input:	An FSSpec record specifying the object.
	fullPathLength	output:	The number of characters in the full pathname.
							If the function fails to create a full pathname,
							it sets fullPathLength to 0.
	fullPath		output:	A handle to the newly created full pathname
							buffer. If the function fails to create a
							full pathname, it sets fullPath to NULL.
	
	Result Codes
		noErr				0		No error	
		nsvErr				-35		No such volume
		ioErr				-36		I/O error
		bdNamErr			-37		Bad filename
		fnfErr				-43		File or directory does not exist (fullPath
									and fullPathLength are still valid)
		paramErr			-50		No default volume
		memFullErr			-108	Not enough memory
		dirNFErr			-120	Directory not found or incomplete pathname
		afpAccessDenied		-5000	User does not have the correct access
		afpObjectTypeErr	-5025	Directory not found or incomplete pathname
	
	__________
	
	See also:	GetFullPath
*/

/*****************************************************************************/

#endif
