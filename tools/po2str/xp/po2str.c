/* AbiSource Build Tools
 * Copyright (C) 2005 AbiSource, Inc. 
 * Copyright (C) 2005 Michael D. Pritchett.
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

/*  This is a cross platform C-based translation of ui-backport.pl.
 *  ui-backport.pl was originally coded by Kenneth Christiansen.
 *  It to translate <lang>.po files to Abiwords XML <lang>.strings 
 *  format.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

enum{ XAP_TAG, AP_TAG };

typedef struct {
	char* tag;
	char* msg;
	int tag_length;
	int msg_length;
	int type;
} OutputData;

#define STRMAX 20

int _ParseFileName( char* filename, char* lang )
{
	char* pTemp;

	pTemp = strrchr( filename, '.' );
	if( !pTemp ) return -1;

	
	if( (pTemp - filename) > STRMAX ) return -1; 
	strncpy( lang, filename, pTemp - filename );
	lang[ pTemp - filename] = '\0';
	
	/* substitute _ for - */
	pTemp = lang;
	while( pTemp = strchr(pTemp, '_') ) { lang[pTemp - lang] = '-'; } 
	
	return 1;
}

int _FileLength(char* pszFileName)
{
	FILE* fp;
	int iFileLength;


	fp = fopen(pszFileName, "rb");
	if (!fp) return -1;
	if (0 != fseek(fp, 0, SEEK_END))
	{
		fclose(fp);
		return -1;
	}
	iFileLength = ftell(fp);
	fclose(fp);

	return iFileLength;
}

void _GetEncoding( char* encoding, char* pBytes )
{
	char* pStart = '\0';
	char* pEnd = '\0';

	pStart = strstr( pBytes, "\n\"Content-Type:") ;
	if( !pStart ) return;

	pStart = strstr( pStart, "charset=" );
	if( !pStart ) return;

	pStart += 8;
	pEnd = strstr( pStart, "\\n\"" );
	if( !pEnd ) return;

	/* Check for Maximum encoding string length */
	if( (pEnd - pStart) > STRMAX ) return; 
	strncpy( encoding, pStart, pEnd - pStart );
	encoding[ pEnd - pStart] = '\0';
}

char* _FindNextTag( char* pData )
{
	char* pFind = strstr(pData, "\n#. ");
	if( pFind ) pFind += 4;

	/* Correction for ^#. * Comments */
	if( pFind && (*pFind == '*') )
	{
		return _FindNextTag( pFind );	
	}
	return ( pFind );
}

char* _FindNextMsg( char* pData )
{
	char* pFind = strstr(pData, "\nmsgstr ");
	if( pFind ) pFind += 8;
	return ( pFind );		
}	

int _LengthofTag( char* pTag )
{
	char* pData;
	pData = pTag;
	while( !isspace(*pData) ) { pData++; }
	return ( pData - pTag );
}

int _LengthofMsg( char* pMsg )
{
	char* pData;
	pData = strstr( pMsg, "\n#" );
	if(pData) return ( pData - pMsg );
	return 0;
}			

char* _FindStartofTags( char* pData )
{
	char* pStart;
	/* Find start of Tag blocks */
	pStart = _FindNextTag(pData);
	pStart = _FindNextTag(pStart);
	pStart = _FindNextTag(pStart);
	return pStart;
}

int _GetTagType(char* pData)
{
	char* pAP;
	char* pXAP;
	pAP = strstr(pData, "/ap_String_Id.h");
	pXAP = strstr(pData, "/xap_String_Id.h");
	if (pAP && pXAP) 
	{
		if ( (pXAP > pAP) )
		{
			return AP_TAG;
		}
		else
		{
			return XAP_TAG;
		}
	}
	else if (pAP) { return AP_TAG; }
	else if (pXAP) { return XAP_TAG; }		
	return -1;
}

int _CountTags(char* pData)
{
	char* pStart;
	int nCount = 0 ;
	
	pStart = pData;
	while( pStart ) 
	{ 
		nCount++;
		pStart = _FindNextTag(pStart);
	}
	return nCount;
}

void _LoadTagData( char* pStart, OutputData* pTags)
{
	char* pData;
	OutputData* pCurTag;
	
	pCurTag = pTags;
	pData = pStart;

	do {
		pCurTag->tag        = pData;
		pCurTag->tag_length = _LengthofTag( pData );
		pCurTag->type       = _GetTagType( pData );
		pCurTag->msg        = _FindNextMsg( pData );
		pCurTag->msg_length = _LengthofMsg( pCurTag->msg );
		pData = _FindNextTag( pData );		
		if( pData ) pCurTag++;
		
	} while (pData);
}

int comp_data(const OutputData* a, const OutputData* b)
{
	if( a->type > b->type ) return 1;
	if( a->type < b->type ) return -1;
	return ( strncmp( a->tag, b->tag, (a->tag_length > b->tag_length) ? a->tag_length : b->tag_length  ) );
}

