#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include "config.h"
#include "mswordview.h"

extern FILE *erroroutput;
extern FILE *outputfile;
extern sep *currentsep;
extern int inaheaderfooter;
extern int sectionpagenumber;
extern int sectionno;


void decode_field(FILE *main,field_info *magic_fields,long *cp,U8 *fieldwas,long *swallowcp1,long *swallowcp2)
	{
	time_t timep;
	struct tm *times;
	char date[1024];
	/*
	called at the start of a field, find the field associated
	with that cp and decode meaning, process to end of field
	*/

	/*wont deal with enbedded fields yet\n*/
	int i=0;
	int type=0;

	*swallowcp1 = -1;
	*swallowcp2 = -1;

	error(erroroutput,"decoding field with cp %x\n",*cp);

	if (magic_fields != NULL)
		{
		while(i<magic_fields->no+1)
			{
			error(erroroutput,"field entry says %d (%x)\n",i,magic_fields->cps[i]);
			if (magic_fields->cps[i] == *cp)
				{
				error(erroroutput,"found field entry at %d\n",i);
				type = (magic_fields->flds[i*2])&0x1f;
				error(erroroutput,"type is %d \n",type);
				break;
				}
			i++;
			}

		switch(type)
			{
			case 19:
				/*beggining a field*/
				*fieldwas = magic_fields->flds[(i*2)+1];
				switch (*fieldwas)
					{
					case 10:	/*style reference*/
					case 12: /*sequence mark, make a hyperlink later on*/
					case 68:
						break;
					case 88:
						error(erroroutput,"HYPERLINK\n");
						/*
						fprintf(outputfile,"<a href=\"");
						*/
						fprintf(outputfile,"<a href=");
						*swallowcp1 = magic_fields->cps[i];	/*beginning of field*/
						*swallowcp2 = magic_fields->cps[i+1]-1; /*end of the first part*/
						break;
					case 33:
						if ((currentsep != NULL) && (inaheaderfooter))
							{
							error(erroroutput,"soing nfc\n");
							error(erroroutput,"nfcPgn is %d\n",currentsep->nfcPgn);
							decode_list_nfc(&sectionpagenumber,currentsep->nfcPgn);
							sectionpagenumber--;
							}
						else 
							fprintf(outputfile,"%d",sectionpagenumber);
						error(erroroutput,"sectionpagenumber is now %d, inaheaderfooter is %d\n",sectionpagenumber,inaheaderfooter);
						break;
					case 65:
						fprintf(outputfile,"%d",sectionno);
						error(erroroutput,"sectionno is now %d\n",sectionno);
						break;
					case 26:
						fprintf(outputfile,"unknown # of pages");
						break;
					case 31:
						timep = time(NULL);
						times = localtime(&timep);
						strftime(date,1024,"%x",times);
						error(erroroutput,"output date as %s\n",date);
						fprintf(outputfile,"%s",date);
						break;
					case 32:
						timep = time(NULL);
						times = localtime(&timep);
						strftime(date,1024,"%X",times);
						error(erroroutput,"output time as %s\n",date);
						fprintf(outputfile,"%s",date);
						break;
					case 29:
						error(erroroutput,"spitting out original filename\n");
						break;
					case 13:
						error(erroroutput,"spitting out toc\n");
						break;
					case 37:
						*swallowcp1 = magic_fields->cps[i];	/*beginning of field*/
						*swallowcp2 = magic_fields->cps[i+1]-1; /*end of the first part*/
					default:
						error(erroroutput,"unsupported field %d\n",*fieldwas);
						break;
					}
				break;
			case 20:
				switch (*fieldwas)
					{
					case 68:
					case 65:
					case 33:
					case 26:
					case 10:
					case 12:
						break;
					case 88:
						error(erroroutput,"HYPERLINK middle\n");
						/*
						fprintf(outputfile,"\">");
						*/
						fprintf(outputfile,">");
						break;
					case 29:
						error(erroroutput,"filename middle\n");
						break;
					case 13:
						error(erroroutput,"toc middle\n");
						break;
					default:
						error(erroroutput,"unsupported field %d\n",*fieldwas);
						break;
					}
				error(erroroutput,"field middle reached\n");
				break;
			case 21:
				switch (*fieldwas)
					{
					case 10:
					case 12:
					case 68:
					case 65:
					case 33:
					case 26:
					case 37:
						break;
					case 88:
						error(erroroutput,"HYPERLINK end\n");
						fprintf(outputfile,"</a>");
						break;
					default:
						error(erroroutput,"unsupported field %d\n",*fieldwas);
						break;
					}
				error(erroroutput,"field end reached\n");
				break;
			default:
				error(erroroutput,"field wierdness!!\n");
				break;
			}
		}
	}
