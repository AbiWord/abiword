#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "config.h"
#include "mswordview.h"

extern FILE *outputfile;
extern FILE *erroroutput;
extern char *outputfilename;
extern char *imagesdir;

extern int madeinmac;

extern char failsafe[1];

extern int errno;

U16 idlist[NOOFIDS] = {0,0x216,0x3D4,0x542,0x6E0,0x46A,0x7A8,0x800};


long get_picture_header(U32 fcPic,FILE *data, U32 *len, U16 *datatype)
	{
	U8 i,j;
	error(erroroutput,"seeking to %x\n",fcPic);
	fseek(data,fcPic,SEEK_SET);
	*len = read_32ubit(data);
	error(erroroutput,"the no of bytes in this PIC is %d\n",*len);
	i = getc(data);
	error(erroroutput,"the no of bytes of the PIC is %x\n",i);
	*datatype = read_16ubit(data);
	error(erroroutput,"an mm is %x\n",*datatype);
	error(erroroutput,"an mm is %x\n",read_16ubit(data));
	error(erroroutput,"an mm is %x\n",read_16ubit(data));
	error(erroroutput,"an mm is %x\n",read_16ubit(data));
	for (j=0;j<(i-13);j++)
		getc(data);
	*len = *len - i;
	return(ftell(data));
	}


