#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "config.h"
#include "mswordview.h"

#define AUTO 0xff
U16 colorlookupr[17] = {AUTO,0x00,0x00,0x00,0x00,0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x87,0x87,0x87,0xa9,0xd3};
U16 colorlookupg[17] = {AUTO,0x00,0x00,0xff,0xff,0x00,0x00,0xff,0xff,0x00,0x87,0x87,0x00,0x00,0x87,0xa9,0xd3};
U16 colorlookupb[17] = {AUTO,0x00,0xff,0xff,0x00,0xff,0x00,0x00,0xff,0x87,0x87,0x00,0x87,0x00,0x00,0xa9,0xd3};

extern FILE *erroroutput;
extern FILE *outputfile;
extern int colcount;
extern int rowcount;
extern char backgroundcolor[8];

int do_tablelooks(pap *apap)
	{
	tablelook temp;
	char *acolor=NULL;
	char red[3];
	char green[3];
	char blue[3];
	U16 templ;
	/*check to see if we are supposed to use the background color of this table style*/

	error(erroroutput,"itl is %d, colcount is %d, rowcount is %d\n",apap->ourtap.tlp.itl,colcount,rowcount);

	switch(apap->ourtap.tlp.itl)
		{
		case 3:
			temp.color[0]="#000000";
			temp.color[1]="#000000";
			temp.color[2]="#000000";
			temp.color[3]=NULL;
			temp.color[4]=NULL;
			temp.color[5]=NULL;
			temp.color[6]=NULL;
			temp.color[7]=NULL;
			temp.color[8]=NULL;
			break;
		case 5:
			temp.color[0]="#780078";
			temp.color[1]="#780078";
			temp.color[2]="#780078";
			temp.color[3]="#c0c0c0";
			temp.color[4]="#ffffff";
			temp.color[5]="#ffffff";
			temp.color[6]="#c0c0c0";
			temp.color[7]="#ffffff";
			temp.color[8]="#ffffff";
			break;
		case 6:
			temp.color[0]="#000078";
			temp.color[1]="#000078";
			temp.color[2]="#000078";
			temp.color[3]="#c0c0c0";
			temp.color[4]="#c0c0c0";
			temp.color[5]="#c0c0c0";
			temp.color[6]="#c0c0c0";
			temp.color[7]="#c0c0c0";
			temp.color[8]="#c0c0c0";
			break;
		case 7:
			temp.color[0]="#787cb8";
			temp.color[1]="#787cb8";
			temp.color[2]="#787cb8";
			temp.color[3]="#ffffff";
			temp.color[4]="#ffffff";
			temp.color[5]="#ffffff";
			temp.color[6]="#ffffff";
			temp.color[7]="#ffffff";
			temp.color[8]="#ffffff";
			break;
		case 8:
			temp.color[0]="#000000";
			temp.color[1]="#000000";
			temp.color[2]="#000000";
			temp.color[3]="#000078";
			temp.color[4]="#007c78";
			temp.color[5]="#007c78";
			temp.color[6]="#000078";
			temp.color[7]="#007c78";
			temp.color[8]="#007c78";
			break;
		case 9:
			temp.color[0]="#780000";
			temp.color[1]="#780000";
			temp.color[2]="#780000";
			temp.color[3]="#f8fcc8";
			temp.color[4]="#f8fcc8";
			temp.color[5]="#f8fcc8";
			temp.color[6]="#f8fcc8";
			temp.color[7]="#f8fcc8";
			temp.color[8]="#f8fcc8";
			break;
		case 10:
			temp.color[0]="#000000";
			temp.color[1]="#007c78";
			temp.color[2]="#007c78";
			temp.color[3]="#007c78";
			temp.color[4]="#b8dcd8";
			temp.color[5]="#b8dcd8";
			temp.color[6]="#007c78";
			temp.color[7]="#b8dcd8";
			temp.color[8]="#b8dcd8";
			break;
		case 11:
			temp.color[0]="#c0c0c0";
			temp.color[1]="#c0c0c0";
			temp.color[2]="#f8fcc8";
			temp.color[3]="#c0c0c0";
			temp.color[4]="#c0c0c0";
			temp.color[5]="#f8fcc8";
			temp.color[6]="#c0c0c0";
			temp.color[7]="#c0c0c0";
			temp.color[8]="#f8fcc8";
			break;
		case 12:
			temp.color[0]="#000078";
			temp.color[1]="#000078";
			temp.color[2]="#000078";
			temp.color[3]="#c0c0c0";
			temp.color[4]="#c0c0c0";
			temp.color[5]="#b8fcb8";
			temp.color[6]="#c0c0c0";
			temp.color[7]="#c0c0c0";
			temp.color[8]="#b8fcb8";
			break;
		case 13:
			temp.color[0]="#000078";
			temp.color[1]="#000078";
			temp.color[2]="#000078";
			temp.color[3]="#c0c0c0";
			temp.color[4]="#c0c0c0";
			temp.color[5]="#e5e5e5";
			temp.color[6]="#c0c0c0";
			temp.color[7]="#c0c0c0";
			temp.color[8]="#e5e5e5";
			break;
		case 14:
			temp.color[0]="#000000";
			temp.color[1]="#000000";
			temp.color[2]="#000000";
			temp.color[3]="#b8dcd8";
			temp.color[4]="#b8dcd8";
			temp.color[5]="#e5e5e5";
			temp.color[6]="#b8dcd8";
			temp.color[7]="#b8dcd8";
			temp.color[8]="#e5e5e5";
			break;
		case 15:
			temp.color[0]="#c0c0c0";
			temp.color[1]="#c0c0c0";
			temp.color[2]="#c0c0c0";
			temp.color[3]="#c0c0c0";
			temp.color[4]="#c0c0c0";
			temp.color[5]="#c0c0c0";
			temp.color[6]="#c0c0c0";
			temp.color[7]="#c0c0c0";
			temp.color[8]="#c0c0c0";
			break;
		case 18:
		case 19:
			temp.color[0]="#f8fcc8";
			temp.color[1]="#f8fcc8";
			temp.color[2]="#f8fcc8";
			temp.color[3]=NULL;
			temp.color[4]=NULL;
			temp.color[5]=NULL;
			temp.color[6]=NULL;
			temp.color[7]=NULL;
			temp.color[8]=NULL;
			break;
		case 23:
			temp.color[0]="#000078";
			temp.color[1]="#000078";
			temp.color[2]="#000078";
			temp.color[3]=NULL;
			temp.color[4]=NULL;
			temp.color[5]=NULL;
			temp.color[6]=NULL;
			temp.color[7]=NULL;
			temp.color[8]=NULL;
			break;
		case 24:
			temp.color[0]="#c0c0c0";
			temp.color[1]="#c0c0c0";
			temp.color[2]="#c0c0c0";
			temp.color[3]="#ffffff";
			temp.color[4]="#ffffff";
			temp.color[5]="#ffffff";
			temp.color[6]="#c0c0c0";
			temp.color[7]="#c0c0c0";
			temp.color[8]="#c0c0c0";
			break;
		case 25:
			temp.color[0]="#007c78";
			temp.color[1]="#007c78";
			temp.color[2]="#007c78";
			temp.color[3]="#b8fcb8";
			temp.color[4]="#b8fcb8";
			temp.color[5]="#b8fcb8";
			temp.color[6]="#b8fcb8";
			temp.color[7]="#b8fcb8";
			temp.color[8]="#b8fcb8";
			break;
		case 27:
			temp.color[0]="#787c78";
			temp.color[1]="#787c78";
			temp.color[2]="#787c78";
			temp.color[3]=NULL;
			temp.color[4]=NULL;
			temp.color[5]=NULL;
			temp.color[6]=NULL;
			temp.color[7]=NULL;
			temp.color[8]=NULL;
			break;
		case 30:
			temp.color[0]="#c0c0c0";
			temp.color[1]="#c0c0c0";
			temp.color[2]="#c0c0c0";
			temp.color[3]="#f8fcc8";
			temp.color[4]="#f8fcc8";
			temp.color[5]="#f8fcc8";
			temp.color[6]="#c0c0c0";
			temp.color[7]="#c0c0c0";
			temp.color[8]="#c0c0c0";
			break;
		case 31:
			temp.color[0]="#ffff00";
			temp.color[1]="#ffff00";
			temp.color[2]="#ffff00";
			temp.color[3]="#f87c78";
			temp.color[4]="#f87c78";
			temp.color[5]="#f87c78";
			temp.color[6]="#f8fcc8";
			temp.color[7]="#f8fcc8";
			temp.color[8]="#f8fcc8";
			break;
		case 32:
		case 33:
			temp.color[0]="#c0c0c0";
			temp.color[1]="#c0c0c0";
			temp.color[2]="#c0c0c0";
			temp.color[3]="#c0c0c0";
			temp.color[4]="#c0c0c0";
			temp.color[5]="#c0c0c0";
			temp.color[6]="#c0c0c0";
			temp.color[7]="#c0c0c0";
			temp.color[8]="#c0c0c0";
			break;
		case 34:
			temp.color[0]="#c0c0c0";
			temp.color[1]="#c0c0c0";
			temp.color[2]="#d8dcd8";
			temp.color[3]="#c0c0c0";
			temp.color[4]="#c0c0c0";
			temp.color[5]="#d8dcd8";
			temp.color[6]="#c0c0c0";
			temp.color[7]="#c0c0c0";
			temp.color[8]="#d8dcd8";
			break;
		case 37:
			temp.color[0]="#000000";
			temp.color[1]="#000000";
			temp.color[2]="#000000";
			temp.color[3]=NULL;
			temp.color[4]=NULL;
			temp.color[5]=NULL;
			temp.color[6]=NULL;
			temp.color[7]=NULL;
			temp.color[8]=NULL;
			break;
		case 38:
			temp.color[0]=NULL;
			temp.color[1]=NULL;
			temp.color[2]=NULL;
			temp.color[3]=NULL;
			temp.color[4]=NULL;
			temp.color[5]=NULL;
			temp.color[6]="#d8dcb8";
			temp.color[7]="#d8dcb8";
			temp.color[8]="#d8dcb8";
			break;
		case 39:
			temp.color[0]=NULL;
			temp.color[1]=NULL;
			temp.color[2]=NULL;
			temp.color[3]="#b8dcb8";
			temp.color[4]="#b8dcb8";
			temp.color[5]=NULL;
			temp.color[6]="#b8dcb8";
			temp.color[7]="#b8dcb8";
			temp.color[8]=NULL;
			break;
		case 35:
			/*works on its own*/
		case 29:
			/*works on its own*/
		case 36:
		case 28:
		case 26:
		case 16:
		case 17:
		case 20:
		case 21:
		case 22:
		default:
			temp.color[0]=NULL;
			temp.color[1]=NULL;
			temp.color[2]=NULL;
			temp.color[3]=NULL;
			temp.color[4]=NULL;
			temp.color[5]=NULL;
			temp.color[6]=NULL;
			temp.color[7]=NULL;
			temp.color[8]=NULL;
			break;
		}

	if ((rowcount == 1) && (apap->ourtap.tlp.fHdrRows))
		{
		if (apap->ourtap.tlp.fShading)
			{
			if ((colcount == 0) && (apap->ourtap.tlp.fHdrCols))
				acolor=temp.color[0];
			else if (isodd(colcount))
				acolor=temp.color[2];
			else
				acolor=temp.color[1];
			}
		}
	else if (isodd(rowcount))
		{
		if (apap->ourtap.tlp.fShading)
			{
			if ((colcount == 0) && (apap->ourtap.tlp.fHdrCols))
				acolor=temp.color[3];
			else if (isodd(colcount))
				acolor=temp.color[5];
			else
				acolor=temp.color[4];
			}
		}
	else
		{
		if (apap->ourtap.tlp.fShading)
			{
			if ((colcount == 0) && (apap->ourtap.tlp.fHdrCols))
				acolor=temp.color[6];
			else if (isodd(colcount))
				acolor=temp.color[8];
			else
				acolor=temp.color[7];
			}
		}

	if (acolor != NULL)
		{
		if (!(apap->ourtap.tlp.fColor))
			{
			red[0] = acolor[1];
			red[1] = acolor[2];
			red[2] = '\0';
			green[0] = acolor[3];
			green[1] = acolor[4];
			green[2] = '\0';
			blue[0] = acolor[5];
			blue[1] = acolor[6];
			blue[2] = '\0';
			templ = (strtol(red,NULL,16)+strtol(blue,NULL,16)+strtol(green,NULL,16))/3;
			sprintf(backgroundcolor,"#%.2x%.2x%.2x",templ,templ,templ);
			}
		else
			strcpy(backgroundcolor,acolor);
			
		fprintf(outputfile," bgcolor=\"%s\">\n",backgroundcolor); 
		return(1);
		}
	return(0);
	}

