#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "config.h"
#include "mswordview.h"

extern FILE *erroroutput;
extern FILE *outputfile;
extern int inunderline;
extern int newline;
extern int inalist;
extern long int cp;
extern long int realcp;



U16 fail16safe[1] = {0};

void get_table_info(FILE *tablefd,list_info *a_list_info,U32 fcSttbFnm,U32 lcbSttbFnm,U32 fcPlcfLst,U32 lcbPlcfLst,U32 fcPlfLfo,U32 lcbPlfLfo, style *sheet)
	{
	U32 i,j;
	int k;
	U16 t;
	U8 len,chplen,paplen,restart,fWord6;

	pap listpap;
	int dolvl;

	S32 startno;
	int starttype;

	U16 temp16=0;

	U16 baseistd=1;

	U16 level=0;

	U16 *list_string=NULL;

	U32 absolutehack; 
	int hackflag=1;
	/*this variable is dedicated to Tethys <tethys@ml.com> who found the
	first file to require it*/

	list_def *tempdef;
	list_def *tempdef2;
	list_def *defptr=NULL;

	chp achp;

	a_list_info->array = NULL;
	a_list_info->count= 0;
	a_list_info->lstarray = NULL;
	a_list_info->lstcount= 0;
	a_list_info->o_lst_ids=NULL;
	a_list_info->current_index_nos=NULL;
	a_list_info->o_list_def=NULL;
	a_list_info->level=NULL;
	a_list_info->lst_ids=NULL;
	a_list_info->a_list_def=NULL;
	a_list_info->overridecount=NULL;


	error(erroroutput,"table names begins at %X, and it %d long",fcSttbFnm,lcbSttbFnm);
	if (lcbSttbFnm !=0)
		{
		fseek(tablefd,fcSttbFnm,SEEK_SET);

		for(i=0;i<lcbSttbFnm;i++)
			{
			error(erroroutput,"%x ",getc(tablefd));
			}
		error(erroroutput,"\n"); 
		}

	if (lcbPlcfLst!=0)
		{
		error(erroroutput,"table lst begins at %X, and it %d long",fcPlcfLst,lcbPlcfLst);
		fseek(tablefd,fcPlcfLst,SEEK_SET);

		a_list_info->lstarray = (U8 *) malloc(lcbPlcfLst);
		if (a_list_info->lstarray == NULL)
			{
			fprintf(erroroutput,"no memory, lp\n");
			exit(-1);
			}
		a_list_info->lstcount = lcbPlcfLst;
		
		a_list_info->nooflsts = read_16ubit(tablefd);
		error(erroroutput,"number of lsts is %d\n",a_list_info->nooflsts);


		a_list_info->current_index_nos= (int **) malloc(sizeof(int*) * (a_list_info->nooflsts+1));
		if (a_list_info->current_index_nos== NULL)
			{
			fprintf(erroroutput,"no memory here\n");
			exit(-1);
			}

		/*make a no of lsts x 9 array which will keep track of what value each
		list is indexed at for each level*/
		for (i=0;i<a_list_info->nooflsts+1;i++)
			{
			a_list_info->current_index_nos[i] = (int *) malloc(sizeof(int) * 9 );
				{
				if (a_list_info->current_index_nos[i]== NULL)
					{
					fprintf(erroroutput,"no memory here\n");
					exit(-1);
					}
				for (j=0;j<9;j++)
					a_list_info->current_index_nos[i][j] = -1;
				}
			}

		a_list_info->o_lst_ids = (U32 *) malloc(sizeof(U32) * a_list_info->nooflsts);
		if (a_list_info->o_lst_ids== NULL)
			{
			fprintf(erroroutput,"no memory here\n");
			exit(-1);
			}
		for(k=0;k<a_list_info->nooflsts;k++)
			a_list_info->o_lst_ids[k] = 0;

		a_list_info->level = (U8 *) malloc(a_list_info->nooflsts);
		if (a_list_info->level == NULL)
			{
			fprintf(erroroutput,"no memory here\n");
			exit(-1);
			}

		i=0;
		k=0;
		while ( (i<lcbPlcfLst-2) && (k <a_list_info->nooflsts) )
			{
			a_list_info->o_lst_ids[k] = read_32ubit(tablefd);
			error(erroroutput,"ID is %x\n",a_list_info->o_lst_ids[k]);
			error(erroroutput,"template ID is %x\n",read_32ubit(tablefd));
			i+=8;
			for(j=0;j<18;j++)
				{
				a_list_info->lstarray[j] = getc(tablefd);
				i++;
				error(erroroutput,"%x ",a_list_info->lstarray[j]);
				}
			a_list_info->level[k] = getc(tablefd);
			error(erroroutput,"level flag is %x\n",a_list_info->level[k]);
			a_list_info->level[k] &= 0x01;
			getc(tablefd);
			i+=2;
			k++;
			}
		error(erroroutput,"\n"); 
		a_list_info->a_list_def = (list_def *)malloc(a_list_info->nooflsts*sizeof(list_def));
		if (a_list_info->a_list_def == NULL)
			{
			fprintf(erroroutput,"no memory ppp\n");
			exit(-1);
			}
		for (k=0;k<a_list_info->nooflsts;k++)
			{
			a_list_info->a_list_def[k].sub_def_list=NULL;
			a_list_info->a_list_def[k].len=0;
			a_list_info->a_list_def[k].list_string=NULL;
			}

		

		error(erroroutput,"table lst begins at %X, ends at %x",fcPlcfLst,fcPlcfLst+lcbPlcfLst);
		error(erroroutput,"but table override begins at %X, and it %d long ends at %x",fcPlfLfo,lcbPlfLfo,fcPlfLfo+lcbPlfLfo);

		fseek(tablefd,fcPlcfLst+lcbPlcfLst,SEEK_SET);

		/*begin determined with a hex editor*/
		i=0;
		k=0;
		while ( (i<fcPlfLfo-(fcPlcfLst+lcbPlcfLst)) && (k < a_list_info->nooflsts) )
		/*
		while (i<fcPlfLfo-(fcPlcfLst+lcbPlcfLst)) 
		*/
			{
			error(erroroutput,"discarding unknown list related data list data");
/*
			startno = getc(tablefd);
			i++;
			error(erroroutput,"start no is %d\n",startno);

			for(j=0;j<3;j++)
				{
				error(erroroutput,"unknown: %x\n",getc(tablefd));
				i++;
				}
*/
			startno = (S32) read_32ubit(tablefd);
			error(erroroutput,"start no is %d\n",startno);
			i+=4;

			starttype = getc(tablefd);
			i++;
			error(erroroutput,"start type is %d\n",starttype);
			if (starttype == 0)
				error(erroroutput,"normal type\n");
			else if (starttype == 1)
				error(erroroutput,"roman type\n");
			else if (starttype == 3)
				error(erroroutput,"alpha type\n");

			restart = getc(tablefd);
			i++;
			fWord6 = restart;
			restart &= 0x10;
			fWord6 &= 0x40;
			if ((fWord6) && (restart))
				restart=1;
			else
				restart=0;
			error(erroroutput,"list restart is %d\n",restart);

			for(j=0;j<18;j++)
				{
				error(erroroutput,"unknown: %x\n",getc(tablefd));
				i++;
				}
			
			temp16 = getc(tablefd);
			temp16+=getc(tablefd);
			i+=2;
			error(erroroutput,"len ? %d (%x)\n",temp16,temp16);
			/*
			fill_pap(style *stylelist,int m,int b)
			*/
			baseistd = read_16ubit(tablefd);
			error(erroroutput,"base istd ? (%x)\n",baseistd);
			i+=2;
			t=0;
			len = temp16;
			error(erroroutput,"begin LIST PAP/CHP\n");
		
			init_chp(&achp);

			while(t<len)
				{
				temp16 = read_16ubit(tablefd);
				t+=2;
				decode_sprm(tablefd,temp16,&listpap,&achp,NULL,&t,NULL,sheet,baseistd);
				}
			i+=t;
			temp16 = read_16ubit(tablefd);
			i+=2;
			error(erroroutput,"len of string is %d\n",temp16);

			if (i == a_list_info->nooflsts)
				{
				error(erroroutput,"oh oh !\n");
				/* abandon as something has gone wrong, hope for the best*/
				break;
				}
			list_string = (U16 *) malloc(sizeof(U16) * (temp16+1));

			if (list_string == NULL)
				{
				fprintf(erroroutput,"no memory len %d\n",temp16);
				exit(-1);
				}

			error(erroroutput,"were at %x\n",ftell(tablefd));

			/*check the values, if it falls between 0 and 9, then this is the level we're at*/

			for (j=0;j<temp16;j++)
				{
				list_string[j] = read_16ubit(tablefd);
				i+=2;
				}
				
			list_string[j]='\0';
			for (j=0;j<temp16;j++)
				error(erroroutput,"%c %x",list_string[j],list_string[j]);


			if (a_list_info->level[k] == 1)
				{
				a_list_info->a_list_def[k].len = temp16;
				a_list_info->a_list_def[k].begin_no = startno;
				a_list_info->a_list_def[k].no_type = starttype;
				a_list_info->a_list_def[k].achp = achp;
				a_list_info->a_list_def[k].list_string = list_string;
				a_list_info->a_list_def[k].fPrev= restart;
				a_list_info->a_list_def[k].sub_def_list=NULL;
				list_string = NULL;
				error(erroroutput,"simple list (no %d)\n",k);
				for (j=0;j<temp16;j++)
					error(erroroutput,"%c ",a_list_info->a_list_def[k].list_string[j]);
				defptr=NULL;
				k++;
				level=0;
				}
			else  /*long list*/
				{
				error(erroroutput,"complex list (no %d) level %d\n",k,level);
				if (level == 0)
					{
					a_list_info->a_list_def[k].len = temp16;
					a_list_info->a_list_def[k].begin_no = startno;
					a_list_info->a_list_def[k].no_type = starttype;
					a_list_info->a_list_def[k].achp = achp;
					a_list_info->a_list_def[k].list_string = list_string;
					a_list_info->a_list_def[k].sub_def_list=NULL;
					a_list_info->a_list_def[k].fPrev= restart;
					list_string = NULL;
					for (j=0;j<temp16;j++)
						error(erroroutput,"%c ",a_list_info->a_list_def[k].list_string[j]);
					defptr=NULL;
					}
				else
					{
					tempdef = (list_def *)malloc(sizeof(list_def));
					if (tempdef == NULL)
						{
						fprintf(erroroutput,"no memory ppp\n");
						exit(-1);
						}
					tempdef->len = temp16;
					if (tempdef->len == 0)
						error(erroroutput,"0 len 1\n");
					tempdef->begin_no = startno;
					tempdef->no_type = starttype;
					tempdef->achp = achp;
					tempdef->fPrev = restart;
					tempdef->list_string = list_string;
					tempdef->sub_def_list=NULL;
					if (level == 1)
						{
						a_list_info->a_list_def[k].sub_def_list = tempdef;
						defptr = a_list_info->a_list_def[k].sub_def_list;
						}
					else
						{
						if (defptr != NULL)
							{
							defptr->sub_def_list = tempdef;
							defptr =  defptr->sub_def_list;
							}
						else
							{
							error(erroroutput,"defptr was NULL !!\n");
							}
						}
					tempdef = tempdef->sub_def_list;
					}
				level++;
				if (level == 9) 
					{
					k++;
					level=0;
					}
				}

			error(erroroutput,"end LIST PAP/CHP\n");
			tempdef=NULL;
			}
		/*end determined with a hex editor*/
		}

	fseek(tablefd,fcPlfLfo,SEEK_SET);

	error(erroroutput,"fcPlfLfo is %x, ends at %x\n",fcPlfLfo+lcbPlfLfo,fcPlfLfo);

	if (lcbPlfLfo !=0)
		{
		fseek(tablefd,fcPlfLfo,SEEK_SET);

		a_list_info->array = (U8 *) malloc(lcbPlfLfo);
		if (a_list_info->array == NULL)
			{
			fprintf(erroroutput,"no memory ppsd\n");
			exit(-1);
			}
		a_list_info->count = lcbPlfLfo;

		a_list_info->nooflfos=read_32ubit(tablefd);
		error(erroroutput,"there are %ld LFO's\n",a_list_info->nooflfos);


		a_list_info->lst_ids = (U32 *) malloc(sizeof(U32) * a_list_info->nooflfos);
		if (a_list_info->lst_ids== NULL)
			{
			fprintf(erroroutput,"no memory opo\n");
			exit(-1);
			}

		a_list_info->overridecount = (int *) malloc(sizeof(int) * a_list_info->nooflfos);
		if (a_list_info->overridecount == NULL)
			{
			fprintf(erroroutput,"no memory opo\n");
			exit(-1);
			}

		a_list_info->o_list_def = (list_def *)malloc(a_list_info->nooflfos*sizeof(list_def));
		if (a_list_info->o_list_def == NULL)
			{
			fprintf(erroroutput,"scream\n");
			exit(-1);
			}

		for(i=0;i<a_list_info->nooflfos;i++)
			{
			a_list_info->lst_ids[i] = read_32ubit(tablefd);
			error(erroroutput,"IDHERE-->%x \n",a_list_info->lst_ids[i]);
			for (j=0;j<8;j++)
				error(erroroutput,"LFO:undoc %x\n",getc(tablefd));
			a_list_info->overridecount[i] = getc(tablefd);
			error(erroroutput,"is there a lfovl ? %d\n",a_list_info->overridecount[i]);

			k=0;
			while(k<a_list_info->nooflsts)
				{
				if (a_list_info->lst_ids[i]  == a_list_info->o_lst_ids[k])
					break;
				k++;
				}

			if (a_list_info->overridecount[i] > 0)
				{
				error(erroroutput,"create a copy\n");
				/*
				a_list_info->o_list_def[i] = (list_def *) malloc(sizeof(list_def));
				*/
				tempdef = &(a_list_info->o_list_def[i]);
				tempdef2 = &(a_list_info->a_list_def[k]);

				if ((tempdef == NULL) || (tempdef2 == NULL))
					{
					fprintf(erroroutput,"list problem 2\n");
					return;
					}

				do
					{
					tempdef->len =  tempdef2->len;
					if (tempdef->len == 0)
						error(erroroutput,"0 len 2\n");
					else
						error(erroroutput,"lsn is %d\n",tempdef->len);
					tempdef->begin_no = tempdef2->begin_no;
					tempdef->no_type =  tempdef2->no_type;
					tempdef->achp = tempdef2->achp;
					tempdef->fPrev = tempdef2->fPrev;
					tempdef->list_string = (U16 *) malloc(sizeof(U16) * (tempdef2->len+1));
					if (tempdef->list_string != NULL)
						{
						for (j=0;j<tempdef->len;j++)
							{
							if (j==tempdef2->len)
								{
								error(erroroutput,"overran string len %d, somthings wrong!\n",tempdef2->len);
								if (tempdef2->len == 0)
									{
									fprintf(erroroutput,"somehow len was 0\n");
									tempdef2->list_string=(U16*) malloc(sizeof(U16));
									tempdef2->list_string[0] = '\0';
									}
								break;
								}
							tempdef->list_string[j] = tempdef2->list_string[j];
							}
						tempdef->list_string[j] = '\0';
						}

					if (tempdef2->sub_def_list != NULL)
						tempdef->sub_def_list = (list_def *) malloc(sizeof(list_def));
					else
						tempdef->sub_def_list= NULL;
					tempdef = tempdef->sub_def_list;
					tempdef2 = tempdef2->sub_def_list;
					}
				while(tempdef2 != NULL);

				}
			else
				{
				a_list_info->o_list_def[i] = a_list_info->a_list_def[k];
				error(erroroutput,"linking lfo id %x %d to lst id %x %d\n",a_list_info->lst_ids[i],i,a_list_info->o_lst_ids,k);
				}
			for (j=0;j<3;j++)
				error(erroroutput,"LFO:undoc %x\n",getc(tablefd));
			}


		error(erroroutput,"---->nooflfos is %d, nooflsts is %d,table offset is %x\n",a_list_info->nooflfos,a_list_info->nooflsts,ftell(tablefd));
/*
		for(i=0;i<a_list_info->nooflfos;i++)
*/
		for(i=0;i<a_list_info->nooflsts;i++)
			{
			if (a_list_info->overridecount[i] > 0)
				{
#if 0
				fprintf(stderr,"sample ffffffff doc\n");
#endif
#if 0
				do
					{
					absolutehack = read_32ubit(tablefd); 
					/*
					what the hell is this ????, dont know where it 
					comes from, im not sure *what* is going on here.
					just that theres from one ffffffff to two ffffffff 
					sets that are showing up for no reason that i understand.
					im ignoring them for now, maybe they mean to skip a level.
					maybe they mean something else. 
					*/
					error(erroroutput,"absolute hack = %x\n",absolutehack);
					}
				while(absolutehack == 0xffffffffUL);
#endif

				/*
				the reasoning here is quite poor, but basically
				im running on the theory that ffffffff is a flag
				that tells us when to begin using the lfovls
				*/
				absolutehack = read_32ubit(tablefd); 
				if (absolutehack == 0xffffffffUL)
					{
					hackflag=0;
					continue;
					}
				else if (hackflag==1)
					continue;
					
				fseek(tablefd,-4,SEEK_CUR);

				error(erroroutput,"sitting at %x\n",ftell(tablefd));

				error(erroroutput,"startat etc ? is %x\n",read_32ubit(tablefd));
				len = getc(tablefd);
				tempdef2 = &(a_list_info->o_list_def[i]);
				error(erroroutput,"overrode level is %d\n",len & 0x0F);
				k = len & 0x0F;
				j=0;
				while(j < k)
					{
					if (tempdef2->sub_def_list != NULL)
						tempdef2 = tempdef2->sub_def_list;
					else
						error(erroroutput,"oh oh fell off end\n");
					j++;
					}

				error(erroroutput,"following LVL is %d\n",len & 0x20);
				dolvl = len & 0x10;
				len = len &= 0x01;
				for (j=0;j<3;j++)
					{
					error(erroroutput,"reserved %c\n",getc(tablefd));
					}
				error(erroroutput,"onto lvl\n");
				tempdef2->begin_no = read_32ubit(tablefd);
				tempdef2->no_type = getc(tablefd);

				restart = getc(tablefd);
				fWord6 = restart;
				restart &= 0x10;
				fWord6 &= 0x40;
				if ((fWord6) && (restart))
					tempdef2->fPrev=1;

				j+=6;
				for (j=0;j<18;j++)
					error(erroroutput,"temp unused %c\n",getc(tablefd));

				chplen = getc(tablefd);
				error(erroroutput,"chp len is %d\n",chplen);
				paplen  = getc(tablefd);
				error(erroroutput,"pap len is %d\n",paplen);
				read_16ubit(tablefd);
				error(erroroutput,"sitting at %x\n",ftell(tablefd));


				error(erroroutput,"id is %x, adding to number %d\n",a_list_info->o_lst_ids[i],i);
				t=0;
				while(t<paplen+chplen)
					{
					temp16 = read_16ubit(tablefd);
					t+=2;
					/*ignore pap here*/
					decode_sprm(tablefd,temp16,&listpap,&(tempdef2->achp),NULL,&t,NULL,sheet,baseistd);
					}
				tempdef2->len = read_16ubit(tablefd);
				error(erroroutput,"following string is %d long\n",tempdef2->len);
				if (tempdef2->list_string != NULL)
					free(tempdef2->list_string);
				tempdef2->list_string = (U16*) malloc(sizeof(U16) * (tempdef2->len+1));
				for (k=0;k<tempdef2->len;k++)
					{
					tempdef2->list_string[k] = read_16ubit(tablefd);
					error(erroroutput,"str char ? -->%x\n",tempdef2->list_string[k]);
					}
				tempdef2->list_string[k] = '\0';
				}
			}
		error(erroroutput,"\n"); 
		}
	}

