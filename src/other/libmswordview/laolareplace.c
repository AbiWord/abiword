/*
The interface to myOLEdecode now has
  int OLEdecode(char *filename, FILE **mainfd, FILE **tablefd0, FILE 
**tablefd1,FILE **data)	
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "mswordview.h"
#include "oledecod.h"

extern FILE*erroroutput;

pps_entry *stream_tree;

void myfreeOLEtree(void)
	{
	/* need to free all the allocated memory */
	freeOLEtree (stream_tree);
	}


int myOLEdecode(char *filename, FILE **mainfd, FILE **tablefd0, FILE **tablefd1,FILE **data)
	{
	int result;
	U32 root_stream;
	U32 stream;

	result = OLEdecode (filename, &stream_tree, &root_stream, 1);
	if (result == 0)
		{
		for (stream = stream_tree[root_stream].dir; stream != 0xffffffff; stream = stream_tree[stream].next)
			{
			if (stream_tree[stream].type != 1 && stream_tree[stream].level == 1)
				{
				if (!(strcmp(stream_tree[stream].name,"WordDocument")))
					{
					*mainfd = fopen(stream_tree[stream].filename,"rb");
					}
				else if (!(strcmp(stream_tree[stream].name,"1Table")))
					{
					*tablefd1 = fopen(stream_tree[stream].filename,"rb");
					}
				else if (!(strcmp(stream_tree[stream].name,"0Table")))
					{
					*tablefd0 = fopen(stream_tree[stream].filename,"rb");
					}
				else if (!(strcmp(stream_tree[stream].name,"Data")))
					{
					*data = fopen(stream_tree[stream].filename,"rb");
					}
				}
			}
		}
	switch(result)
		{
		case 5:
			fprintf(erroroutput,"OLE file appears to be corrupt, unable to extract streams\n");
			break;
		}

	return(result);
	}
