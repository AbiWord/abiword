
#include <stdio.h>
#include "config.h"
#include "mswordview.h"

extern FILE *erroroutput;
extern FILE *outputfile;
extern long int cp;
extern int insuper;
extern int footnotehack;


void decode_gpprls(pap *apap,chp *achp,sep *asep,U16 *gpprl,int ndx,Sprm *sprmlists,style *sheet)
	{
	/**/
	int j=0;
	U16 i=0;
	Sprm *thesprmlist=sprmlists;
	U16 sprm;
	U8 operand;
	U8 *pointer;
	U8 littlelist[2];
	int val;

	error(erroroutput,"index were looking for is %d\n",ndx);
	if (ndx <0)
		return;

	/*the index were given is the sprm, if its a varient 1 
	ignore it for now, if its varient 2 go through the list of sprms countinf
	varients 2, until we get to this one*/
	if (0 == (gpprl[ndx] & 0x01))
		{
		error(erroroutput,"varient 1s, (%x)\n",gpprl[ndx]);
		error(erroroutput,"sprm varient 1, isprm is %x, val is %d\n",(gpprl[ndx]&0x00fe)>>1,(gpprl[ndx]&0xff00)>>8);
		sprm = (gpprl[ndx]&0x00fe)>>1;
		operand = (gpprl[ndx]&0xff00)>>8;
		error(erroroutput,"GPRL is (%x) operand is %d",sprm,operand);
		switch (sprm)
			{
			case 5:
				sprm = 0x2403;
				littlelist[0] = operand;
				break;
			default:
				error(erroroutput,"unsupported gpprl %d\n",sprm);
				return;
				break;
			}
		pointer = littlelist;
		decode_sprm(NULL,sprm,apap,achp,asep,&i,&pointer,sheet,apap->istd);
		return;
		}
	else
		{
		error(erroroutput,"varient 2s, (%x) (%x) (%x)\n",(gpprl[ndx]&0xFFFE)>>1,gpprl[ndx]>>1,gpprl[ndx]);
		/*
		for (i=0;i<ndx;i++)
			{
			if (gpprl[i] & 0x01)
				j++;
			}
		*/
		error(erroroutput,"read ndx val is %d\n",j);
		}
	i=0;	
	val = (gpprl[ndx]&0xFFFE)>>1;
	
	while(i<val)
		{
		thesprmlist=thesprmlist->next;
		if (thesprmlist == NULL)
			{
			error(erroroutput,"gpprl ndx is wrong at %d!\n",i);
			return;
			}
		i++;
		}


/*hum 261*/
	i=0;
	error(erroroutput,"sprm list len is %d\n",thesprmlist->len);
	while (i<thesprmlist->len)
		{
		sprm = sread_16ubit(thesprmlist->list+i);
		i+=2;
		error(erroroutput,"clist is %x, at len point %d\n",sprm,i);

		pointer = thesprmlist->list+i;
		if (apap == NULL)
			fprintf(erroroutput,"aha !!\n");

		decode_sprm(NULL,sprm,apap,achp,asep,&i,&pointer,sheet,apap->istd);
		error(erroroutput,"after line\n");
		/*change above to include sep*/
		}
	}