void decode_e_list(pap *apap,chp *achp,list_info *a_list_info)
    {
    int i;
    int lvl;
    if ( (apap->ilvl+1) < inalist)
        {
        error(erroroutput,"ILVL: apap->ilvl+1 id %d inalist is %d\n",(apap->ilvl+1),inalist);
/*
        if (apap->ilvl+1 == 0)
            lvl = 1;
        else
*/
            lvl = apap->ilvl+1;

        for(i=inalist;i>lvl;i--)
            {
            /*
            if (i==1)
                fprintf(outputfile,"\n<!--</DIR>-->");
            else
                fprintf(outputfile,"\n</DIR>");
            */
            }
        inalist=apap->ilvl+1;
        }
    }
	

void decode_list_level(pap *apap,int inalist,int num)
	{
	int i;
	if (apap->ilvl+1 != 0)
		{
		error(erroroutput,"List stuff inalist is %d\n, ilvl+1 is %d\n",inalist,apap->ilvl+1);
		if (apap->ilvl+1 == inalist)
			{
			if (apap->ilvl+1 != 1)
				{
				/*
				fprintf(outputfile,"\n</DIR>\n");
				fprintf(outputfile,"\n<dir>\n");
				*/
				}
			else
				{
				/*
				if (num == IGNOREALL)
					fprintf(outputfile,"\n</fake DIR>\n<fake DIR>\n");
				else
					fprintf(outputfile,"\n<!--</DIR>\n<dir>-->\n");
				*/

				}
			}
		else
			{
			for(i=inalist;i<apap->ilvl+1;i++)
				{
				if (i==0)
					{
					if (num == IGNOREALL)
						fprintf(outputfile,"\nfake DIR>\n");
						/*
					else
						fprintf(outputfile,"\n<!--<dir>-->\n");
						*/
					}
				else
					fprintf(outputfile,"\n<DIR>");
				}
			}
		}
	inalist = apap->ilvl+1;
	for(i=1;i<inalist;i++)
		{
		if (inunderline)
			fprintf(outputfile,"</u>");
		fprintf(outputfile,"&nbsp;&nbsp;&nbsp;&nbsp;");
		if (inunderline)
			fprintf(outputfile,"<u>");
		}
	}