void output_tablebg(pap *apap)
	{
	int shademodifier = 100;
	int foreground;

	if (apap->ourtap.tlp.itl)
		{
		if (do_tablelooks(apap))
			return;
		}
	
	if (apap->ourtap.cell_pattern[colcount] != 0)
		{
		if ((apap->ourtap.cell_pattern[colcount] > 13) && (apap->ourtap.cell_pattern[colcount] < 35) )
			{
			foreground = apap->ourtap.cell_fronts[colcount];
			if (foreground == 0) foreground=1;
			fprintf(outputfile," background=\"%s/%d.gif\"",patterndir(),(apap->ourtap.cell_pattern[colcount])+((foreground-1)*12));
			}
		else
			{
			switch (apap->ourtap.cell_pattern[colcount])
				{
				case 1:
					shademodifier = 100;
					break;
				case 2:
					shademodifier = 95;
					break;
				case 3:
					shademodifier = 90;
					break;
				case 4:
					shademodifier = 80;
					break;
				case 5:
					shademodifier = 75;
					break;
				case 6:
					shademodifier = 70;
					break;
				case 7:
					shademodifier = 60;
					break;
				case 8:
					shademodifier = 50;
					break;
				case 9:
					shademodifier = 40;
					break;
				case 10:
					shademodifier = 30;
					break;
				case 11:
					shademodifier = 25;
					break;
				case 12:
					shademodifier = 20;
					break;
				case 13:
					shademodifier = 10;
					break;
				case 35:
					shademodifier = 97.5;
					break;
				case 36:
					shademodifier = 92.5;
					break;
				case 37:
					shademodifier = 87.5;
					break;
				case 38:
					shademodifier = 85;
					break;
				case 39:
					shademodifier = 82.5;
					break;
				case 40:
					shademodifier = 77.5;
					break;
				case 41:
					shademodifier = 82.5;
					break;
				case 42:
					shademodifier = 67.5;
					break;
				case 43:
					shademodifier = 65;
					break;
				case 44:
					shademodifier = 62.5;
					break;
				case 45:
					shademodifier = 57.5;
					break;
				case 46:
					shademodifier = 55;
					break;
				case 47:
					shademodifier = 52.5;
					break;
				case 48:
					shademodifier = 47.5;
					break;
				case 49:
					shademodifier = 45;
					break;
				case 50:
					shademodifier = 42.5;
					break;
				case 51:
					shademodifier = 37.5;
					break;
				case 52:
					shademodifier = 35;
					break;
				case 53:
					shademodifier = 32.5;
					break;
				case 54:
					shademodifier = 27.5;
					break;
				case 55:
					shademodifier = 22.5;
					break;
				case 56:
					shademodifier = 17.5;
					break;
				case 57:
					shademodifier = 15;
					break;
				case 58:
					shademodifier = 12.5;
					break;
				case 59:
					shademodifier = 7.5;
					break;
				case 60:
					shademodifier = 5;
					break;
				case 61:
					shademodifier = 2.5;
					break;
				case 62:
					shademodifier = 2;
					break;
				default:
					shademodifier = 100;
					break;
				}
			}
		}

	if (apap->ourtap.cell_backs[colcount] != 0)
		{
		fprintf(outputfile," bgcolor=\"#%.2x",(colorlookupr[apap->ourtap.cell_backs[colcount]])*shademodifier/100);
		fprintf(outputfile,"%.2x",(colorlookupg[apap->ourtap.cell_backs[colcount]])*shademodifier/100);
		fprintf(outputfile,"%.2x",(colorlookupb[apap->ourtap.cell_backs[colcount]])*shademodifier/100);
		fprintf(outputfile,"\"");
		sprintf(backgroundcolor,"#%.2x%.2x%.2x",(colorlookupr[apap->ourtap.cell_backs[colcount]])*shademodifier/100,(colorlookupg[apap->ourtap.cell_backs[colcount]])*shademodifier/100,(colorlookupb[apap->ourtap.cell_backs[colcount]])*shademodifier/100);
		}

	fprintf(outputfile,">\n");
	}