obj_by_spid * get_blips(U32 fcDggInfo,U32 lcbDggInfo,FILE *tablefd,FILE *mainfd,int *noofblips,int streamtype,obj_by_spid **realhead)
	{
	U16 element;
	U32 len;
	U32 fulllen=0;
	U32 noofpics;
	U32 offset;
	int i;
	FILE *graphic;
	char *buffer=NULL;
	obj_by_spid *head=NULL;
	obj_by_spid *p=NULL;
	char *imageprefix=NULL;

	int currentspid=0;
	
	U16 spids[1024];
	char **names;
	int *no;
	char *names01[1024];
	static char *names08[1024];
	static int no01=0;
	static int no08=0;

	U16 id;
	U32 op;
	U16 msofbh;
	U16 extra=0;
	U32 flags;

	*noofblips=0;

	if (imagesdir == NULL)
		{
		imageprefix = malloc(strlen(outputfilename)+1);
		if (imageprefix == NULL)
			{
			fprintf(erroroutput,"arrgh, no mem\n");
			exit(-1);
			}
		strcpy(imageprefix,outputfilename);
		}
	else
		{
		imageprefix = malloc(strlen(imagesdir)+2+strlen(ms_basename(outputfilename)));
		if (imageprefix == NULL)
			{
			fprintf(erroroutput,"arrgh, no mem\n");
			exit(-1);
			}
		sprintf(imageprefix,"%s/%s",imagesdir,ms_basename(outputfilename));
		}


	if (streamtype == 0x01)
		{
		mainfd=tablefd;
		names=names01;
		no = &no01;
		memset(names, 0, 1024);
		}
	else		
		{
		names=names08;
		no = &no08;
		}

	if ( (*no) == 0)	
		memset(names, 0, 1024);

	if ((realhead != NULL) && (*realhead != NULL))
		{
		error(erroroutput,"adding onto an existing list\n");
		p=*realhead;
		head=*realhead;
		error(erroroutput,"REAL TEST, filename is %s\n",(*realhead)->filename);
		error(erroroutput,"head isnt null here\n");

		error(erroroutput,"TEST, no is %d, filename is %s\n",*no,p->filename);

		while (p->next != NULL)
			p = p->next;
		}

	fseek(tablefd,fcDggInfo,SEEK_SET);

	while(fulllen < lcbDggInfo)
		{
		error(erroroutput,"at this moment fullen is %d, lcbDggInfo is %d\n",fulllen,lcbDggInfo);
		msofbh = read_16ubit(tablefd);
		element = read_16ubit(tablefd);
		len = read_32ubit(tablefd);

		fulllen+=8;
		error(erroroutput,"len is %x, element is %x\n",len,element);
		error(erroroutput,"dec, fullen is %d, lcbDggInfo is %d\n",fulllen,lcbDggInfo);
		switch (element)
			{
			case 0xF000:
				/*keep going*/
				break;
			case 0xf006:
				error(erroroutput,"1-->%d\n",read_32ubit(tablefd));
				error(erroroutput,"2-->%d\n",read_32ubit(tablefd));
				error(erroroutput,"3-->%d\n",read_32ubit(tablefd));
				noofpics = read_32ubit(tablefd);
				fulllen+=16;
				error(erroroutput,"there are %d pics visible\n",noofpics);
				/*
				for (i=0;i<noofpics;i++)
					{
					read_32ubit(tablefd);
					read_32ubit(tablefd);
					fulllen+=8;
					}
				*/
				for (i=16;i<len;i++)
					{
					getc(tablefd);
					fulllen++;
					}
				break;
			case 0xf001: /*container for pics*/
				break;
			case 0xf002: /*container for page or something*/
				break;
			case 0xf003: /*container for many shapes*/
				break;
			case 0xf004: /*container for a shape*/
				break;
			case 0xf005: /*container for shape rules*/
				break;
			case 0xf007: /*a pic, yeah!!*/
				error(erroroutput,"window pic type is %d\n",getc(tablefd));	
				error(erroroutput,"mac pic type is %d\n",getc(tablefd));	
				fulllen+=2;
				for(i=0;i<18;i++)
					getc(tablefd);
				fulllen+=18;
				error(erroroutput,"pic data len is (%x)\n",read_32ubit(tablefd));
				error(erroroutput,"blid no is %d\n",read_32ubit(tablefd));
				offset = read_32ubit(tablefd);
				fulllen+=12;
				error(erroroutput,"file offset into main stream is %x\n",offset);

				read_32ubit(tablefd);
				fulllen+=4;

				buffer = malloc(strlen("-graphic100-mswv.tiff")+80+strlen(imageprefix));
				if (buffer==NULL)
					{
					fprintf(erroroutput,"no mem\n");
					exit(-1);
					}

				error(erroroutput,"were at %x\n",ftell(tablefd));
				if ((offset != 0xffffffffUL) || (streamtype == 0x01))
					{
					if (streamtype == 0x08)
						fseek(mainfd,offset,SEEK_SET);
					error(erroroutput,"were at %x\n",ftell(mainfd));
					msofbh = read_16ubit(mainfd);
					error(erroroutput,"element is %x, inst is %x\n",msofbh ,msofbh>> 4);
					msofbh = msofbh >> 4;
					for (i=0;i<NOOFIDS;i++)
						{
						if ((msofbh ^ idlist[i]) == 1)
							{
							error(erroroutput,"extra 16\n");
							extra=16;
							}
						}
					element = read_16ubit(mainfd);
					len = read_32ubit(mainfd);
					if (streamtype == 0x01)
						fulllen+=8;
					error(erroroutput,"graphic len is %x\n",len);
					if ((element >= 0xf018) && (element <= 0xf117))
						{
						switch (element - 0xf018)
							{
							case 6:
								sprintf(buffer,"%s-graphic%d-mswv-%d.%s",imageprefix,(*no)+1,streamtype,"png");
								for(i=0;i<(17+extra);i++)
									getc(mainfd);
								len-=(17+extra);
								if (streamtype == 0x01)
									fulllen+=(17+extra);
								break;
							case 5:
								sprintf(buffer,"%s-graphic%d-mswv-%d.%s",imageprefix,(*no)+1,streamtype,"jpg");
								for(i=0;i<(17+extra);i++)
									getc(mainfd);
								len-=(17+extra);
								if (streamtype == 0x01)
									fulllen+=(17+extra);
								break;
							case 4:
								sprintf(buffer,"%s-graphic%d-mswv-%d.%s",imageprefix,(*no)+1,streamtype,"pict");
								for(i=0;i<(17+extra);i++)
									getc(mainfd);
								len-=(17+extra);
								if (streamtype == 0x01)
									fulllen+=(17+extra);
								break;
							case 3:
								sprintf(buffer,"%s-graphic%d-mswv-%d.%s",imageprefix,(*no)+1,streamtype,"wmf.?gz");
								for(i=0;i<16;i++)
									getc(mainfd);
								error(erroroutput,"uncompressed size would be %d\n",read_32ubit(mainfd));
								for(i=0;i<24;i++)
									getc(mainfd);
								error(erroroutput,"compressed size is %d\n",read_32ubit(mainfd));
								error(erroroutput,"compression is %d\n",getc(mainfd));
								error(erroroutput,"filter is %d\n",getc(mainfd));
								len-=50;
								if (streamtype == 0x01)
									fulllen+=50;
								break;
							case 2:
								sprintf(buffer,"%s-graphic%d-mswv-%d.%s",imageprefix,(*no)+1,streamtype,"emf.?gz");
								for(i=0;i<16;i++)
									getc(mainfd);
								error(erroroutput,"uncompressed size would be %d\n",read_32ubit(mainfd));
								for(i=0;i<24;i++)
									getc(mainfd);
								error(erroroutput,"compressed size is %d\n",read_32ubit(mainfd));
								error(erroroutput,"compression is %d\n",getc(mainfd));
								error(erroroutput,"filter is %d\n",getc(mainfd));
								len-=50;
								if (streamtype == 0x01)
									fulllen+=50;
								break;
							default:
								sprintf(buffer,"%s-graphic-dontknow%d-mswv-%d.jpg",imageprefix,(*no)+1,streamtype);
								break;
							}

						names[(*no)++] = buffer;

						error(erroroutput,"name is %s, index %d\n",names[(*no)-1],(*no)-1);

						error(erroroutput,"the current pos of the beast is %x\n",ftell(mainfd));
						graphic = fopen(buffer,"wb");
						if (graphic == NULL)
							{
							fprintf(erroroutput,"warning couldnt create file %s, ignoring \n(failure was :%s)\n",buffer,strerror(errno));
							for (i=0;i<len;i++)
								{
								getc(mainfd);
								if (streamtype == 0x01)
									fulllen++;
								}
							}
						else
							{
							error(erroroutput,"len changed to %x\n",len);
							for (i=0;i<len;i++)
								{
								putc(getc(mainfd),graphic);
								if (streamtype == 0x01)
									fulllen++;
								}
							fclose(graphic);
							}
						}
					error(erroroutput,"blit no %d is filename %s\n",no,names[(*no)-1]);


					if (p != NULL)
						if (p->filename == NULL)
							p->filename = buffer;
					
					error(erroroutput,"THE FILENAME is %s\n",buffer);
					}
				else
					{
					error(erroroutput,"would have been %s-graphic-badoffset%d-mswv.jpg",imageprefix,(*no)+1);
					error(erroroutput,"THE FILENAME is %s\n",buffer);
					free(buffer);
					}
				break;
			case 0xf11e:
				/*i dont quite understand this for now*/
				len=25;
				for(i=0;i<len;i++)
					{
					getc(tablefd);
					fulllen++;
					}
				break;
			case 0xf008:
				read_32ubit(tablefd);	
				read_32ubit(tablefd);	
				fulllen+=8;
				break;
			case 0xf00a:
				spids[currentspid++] = read_32ubit(tablefd);
				error(erroroutput,"The ID of this ENTITY is %x\n",spids[currentspid-1]);
				/*
				this is the identity of the object, the real spid as far 
				as im concerned, following this will be the shape property
				table that will have a list of blits associated with
				this id, (referred to by an index into the previous blit 
				table)
				*/
				flags = read_32ubit(tablefd);
				error(stderr,"flags is %x\n",flags);

				if (head == NULL)
					{
					head = (obj_by_spid*) malloc(sizeof(obj_by_spid));
					error(erroroutput,"head is null\n");
					if (head == NULL)
						{
						error(erroroutput,"no mem\n");
						exit(-1);
						}
					p = head;
					}
				else
					{
					p->next = (obj_by_spid*) malloc(sizeof(obj_by_spid));
					error(erroroutput,"head is not null\n");
					if (p->next == NULL)
						{
						error(erroroutput,"no mem\n");
						exit(-1);
						}
					p = p->next;
					}
				
				p->spid = spids[currentspid-1];
				p->filename = failsafe;
				error(erroroutput,"TEST-->%x %d \n",p->spid,op);
				p->next = NULL;
				(*noofblips)++;
				
				fulllen+=8;
				for(i=0;i<len-8;i++)
					{
					getc(tablefd);
					fulllen++;
					}
				break;
			case 0xf00b:
				id = read_16ubit(tablefd);
				op = read_32ubit(tablefd);
				error(erroroutput,"id is %x\n",id);
				error(erroroutput,"op is %x\n",op);

				if ((id & 0x4000) && (!(id & 0x8000)))
					{
					error(erroroutput,"talking about blit id of %x,mine %x\n",op,id&0x3fff);
					p->filename=names[op-1];
					if (p->filename==NULL)
						{
						/*
						this means that there was no blip in the blip store to match with this,
						from looking at it it looks like this means that there will be data in
						the client data to fill this blank
						so im going to temporarily allow a temp in the filename when we come
						to blips to be assigned
						*/
						}
					}

				if (p==NULL)
					error(erroroutput,"SO p is NULL\n");
				else if (p->filename == NULL)
					error(erroroutput,"SO p->filenamae is NULL\n");
				else
					error(erroroutput,"SO spid %x has a filename of %s\n",p->spid,p->filename);

				fulllen+=6;
				for(i=0;i<len-6;i++)
					{
					getc(tablefd);
					fulllen++;
					}
				break;
			default:
				error(erroroutput,"unrecognized element %x len is %d\n",element,len);
				error(erroroutput,"were here at %x\n",ftell(tablefd));
				for(i=0;i<len;i++)
					{
					getc(tablefd);
					fulllen++;
					}
				break;
			}
		}

	p=head;
	error(erroroutput,"-->no is %d\n",(*no));
	for (i=0;i<(*no);i++)
		{
		error(erroroutput,"-->filename is %s\n",names[i]);
		error(erroroutput,"-->spid is %x\n",spids[i]);
		}
	p=head;
	while (p != NULL)
		{
		error(erroroutput,"-->p->filename is %s\n",p->filename);
		error(erroroutput,"-->p->spid is %x\n",p->spid);
		p = p->next;
		}
	if (imageprefix != NULL)
		free(imageprefix);
	return(head);
	}

void output_draw(U32 cp,textportions *portions)
	{
	int i;
	U16 ourspid=0;
	obj_by_spid *p;
	char *temppointer;


	for (i=0;i<portions->noofficedraw;i++)
		{
		if (cp == portions->officedrawcps[i])
			{
			ourspid = portions->spids[i];
			break;
			}
		}
	
	error(erroroutput,"outspid is %x in output_draw, only able to handle blips, not real draw objects as of yet\n",ourspid);

	error(erroroutput,"noofficedraw is %d portions->noofblipdata is %d\n",portions->noofficedraw,portions->noofblipdata);

	p = portions->ablipdata;

	if ((ourspid != 0) && (p != 0)) /*Craig J Copi <cjc5@po.cwru.edu>*/
		{
		/*search to see if theres a filename assiciated with this spid*/
		for(i=0;i<portions->noofblipdata;i++)
			{
			error(erroroutput,"p->spid is %x\n",p->spid);
			error(erroroutput,"p->filename is %s\n",p->filename);
			if (ourspid == p->spid)
				{
				outputimgsrc(p->filename);
				return;
				}
			p = p->next;
			}
		}
	error(erroroutput,"given spid %x, no luck\n",ourspid);
	}