void decode_s_anld(pap *apap,chp *achp,list_info *a_list_info,ffn *fontnamelist)
	{
	int i;
	int issection;
	
	chp anldchp;

	error(erroroutput,"ilvl is %d\n",apap->ilvl);

	if ( ( (apap->ilvl+1) > inalist)  /*|| (tempid)*/ )
		{
		error(erroroutput,"ANLD: new one is %d",apap->ilvl+1);
		decode_list_level(apap,0,DONTIGNORENUM);

		/*if we are at a sub level i.e > 0, then */
		error(erroroutput,"the ilfo to use is %d\n",apap->ilfo);

		inalist=apap->ilvl+1;
		}

	if ( (apap->ilvl) != -1)
		{
		if ( (apap->ilfo > -1) && (newline))
			{
			fprintf(outputfile,"\n");

			anldchp = *achp;
			if (apap->anld.fSetBold)
				if (apap->anld.flags2&0x08)
					anldchp.fBold =1;
			if (apap->anld.fSetItalic)
				if (apap->anld.flags2&0x10)
					anldchp.fItalic=1;
			if (apap->anld.flags2&0x02)
					anldchp.underline = apap->anld.flags3&0x07;
			anldchp.ascii_font = apap->anld.ftc;
			anldchp.eastfont = apap->anld.ftc;
			anldchp.noneastfont =apap->anld.ftc;
			anldchp.fontsize = apap->anld.hps;

			i=0;
			while(i<apap->anld.cxchTextBefore)
				{
				error(erroroutput,"list string-->%d %c\n",apap->anld.rgxch[i],apap->anld.rgxch[i]);
				if (apap->anld.rgxch[i] == '*')
					fprintf(outputfile,"<img src=\"%s/%d.gif\">",symbolfontdir(),151);
				else
					{
					decode_letter(apap->anld.rgxch[i],0,apap,&anldchp,NULL,NULL,NULL,fontnamelist,NULL,NULL,&issection);
					realcp--;
					cp--;
					}
				i++;
				}

			error(erroroutput,"numbering type is %d\n",apap->anld.nfc);
			/*
			decode_list_nfc(&(listvalue[apap->ilvl]),apap->anld.nfc);
			*/
			decode_list_nfc(&(a_list_info->current_index_nos[apap->ilfo][apap->ilvl]),apap->anld.nfc);

			i=apap->anld.cxchTextBefore;
			while(i<apap->anld.cxchTextAfter)
				{
				error(erroroutput,"list string-->%d %c\n",apap->anld.rgxch[i],apap->anld.rgxch[i]);
				if (apap->anld.rgxch[i] == '*')
					fprintf(outputfile,"<img src=\"%s/%d.gif\">",symbolfontdir(),151);
				else
					{
					decode_letter(apap->anld.rgxch[i],0,apap,NULL,NULL,NULL,NULL,fontnamelist,NULL,NULL,&issection);
					realcp--;
					cp--;
					}
				i++;
				}

			fprintf(outputfile," ");
			newline=0;
			}
		}
	}
