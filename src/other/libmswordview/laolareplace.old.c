
/*
Released under GPL, written by 
	Andrew Scriven <andy.scriven@research.natpower.co.uk>

Copyright (C) 1998
	Andrew Scriven
*/

/*
-----------------------------------------------------------------------
Andrew Scriven
Research and Engineering
Electron Building, Windmill Hill, Whitehill Way, Swindon, SN5 6PB, UK
Phone (44) 1793 896206, Fax (44) 1793 896251
-----------------------------------------------------------------------


The interface to OLEdecode now has
  int OLEdecode(char *filename, FILE **mainfd, FILE **tablefd0, FILE 
**tablefd1,FILE **data)	
*/

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ctype.h>
#include <sys/types.h>
#include <assert.h>

#include "config.h"
#include "mswordview.h"

#define THEMIN(a,b) ((a)<(b) ? (a) : (b))

#define MAXBLOCKS 256

extern FILE *erroroutput;

struct pps_block
  {
  char name[64];
  char type;
  struct pps_block *previous;
  struct pps_block *next;
  struct pps_block *directory;
  S32 start;
  S32 size;
  int level;
  };

typedef struct pps_block pps_entry;

/* Routine prototypes */
unsigned short int ShortInt(unsigned char* array);
U32 LongInt(unsigned char* array);

unsigned short int ShortInt(unsigned char* array)
{
/*
union two_byte {
 unsigned short int num;
 char  ch[2];
 } Short;

#ifndef INTEL
  Short.ch[1] = *array++;
  Short.ch[0] = *array;
#else
  Short.ch[0] = *array++;
  Short.ch[1] = *array;
#endif
return Short.num;
*/
return sread_16ubit(array);

}

U32 LongInt(unsigned char* array)
{
return sread_32ubit(array);
}

pps_entry **pps_list=NULL;
char *SDepot=NULL;

/* recurse to follow forward/backward list of root pps's */
void unravel(pps_entry *pps_node)
{
  if (pps_node != NULL)
  	{
	if (pps_node->previous != NULL) 
		unravel(pps_node->previous);
	pps_node->level = 0;
	if(pps_node->next != NULL) 
		unravel(pps_node->next);
	}
}