void xform_data( FILE* fp, char* pData, int iLen )
{
	char* pC;
	pC = pData;
	while ( (pC-pData) < iLen )
	{
		switch (*pC) {
		case '\\':
			pC++;
			switch (*pC) {
				case 'n':
					fputs( "&#x000a;", fp );
					break;
				case '"':
					fputs( "&quot;", fp);
					break;
				case 't':
					fputs( "&#x09;", fp);
					break;
				case '\\':
					fputc( '\\', fp);
					break;
				default:
					fwrite( pC, sizeof(unsigned char), 1, fp);
					break;
			}
			break;	
		case '&':
			fputs( "&amp;", fp);
			break;
		case '<':
			fputs( "&lt;", fp);
			break;
		case '>':
			fputs( "&gt;", fp);
			break;
				
		case '\n':
		case '\r':
		case '"':
			break;
		default:
			fwrite( pC, sizeof(unsigned char), 1, fp);
			break;
		}
		pC++;
	}
}

void print_header( FILE* fp, char* encoding, char* lang )
{
	fputs( "<?xml version=\"1.0\" encoding=\"",fp);
	fputs( encoding, fp);
	fputs( "\"?>\n", fp);
	fputs( "<!-- ==============================================================  -->\n", fp);
	fputs( "<!-- This file contains AbiWord Strings.  AbiWord is an Open Source  -->\n", fp);
	fputs( "<!-- word processor developed by AbiSource, Inc.  Information about  -->\n", fp);
	fputs( "<!-- this application can be found at http://www.abisource.com       -->\n", fp);
	fputs( "<!-- This file contains the string translations for one language.    -->\n", fp);
	fputs( "<!-- This file is covered by the GNU Public License (GPL).           -->\n", fp);
	fputs( "<!-- ==============================================================  -->\n\n", fp);
	fputs( "<AbiStrings app=\"AbiWord\" ver=\"1.0\" language=\"", fp);
	fputs( lang, fp);
	fputs( "\">\n\n", fp);
}

void print_data( FILE* fp, const OutputData* data, int num, int type )
{
	int i;

	if( type == XAP_TAG ) { fputs("<Strings    class=\"XAP\"\n",fp); }
	else if( type == AP_TAG ) { fputs("<Strings    class=\"AP\"\n" ,fp); }
	else return;
	
	for( i = 0; i < num; i++)
	{
		if( data->type == type )
		{
			xform_data(fp, data->tag, data->tag_length );
			fputc('=', fp);
			fputc('"', fp);
			xform_data(fp, data->msg, data->msg_length );
			fputc('"', fp);
			fputc( '\n',  fp);
		}
		data++;
	}
	fputs( "/>\n\n",fp);
}

void print_footer( FILE* fp )
{
	fputs( "</AbiStrings>\n\n" ,fp);
}

int main(int argc, char** argv)
{
	FILE* fp;
	int iFileSize;
	unsigned char* pBytes;
	char encoding[STRMAX] = "iso-8859-1\0";
	char lang[STRMAX];
	OutputData* pTags;
	char* pStartofTags;
	int numTags;
	
	if (argc != 3)
	{
		fprintf(stderr, "Usage: %s [POfile] [STRINGfile]\n", argv[0] );			
		
		return -1;
	}
	
	/* Determine Lang from <lang>.po of the argv[1] */
	if ( _ParseFileName( argv[1], lang ) == -1 )
	{
		fprintf(stderr, "%s is not a valid *.po filename", argv[1] );
		return -1;
	}
	

	/* Load File into Memory */
	iFileSize = _FileLength( argv[1] );
	if (iFileSize == -1 )
	{
		fprintf(stderr, "Error reading input file: %s\n", argv[1] );
		return -1;
	}
	pBytes = malloc(iFileSize);
	fp = fopen(argv[1], "rb");	
	fread( pBytes, sizeof(char), iFileSize, fp);
	fclose(fp);
	
	/* Parse .po File into index */
	_GetEncoding( encoding, pBytes );
	
	/* Find start of Tag blocks */
	pStartofTags = _FindStartofTags( pBytes );	
	numTags = _CountTags( pStartofTags );

	/* Allocate Memory for array */
	pTags = malloc( sizeof(OutputData) * numTags );
	_LoadTagData( pStartofTags, pTags );	

	/* Sort the Array */
	qsort( pTags, numTags, sizeof(OutputData), (void *)comp_data );

	/* Output .string file XML format */
	fp = fopen(argv[2], "wb" );

	print_header( fp, encoding, lang );
	print_data( fp, pTags, numTags, XAP_TAG );
	print_data( fp, pTags, numTags, AP_TAG );
	print_footer( fp );

	fclose(fp);

	/* Clean up Memory and exit politely */
	free(pBytes);
	free(pTags);
	
	return 0;
}