/*
ok im going out on a limb here and see does this fit the realworld: heres the hack
sections titles have the section number prepended to them, now the 0x0c is use like
i would expect it to be, *only* that after this mark sections are also considered list 
elements with links to listdata which contain a mark for a number to be used, 
with my logic i get
<section no> <list number> <the rest of list data e.g. "."> Section Name
other logic gave
<list number> <the rest of list data e.g. "."> <section no> Section Name

what ms gives is

<section no> <the rest of list data e.g. "."> Section Name

so if i have a 0x0c 

CHOP CHOP CHOP, all wrong, talking out my bum, solution is in CHANGELOG for 
version 0.2.1, in fact the section break has NOTHING to do with it, its part
of a wierd mechanism with dummy list elements, im dead proud i figured it out.
*/


void decode_s_list(pap *apap,chp *achp,list_info *a_list_info,ffn *fontnamelist,int num)
	{
	/*
	U32 tempid=0;
	*/
	int i,j,k,p;
	int index;
	int fPrev;

	int issection;/*dummy*/
	
	list_def *list_data=NULL;
	list_def *flist_data=NULL;
	list_def *plist_data=NULL;

	error(erroroutput,"doing start list\n");

	if (apap->ilfo == 2047)
		{
		error(erroroutput,"special ilfo, doing an anld\n");
		decode_s_anld(apap,achp,a_list_info,fontnamelist);
		return;
		}
	
	if (apap->ilfo > -1) 
		{
		index = decode_ilfo(apap,a_list_info);
		if (index == -1)
			{
			error(erroroutput,"invalid ilfo index\n");
			return;
			}

		if ( ( (apap->ilvl+1) > inalist) )
			{
			error(erroroutput,"ILVL: incoming indent level %d new one is %d",inalist,apap->ilvl+1);

			if (num == IGNOREALL)
				fprintf(outputfile,"\n<!--tricky old ms, hidden list values here\n");

			/*if we are at a sub level i.e > 0, then */
			error(erroroutput,"the ilfo to use is %d\n",apap->ilfo);

			flist_data = apap->list_data;
			
			i=0;
			while ( (i<apap->ilvl) && (flist_data->sub_def_list != NULL) )
				{
				error(erroroutput,"1 indenting into structure\n");
				flist_data = flist_data->sub_def_list;
				i++;
				}
	
			if (a_list_info->current_index_nos[index][apap->ilvl]==-1)
				{
				/*new list, start again*/
				plist_data = flist_data;
				for (p=apap->ilvl;p<9;p++)
					{
					if (plist_data != NULL)
						{
						a_list_info->current_index_nos[index][p]=plist_data->begin_no;
						plist_data = plist_data->sub_def_list;
						}
					}
				}
			inalist=apap->ilvl+1;
			}
		else
			flist_data = apap->list_data;

		if ( (apap->ilvl) != -1)
			{
			error(erroroutput,"list stage 1\n");
			if ( (apap->ilfo > -1) && (newline))
				{
				error(erroroutput,"list stage 2\n");
				if (apap->list_data != NULL)
					{
					j=0;
					list_data = apap->list_data;
					fPrev = flist_data->fPrev;
					do
						{
						error(erroroutput,"list stage 3\n");
						if (list_data->len > 0)
							{
							error(erroroutput,"list stage 4\n");
							i=0;
							
							if ( ((!fPrev) && (j == apap->ilvl)) || (fPrev) )
								{
								if (flushbreaks(0))
									{
									error(erroroutput,"<!--new paragraph-->");
									do_indent(apap);
									}
								fprintf(outputfile,"\n");
								decode_list_level(apap,inalist,num);
								while ( i<list_data->len )
									{
									error(erroroutput,"list string-->%d %c\n",list_data->list_string[i],list_data->list_string[i]);
									if (list_data->list_string[i] <= 9)
										{
										flist_data = apap->list_data;
										k=0;
										while(k<list_data->list_string[i])	
											{
											flist_data = flist_data->sub_def_list;
											if (flist_data == NULL)
												{
												fprintf(erroroutput,"list problem 2\n");
												return;
												}
											k++;
											}
										error(erroroutput,"numbering type is %d\n",flist_data->no_type);

										if (a_list_info->current_index_nos[index][k]==-1)
											{
											/*new list, start again*/
											plist_data = flist_data;
											for (p=k;p<9;p++)
												{
												if (plist_data != NULL)
													{
													a_list_info->current_index_nos[index][p]=plist_data->begin_no;
													plist_data = plist_data->sub_def_list;
													}
												}
										/*	
											a_list_info->current_index_nos[index][k]=flist_data->begin_no;
										*/
											
											}
										else if (k!=j)
											a_list_info->current_index_nos[index][k]--;
										decode_list_nfc(&(a_list_info->current_index_nos[index][k]),flist_data->no_type);
										
										plist_data = flist_data->sub_def_list;
										for (p=j+1;p<9;p++)
											{
											if (plist_data != NULL)
												{
												a_list_info->current_index_nos[index][p]=plist_data->begin_no;
												plist_data = plist_data->sub_def_list;
												}
											}
										
										}
									else if (list_data->list_string[i] == '*')
										fprintf(outputfile,"<img src=\"%s/%d.gif\">",symbolfontdir(),151);
									else
										{
										decode_letter(list_data->list_string[i],0,apap,&(list_data->achp),NULL,NULL,NULL,fontnamelist,NULL,NULL,&issection);
										realcp--;
										cp--;
										}
									i++;
									}
									
								fprintf(outputfile," ");
								newline=0;
								}
							}	
						list_data = list_data->sub_def_list;
						j++;
						}
					while ( (j<apap->ilvl+1)  && (list_data != NULL) );
					}
					
					
				}
			if (num == IGNOREALL)
				fprintf(outputfile,"\n-->\n");
			}
		}
	}
