/* haven't touched */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "config.h"
#include "mswordview.h"

int nostyles;

S8 decode_stylesheet(doc *d) {

/* stsh = d->fib.fcStshf
   stshlen = d->fib.lcbStshf */

	U16 sprm=0;
	U16 j;
	int i,upxbytes,len,m;
	U16 thingy;
	
	U16 cb;
	style *stylelist;
	U16 pos;
	int baseistd;
	int tempistd;


	trace("the new stsh is (%x), len (%x)\n",stsh,stshlen);

	fseek(tablefd,stsh,SEEK_SET);
	trace("not really stsh size is %ld\n", read_16ubit(tablefd));
	
	nostyles = read_16ubit(tablefd);
	error(erroroutput,"count of styles is %d\n",nostyles);
	error(erroroutput,"cbSTDBaseInFile is %ld\n",read_16ubit(tablefd));
	for (i=0;i<7;i++)
		read_16ubit(tablefd);

	stylelist = (style *) malloc(sizeof(style) * nostyles);
	if (stylelist == NULL)
		{
		fprintf(erroroutput,"arse no mem for styles !\n");
		exit(-1);
		}
	
	for (m=0;m<nostyles;m++)
		{
		cb = read_16ubit(tablefd);
		error(erroroutput,"size of std %ld\n",cb);

		init_pap(&stylelist[m].thepap);

		init_chp(&stylelist[m].thechp);
		
		error(erroroutput,"m val is %d\n",m);

		if (cb != 0)
			{
			pos=0;

			read_16ubit(tablefd);
			pos+=2;
			j = read_16ubit(tablefd);
			pos+=2;
			baseistd = j>>4;
			error(erroroutput,"based upon istd is %d %d",j,baseistd);
			if (baseistd < 0x0ffe)	
				{
				if (baseistd < m)
					fill_pap(stylelist,m,baseistd);
				else
					error(erroroutput,"how can i be based on a style that i havent seen yet?\n");
				}
			else
				error(erroroutput,"WARNING strange and unsupported istd base %x\n",baseistd);
			j = read_16ubit(tablefd);
			pos+=2;
			error(erroroutput,"# of upx is %d %d\n",j,j>>12); 
			upxbytes = j;
			for (i=0;i<2;i++)
				{
				read_16ubit(tablefd);
				pos+=2;
				}
			len = read_16ubit(tablefd);
			pos+=2;
			error(erroroutput,"name len is %d,\n",len);
			for(i=0;i<len+1;i++)
				{
				thingy = read_16ubit(tablefd);
				pos+=2;
				error(erroroutput,"thingy is %ld (%X) (-%x-) (-%c-)\n",thingy,thingy,thingy,thingy);
				}

			error(erroroutput,"pos is %x and %d",ftell(tablefd),pos);

			if ((pos+1)/2 != pos/2)
				{
				/*eat odd bytes*/
				error(erroroutput,"odd offset\n");
				fseek(tablefd,1,SEEK_CUR);
				pos++;
				}

			if (pos == cb)
				{
				error(erroroutput,"continuing\n");
				continue;
				}
			else if (pos > cb)
				{
				error(erroroutput,"rewinding & continuing\n");
				fseek(tablefd,cb-pos,SEEK_CUR);
				continue;
				}
			if ((pos+1)/2 != pos/2)
				{
				/*eat odd bytes*/
				error(erroroutput,"odd offset\n");
				fseek(tablefd,1,SEEK_CUR);
				pos++;
				}

			if (pos == cb)
				{
				error(erroroutput,"continuing\n");
				continue;
				}
			else if (pos > cb)
				{
				error(erroroutput,"rewinding & continuing\n");
				fseek(tablefd,cb-pos,SEEK_CUR);
				continue;
				}

			/*read some kind of len variable, hairy territory again*/
			upxbytes = read_16ubit(tablefd);
			pos+=2;
			error(erroroutput,"1 eat %d bytes",upxbytes);
			j=0;
			if (upxbytes > 1)
				{
				tempistd = read_16ubit(tablefd);
				pos+=2;
				j+=2;
				if (tempistd != m)
					{
					error(erroroutput,"istds mismatch is %x %x\n",m,tempistd);
					/*treat it as a sprm instead*/
					sprm = tempistd;
					}
				else
					{
					if (j < upxbytes)
						{
						sprm = read_16ubit(tablefd);
						j+=2;
						pos+=2;
						}
					}
				}

			while (j<upxbytes)
				{
				error(erroroutput," j was %d\n",j);
				pos-=j;
				decode_sprm(tablefd,sprm,&(stylelist[m].thepap),&(stylelist[m].thechp),NULL,&j,NULL,stylelist,stylelist[m].thepap.istd);
				error(erroroutput," j is %d\n",j);
				pos+=j;
				if (j>=upxbytes)
					break;
				sprm = read_16ubit(tablefd);
				j+=2;
				pos+=2;
				}

			if ((pos+1)/2 != pos/2)
				{
				/*eat odd bytes*/
				error(erroroutput,"odd offset\n");
				fseek(tablefd,1,SEEK_CUR);
				error(erroroutput,"were at %x\n",ftell(tablefd));
				pos++;
				}

			if (pos == cb)
				{
				error(erroroutput,"continuing\n");
				continue;
				}
			else if (pos > cb)
				{
				error(erroroutput,"rewinding & continuing\n");
				fseek(tablefd,cb-pos,SEEK_CUR);
				continue;
				}

			/*then this is the len of bytes of the next stuff*/
			upxbytes = read_16ubit(tablefd);
			pos+=2;
			error(erroroutput,"2 eat %d bytes",upxbytes);

			/*k this is the good stuff*/
			j=0;
			while (j < upxbytes)
				{
				sprm = read_16ubit(tablefd);
				j+=2;
				pos+=2;
				error(erroroutput," j was %d\n",j);
				pos-=j;
				decode_sprm(tablefd,sprm,&(stylelist[m].thepap),&(stylelist[m].thechp),NULL,&j,NULL,stylelist,stylelist[m].thepap.istd);
				error(erroroutput," j is %d\n",j);
				pos+=j;
				}
			
			if ((pos+1)/2 != pos/2)
				{
				/*eat odd bytes*/
				error(erroroutput,"odd offset\n");
				fseek(tablefd,1,SEEK_CUR);
				pos++;
				}

			error(erroroutput,"m val %d, ilfo is %d\n",m,stylelist[m].thepap.ilfo);
			}
		}
	return(stylelist);
	}


