#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "config.h"
#include "mswordview.h"

extern FILE *erroroutput;
extern FILE *outputfile;
extern long int cp;
extern int insuper;

ATRD *key_atrd;

void decode_annotation(textportions *portions, FILE *main)
	{
	int i,j;
	/*
	search in the first thing for the cp of this
	reference.
	*/
	i=0;
	while(i<portions->andref_no)
		{
		if (portions->andRef[i] == cp)
			{
			error(erroroutput,"found correct anno ref pos at %x, index was %d\n",portions->andRef[i],i);
			if (!insuper)
				fprintf(outputfile,"<sup>");
			fprintf(outputfile,"<a href=\"#");
			j = portions->the_atrd[i].xstUsrInitl[0];
			for (j=1;j<portions->the_atrd[i].xstUsrInitl[0]+1;j++)
                    {
                    /*warning despite the possibility of being 16 bit nos ive done this*/
                    fprintf(outputfile,"%c",portions->the_atrd[i].xstUsrInitl[j]);
                    }
			fprintf(outputfile,"%d\">",i);
			j = portions->the_atrd[i].xstUsrInitl[0];
			key_atrd[portions->the_atrd[i].ibst].xstUsrInitl[0] = j;
			key_atrd[portions->the_atrd[i].ibst].ibst = portions->the_atrd[i].ibst;
			for (j=1;j<portions->the_atrd[i].xstUsrInitl[0]+1;j++)
                    {
                    /*warning despite the possibility of being 16 bit nos ive done this*/
                    fprintf(outputfile,"%c",portions->the_atrd[i].xstUsrInitl[j]);
					key_atrd[portions->the_atrd[i].ibst].xstUsrInitl[j] = portions->the_atrd[i].xstUsrInitl[j];
                    }
			fprintf(outputfile,"%d</a>",i+1);

			if (!insuper)
				fprintf(outputfile,"</sup>");
			break;
			}
		i++;
		}

	/*part deux*/
	/*
	now to hold off to the very end any spray them all out, or at the end of every page.
	ill do the latter for the moment
	*/
	if ( (i < portions->andref_no ) && (i < 256) )
		portions->list_annotations[portions->list_anno_no++] = i;
	else if (i >= 256)
		fprintf(erroroutput,"oops silly programmer :-) lost an annotation\n");
	}


stringgroup *extract_authors(FILE *tablefd,U32 fcGrpXstAtnOwners,U32 lcbGrpXstAtnOwners)
	{
	U16 len,i;
	U32 count=0;
	stringgroup *authorlist=NULL;
	stringgroup *current=NULL;

	if (lcbGrpXstAtnOwners > 0)
		{
		fseek(tablefd,fcGrpXstAtnOwners,SEEK_SET);
		authorlist = (stringgroup*) malloc(sizeof(stringgroup));

		if (authorlist == NULL)
			{
			fprintf(erroroutput,"not enough mem for annotation group\n");
			return(NULL);
			}

		authorlist->next = NULL;
		authorlist->author = NULL;
		authorlist->noofstrings=0;
		current = authorlist;

		while (count < lcbGrpXstAtnOwners)
			{
			len = read_16ubit(tablefd);
			count+=2;
			current->author = malloc((len+1) * sizeof(U16));
			authorlist->noofstrings++;
			if (current->author == NULL)
				{
				fprintf(erroroutput,"not enough mem for author string of len %d\n",len);
				break;
				}
			for (i=0;i<len;i++)
				{
				current->author[i] = read_16ubit(tablefd);
				count+=2;
				}
			current->author[i] = '\0';

			if (count < lcbGrpXstAtnOwners)
				{
				current->next= (stringgroup*) malloc(sizeof(stringgroup));
				if (current->next == NULL)
					{
					fprintf(erroroutput,"not enough mem for annotation group\n");
					break;
					}
				current = current->next;
				current->next = NULL;
				current->author = NULL;
				}
			}
		}
	return(authorlist);
	}


void decode_annotations(FILE *mainfd,FILE *tablefd,textportions *portions)
	{
	int i;
	U32 fcSttbfAtnbkmk,lcbSttbfAtnbkmk;
	
	U32 fcPlcfAtnbkf,lcbPlcfAtnbkf;
	U32 fcPlcfAtnbkl,lcbPlcfAtnbkl;
	

	fseek(mainfd,0x01C2,SEEK_SET);
	fcSttbfAtnbkmk = read_32ubit(mainfd);
	lcbSttbfAtnbkmk = read_32ubit(mainfd);
	error(erroroutput,"table offset for annotations is %x %d\n",fcSttbfAtnbkmk,lcbSttbfAtnbkmk);

	extract_sttbf(&(portions->annotations),tablefd,fcSttbfAtnbkmk,lcbSttbfAtnbkmk);

	fseek(mainfd,0x01EA,SEEK_SET);

	fcPlcfAtnbkf=read_32ubit(mainfd);
	lcbPlcfAtnbkf=read_32ubit(mainfd);
	fcPlcfAtnbkl=read_32ubit(mainfd);
	lcbPlcfAtnbkl=read_32ubit(mainfd);
	error(erroroutput,"fcPlcfAtnbkf %x lcbPlcfAtnbkf %d\n",fcPlcfAtnbkf,lcbPlcfAtnbkf);
	error(erroroutput,"fcPlcfAtnbkl %x lcbPlcfAtnbkl %d\n",fcPlcfAtnbkl,lcbPlcfAtnbkl);

	extract_bookm_limits(&( portions->a_bookmarks),tablefd,fcPlcfAtnbkf,lcbPlcfAtnbkf,fcPlcfAtnbkl,lcbPlcfAtnbkl);

	key_atrd=NULL;
	if (portions->authors != NULL)
		{
		error(erroroutput,"no of strings here is %d\n",portions->authors->noofstrings);
		key_atrd = (ATRD *) malloc(sizeof(ATRD) * portions->authors->noofstrings);
		if (key_atrd == NULL)
			{
			fprintf(erroroutput,"mem alloc error\n");
			return;
			}
		for (i=0;i<portions->annotations.no_of_strings;i++)
			{
			key_atrd->ibst = i;
			key_atrd->xstUsrInitl[0] = 0;
			}
		}
	}
