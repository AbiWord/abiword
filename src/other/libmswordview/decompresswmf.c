#include "config.h"

#ifdef SYSTEM_ZLIB
#include <zlib.h>
#include <sys/mman.h>
#endif

#include <stdio.h>
#include <string.h>
#include "mswordview.h"

#if 0
#include <unistd.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


extern FILE *outputfile;
extern FILE *erroroutput;

/*
written by thisguy@somewhere.com who doesnt want his name in the source
*/

/*
theres some notes in the notes dir on compression
*/


int decompress(FILE *inputfile,char *outputfile,U32 inlen,U32 outlen)
	{
#ifdef SYSTEM_ZLIB
	char *compr;
	char *uncompr;
	int err;
	uLong uncomprLen, comprLen;
	
	
	char *input,*output;
	int out;
	int in;
	
	

	if (inputfile == NULL)
		{
		fprintf(erroroutput,"danger, file to decompress is NULL\n");
		return(-1);
		}

	in = fileno(inputfile);

	input = mmap(0,inlen,PROT_READ|PROT_WRITE,MAP_SHARED,in,0);

	if (input == (char *)-1)
		{
		fprintf(erroroutput,"unable to mmap inputfile\n");
		return(-1);
		}

	out = creat(outputfile,S_IRUSR|S_IWUSR);
	if (out == -1)
		{
		fprintf(erroroutput,"unable to create %s\n",outputfile);
		munmap(input,inlen);
		exit(-1);
		}
#if 0
	for (i=0;i<outlen;i++)
#else
	lseek(out,outlen,SEEK_SET);
#endif
		if (-1 == write(out,"0",1))
			{
			fprintf(erroroutput,"unable to write is %s\n",outputfile);
			munmap(input,inlen);
			close(out);
			exit(-1);
			}

	close(out);
	out = open(outputfile,O_RDWR);
	if (out == -1)
		{
		fprintf(erroroutput,"unable to open %s\n",outputfile);
		munmap(input,inlen);
		close(out);
		exit(-1);
		}

	output = mmap(0,outlen,PROT_READ|PROT_WRITE,MAP_SHARED,out,0);

	if (output == (char *)-1)
		{
		fprintf(erroroutput,"map out failed\n");
		fprintf(erroroutput,"%s\n",strerror(errno));
		munmap(input,inlen);
		close(out);
		exit(-1);
		}

	/*
	z_verbose = 1;
	*/
	
	/* set the size of the file*/
	comprLen = inlen;

	/* Read in the file contents */
	compr = input;
	uncompr = output;
	if (compr == NULL) 
		{
		fprintf(erroroutput,"no mem to decompress wmf files\n");
		return(-1);
		}
	if (uncompr == NULL) 
		{
		fprintf(erroroutput,"no mem to decompress wmf files\n");
		return(-1);
		}
	
	uncomprLen = outlen;	/* This was the trick :( */
	
	err = uncompress(uncompr, &uncomprLen, compr, comprLen);

	munmap(input,inlen);
	munmap(output,outlen);
	close(out);

	if (err != Z_OK) 
		{
		fprintf(erroroutput, "error: %d\n", err); 
		return(-1); 
		} 
	
#endif
	return 0;
	}