void fill_pap(style *stylelist,int m,int b)
	{
	stylelist[m].thepap.fInTable = stylelist[b].thepap.fInTable;
	stylelist[m].thepap.fTtp= stylelist[b].thepap.fTtp;
	stylelist[m].thepap.ilvl = stylelist[b].thepap.ilvl;
	stylelist[m].thepap.ilfo = stylelist[b].thepap.ilfo;
	stylelist[m].thepap.list_data=stylelist[b].thepap.list_data;
	stylelist[m].thepap.leftmargin=stylelist[b].thepap.leftmargin;
	stylelist[m].thepap.firstline=stylelist[b].thepap.firstline;

	stylelist[m].thechp.fBold=stylelist[b].thechp.fBold;
	stylelist[m].thechp.fItalic=stylelist[b].thechp.fItalic;
	stylelist[m].thechp.fCaps =stylelist[b].thechp.fCaps;
	stylelist[m].thechp.animation=stylelist[b].thechp.animation;
	stylelist[m].thechp.ascii_font=stylelist[b].thechp.ascii_font;
	stylelist[m].thechp.eastfont=stylelist[b].thechp.eastfont;
	stylelist[m].thechp.noneastfont=stylelist[b].thechp.noneastfont;
	stylelist[m].thechp.fontsize=stylelist[b].thechp.fontsize;
	stylelist[m].thechp.supersubscript= stylelist[b].thechp.supersubscript;
	stylelist[m].thechp.fontcode=stylelist[b].thechp.fontcode;
	stylelist[m].thechp.fontspec=stylelist[b].thechp.fontspec;
	strcpy(stylelist[m].thechp.color,stylelist[b].thechp.color);
	stylelist[m].thechp.underline=stylelist[b].thechp.underline;
	stylelist[m].thechp.fSpec=stylelist[b].thechp.fSpec;
	stylelist[m].thechp.fObj=stylelist[b].thechp.fObj;
	stylelist[m].thechp.idctHint=stylelist[b].thechp.idctHint;
	stylelist[m].thechp.fcPic=stylelist[b].thechp.fcPic;
	stylelist[m].thechp.fData=stylelist[b].thechp.fData;
	stylelist[m].thechp.fStrike=stylelist[b].thechp.fStrike;
	stylelist[m].thechp.fDStrike=stylelist[b].thechp.fDStrike;

	}