int myOLEdecode(char *filename, FILE **mainfd, FILE **tablefd0, FILE
**tablefd1,FILE **data)
{
  FILE *input=NULL;
  FILE *OLEfile=NULL;
  FILE *sbfile=NULL;
  FILE *infile=NULL;
  
#ifdef DEBUG
  int debug=1;
#else
  int debug=0;
#endif
  int BlockSize=0,Offset=0;
  int c,i,j,len,bytes;
  char *s,*p,*t;
  char *Block,*BDepot,*Depot,*Root;
  
  char Main[]="WordDocument";
  char Table0[]="0Table";
  char Table1[]="1Table";
  char Data[]="Data";
  U32 FilePos=0x00000000;
  S32 num_bbd_blocks;
  S32 root_list[MAXBLOCKS], sbd_list[MAXBLOCKS];
  S32 pps_size,pps_start=-1;
  S32 linkto;
  int root_entry=-1;
  S32 fullen;
  S32 temppos;
  


  *mainfd = NULL;
  *tablefd0 = NULL;
  *tablefd1 = NULL;
  input = fopen(filename,"rb");
  if(input == NULL) return 4;
  /* peek into file to guess file type */
  c=getc(input);
  ungetc(c,input);

  if(isprint(c)) {
     fprintf(erroroutput,"File looks like a plain text file.\n");
     return 8;
  /* check for MS OLE wrapper */
  } else if(c==0xd0) {
     Block = malloc(512);
	 if (Block == NULL)
	 	{
       	fprintf(erroroutput,"1 ===========> probable corrupt ole file, unable to allocate %d bytes\n",512);
		return(5);
		}
     /* read header block */
     if(fread(Block,512,1,input)!=1) {
       fprintf(erroroutput,"1 ===========> Input file has faulty OLE format\n");
    return 5;
     }
     num_bbd_blocks=(S32)LongInt(Block+0x2c);
	 if ((num_bbd_blocks == 0) || (num_bbd_blocks < 0))
		{
       	fprintf(erroroutput,"2 ===========> Input file has ridiculous bbd, mem for the depot was %d\n",512*num_bbd_blocks);
		return(5);
		}
     BDepot = malloc(512*num_bbd_blocks);
	 if (BDepot == NULL)
	 	{
       	fprintf(erroroutput,"2 ===========> couldnt alloc ole mem for the depot of %d\n",512*num_bbd_blocks);
		return(5);
		}
     s = BDepot;
     root_list[0]=LongInt(Block+0x30);
     sbd_list[0]=(S16)LongInt(Block+0x3c);
    if(debug) fprintf(erroroutput,"num_bbd_blocks %d, root start %d, sbd start %d\n",num_bbd_blocks,root_list[0],sbd_list[0]);
	temppos = ftell(input);
	fseek(input,0,SEEK_END);
	fullen = ftell(input);
	fseek(input,temppos,SEEK_SET);

     /* read big block Depot */
     for(i=0;i<(int)num_bbd_blocks;i++) 
	 	{
		if (0x4c+(i*4) > 512)
			{
			fprintf(erroroutput,"2.1 ===========> Input file has faulty bbd\n");
			return 5;
			}
       	FilePos = 512*(LongInt(Block+0x4c+(i*4))+1);
		if (FilePos > fullen)
			{
			fprintf(erroroutput,"2.2 ===========> Input file has faulty bbd\n");
			return 5;
			}
       	fseek(input,FilePos,SEEK_SET);
       	if(fread(s,512,1,input)!=1) 
			{
			fprintf(erroroutput,"2.3 ===========> Input file has faulty bbd\n");
			return 5;
			}
       	s += 0x200;
     	}

     /* Extract the sbd block list */
     for(len=1;len<MAXBLOCKS;len++)
	 	{
		if (((sbd_list[len-1]*4) < (512*num_bbd_blocks))  && ((sbd_list[len-1]*4) > 0))
			sbd_list[len] = LongInt(BDepot+(sbd_list[len-1]*4));
		else
			{
         	fprintf(erroroutput,"3 ===========> Input file has faulty OLE format\n");
			return(5);
			}
		if(sbd_list[len]==-2) break;
     	}
     SDepot = malloc(512*len);
	 if (SDepot== NULL)
	 	{
       	fprintf(erroroutput,"1 ===========> probable corrupt ole file, unable to allocate %d bytes\n",512*len);
		return(5);
		}
     s = SDepot;
     /* Read in Small Block Depot */
     for(i=0;i<len;i++) {
       FilePos = 512 *(sbd_list[i]+1);
       fseek(input,FilePos,SEEK_SET);
       if(fread(s,512,1,input)!=1) {
         fprintf(erroroutput,"3 ===========> Input file has faulty OLE format\n");
         return 5;
       }
       s += 0x200;
     }
     /* Extract the root block list */
     for(len=1;len<MAXBLOCKS;len++)
		{
		if ( ((root_list[len-1]*4) >= (512*num_bbd_blocks)) || ( (root_list[len-1]*4) < 0) )
			{
         	fprintf(erroroutput,"3.1 ===========> Input file has faulty OLE format\n");
			return(5);
			}
		root_list[len] = LongInt(BDepot+(root_list[len-1]*4));
		if(root_list[len]==-2) break;
		}
     Root = malloc(512*len);
	 if (Root == NULL)
	 	{
       	fprintf(erroroutput,"1 ===========> probable corrupt ole file, unable to allocate %d bytes\n",512*len);
		return(5);
		}
     s = Root;
     /* Read in Root stream data */
     for(i=0;i<len;i++) {
       FilePos = 512 *(root_list[i]+1);
       fseek(input,FilePos,SEEK_SET);
       if(fread(s,512,1,input)!=1) {
         fprintf(erroroutput,"4 ===========> Input file has faulty OLE format\n");
         return 5;
       }
       s += 0x200;
     }

     /* assign space for pps list */
     pps_list = malloc(len*4*sizeof(pps_entry *));
	 if (pps_list == NULL)
	 	{
       	fprintf(erroroutput,"1 ===========> probable corrupt ole file, unable to allocate %d bytes\n",len*4*sizeof(pps_entry *));
		return(5);
		}
     for(j=0;j<len*4;j++) 
	 	{
	 	pps_list[j] = malloc(sizeof(pps_entry));
		if (pps_list[j] == NULL)
			{
			fprintf(erroroutput,"1 ===========> probable corrupt ole file, unable to allocate %d bytes\n",sizeof(pps_entry));
			return(5);
			}
		}
     /* Store pss entry details and look out for Root Entry */
     for(j=0;j<len*4;j++) {
       pps_list[j]->level = -1;
       s = Root+(j*0x80);
       i=ShortInt(s+0x40);
	   if (((j*0x80) + i) >= (512 * len))
	   	{
		fprintf(erroroutput,"1.1 ===========> probable corrupt ole file\n");
		return(5);
		}
       for(p=pps_list[j]->name,t=s;t<s+i;t++) 
	   	*p++ = *t++;
       s+=0x42;
       pps_list[j]->type = *s;
       if(pps_list[j]->type == 5) {
         root_entry = j; /* this is root */
         pps_list[j]->level = 0;
       }
       s+=0x02;
       linkto = LongInt(s);
       if ((linkto != -1) && (linkto < (len*4)) && (linkto > -1))
	   	pps_list[j]->previous = pps_list[linkto];
       else pps_list[j]->previous = NULL;
       s+=0x04;
       linkto = LongInt(s);
       if ((linkto != -1) && (linkto < (len*4)) && (linkto > -1))
	   	pps_list[j]->next = pps_list[linkto];
       else pps_list[j]->next = NULL;
       s+=0x04;
       linkto = LongInt(s);
       if ((linkto != -1) && (linkto < (len*4)) && (linkto > -1))
	   	pps_list[j]->directory = pps_list[linkto];
       else pps_list[j]->directory = NULL;
       s+=0x28;
       pps_list[j]->start = LongInt(s);
       s+=0x04;
       pps_list[j]->size = LongInt(s);
     }

     /* go through the pps entries, tagging them with level number
        use recursive routine to follow list starting at root entry */
     unravel(pps_list[root_entry]->directory);

     /* go through the level 0 list looking for named entries */
     for(j=0;j<len*4;j++) {
       if(pps_list[j]->level != 0) continue; /* skip nested stuff */
       pps_start = pps_list[j]->start;
       pps_size  = pps_list[j]->size;
       OLEfile = NULL;
       if(pps_list[j]->type==5) {  /* Root entry */
         OLEfile = tmpfile();
         sbfile = OLEfile;
         if(debug) fprintf(erroroutput,"Reading sbFile %d\n",pps_start);
       }
       else if(!strcmp(pps_list[j]->name,Main)) {
         OLEfile = tmpfile();
         *mainfd = OLEfile;
         if(debug) fprintf(erroroutput,"Reading Main %d\n",pps_start);
       }
       else if(!strcmp(pps_list[j]->name,Table0)) {
         OLEfile = tmpfile();
         *tablefd0 = OLEfile;
         if(debug) fprintf(erroroutput,"Reading Table0 %d\n",pps_start);
       }
       else if(!strcmp(pps_list[j]->name,Table1)) {
         OLEfile = tmpfile();
         *tablefd1 = OLEfile;
         if(debug) fprintf(erroroutput,"Reading Table1 %d\n",pps_start);
       }
       else if(!strcmp(pps_list[j]->name,Data)) {
         OLEfile = tmpfile();
         *data = OLEfile;
         if(debug) fprintf(erroroutput,"Reading Data %d\n",pps_start);
       }
       if(pps_size<=0) OLEfile = NULL;
       if(OLEfile == NULL) continue;
	   /* 
       if (pps_size>=4096 | OLEfile==sbfile) 
	   */
       if ((pps_size>=4096) || (OLEfile==sbfile))
	   {
         Offset = 1;
         BlockSize = 512;
         infile = input;
         Depot = BDepot;
       } else {
         Offset = 0;
         BlockSize = 64;
         infile = sbfile;
         Depot = SDepot;
       }
       while(pps_start != -2) {
         if(debug) fprintf(erroroutput,"Reading block %d\n",pps_start);
         FilePos = (pps_start+Offset)* BlockSize;
         bytes = THEMIN(BlockSize,pps_size);
         fseek(infile,FilePos,SEEK_SET);
         if(fread(Block,bytes,1,infile)!=1) {
           fprintf(erroroutput,"5 ===========> Input file has faulty OLE format\n");
           return(5);
         }
         fwrite(Block,bytes,1,OLEfile);
         pps_start = LongInt(Depot+(pps_start*4));
         pps_size -= BlockSize;
         if(pps_size <= 0) pps_start=-2;
       }
       rewind(OLEfile);
     }
    free(Root);
    free(BDepot);
    free(Block);
    fclose(input);
    return 0;
  } else {
    /* not a OLE file! */
    fprintf(erroroutput,"7 ===========> Input file is not an OLE file\n");
    return 8;
  }
}

void myfreeOLEtree(void)
    {
	if (SDepot != NULL)
		free(SDepot);
	}

