/* basically done for now, could use comments */

#include "word8.h"

S8 read_footnote_info(doc *d) {

	d->footnote_ref_num = 0;
	d->footnote_ref = NULL;
	d->footnote_FRD = NULL;
	d->footnote_TrueFRD = NULL;
	d->footnote_txt_num = 0;
	d->footnote_txt = NULL;
	d->footnote_list_num = 0;
	d->footnote_auto = 1;
	d->footnote_last = 0;
     
	if (d->fib.lcbPlcffndRef != 0) {
		d->footnote_ref_num = (d->fib.lcbPlcffndRef-4)/6;
		d->footnote_ref = (U32*)malloc((d->footnote_ref_num+1) * sizeof(U32));
		if (d->footnote_ref == NULL) {
			error("NO MEM 1\n");
			return(-1);
		}

		d->footnote_FRD = (S16*)malloc(d->footnote_ref_num * sizeof(S16));
		d->footnote_TrueFRD = (S16*)malloc(d->footnote_ref_num * sizeof(S16));
		if ((d->footnote_FRD == NULL) || (d->footnote_TrueFRD == NULL)) {
			error("NO MEM 2\n");
			return(-1);
		}
		
		fseek(tablefd,d->fib.fcPlcffndRef,SEEK_SET);
		for(i = 0; i < d->footnote_ref_num + 1; i++) {
			d->footnote_ref[i]=read_32ubit(tablefd);
			trace("footnote ref is %x\n", d->footnote_ref[i]);
		}
		for(i = 0; i < d->footnote_ref_num + 1; i++) {
			d->footnote_TrueFRD[i] = d->footnote_FRD[i] = 
				(S16)read_16ubit(tablefd);
			trace("footnote FRD is %d\n", d->footnote_FRD[i]);
		}
	}

	if (d->fib.lcbPlcffndTxt != 0) {
		d->footnote_txt_num = d->fib.lcbPlcffndTxt/4;
		d->footnote_txt = (U32*)malloc(d->footnote_txt_num * sizeof(U32));
		if (d->footnote_txt == NULL) {
			error("NO MEM 3\n");
			return(-1);
		}
		
		fseek(tablefd,d->fib.fcPlcffndTxt,SEEK_SET);
		for (i = 0; i < d->footnote_txt_num; i++) {
			d->footnote_txt[i] = read_32ubit(tablefd);
			trace("footnote->%x\n", d->footnote_txt[i]);
		}
	}

	return(0);
}

S8 read_endnote_info(doc *d) {

	d->endnote_ref_num = 0;
	d->endnote_ref = NULL;
	d->endnote_FRD = NULL;
	d->endnote_TrueFRD = NULL;
	d->endnote_txt_num = 0;
	d->endnote_txt = NULL;
	d->endnote_list_num = 0;
	d->endnote_auto = 1;
	d->endnote_last = 0;

	
	if (d->fib.lcbPlcfendRef != 0) {
		d->endnote_ref_num = (d->fib.lcbPlcfendRef-4)/6;
		d->endnote_ref = (U32*)malloc((d->endnote_ref_num+1) * sizeof(U32));
		if (d->endnote_ref == NULL) {
			error("NO MEM 1\n");
			return(-1);
		}

		d->endnote_FRD = (S16*)malloc(d->endnote_ref_num * sizeof(S16));
		d->endnote_TrueFRD = (S16*)malloc(d->endnote_ref_num * sizeof(S16));
		if ((d->endnote_FRD == NULL) || (d->endnote_TrueFRD == NULL)) {
			error("NO MEM 2\n");
			return(-1);
		}
		
		fseek(tablefd,d->fib.fcPlcfendRef,SEEK_SET);
		for(i = 0; i < d->endnote_ref_num + 1; i++) {
			d->endnote_ref[i]=read_32ubit(tablefd);
			trace("endnote ref is %x\n", d->endnote_ref[i]);
		}
		for(i = 0; i < d->endnote_ref_num + 1; i++) {
			d->endnote_TrueFRD[i] = d->endnote_FRD[i] = 
				(S16)read_16ubit(tablefd);
			trace("endnote FRD is %d\n", d->endnote_FRD[i]);
		}
	}

	if (d->fib.lcbPlcfendTxt != 0) {
		d->endnote_txt_num = d->fib.lcbPlcfendTxt/4;
		d->endnote_txt = (U32*)malloc(d->endnote_txt_num * sizeof(U32));
		if (d->endnote_txt == NULL) {
			error("NO MEM 3\n");
			return(-1);
		}
		
		fseek(tablefd,d->fib.fcPlcfendTxt,SEEK_SET);
		for (i = 0; i < d->endnote_txt_num; i++) {
			d->endnote_txt[i] = read_32ubit(tablefd);
			trace("endnote->%x\n", d->endnote_txt[i]);
		}
	}

	return(0);
}

S8 read_annotation_info(doc *d) {

	d->annotation_ref_num = 0;
	d->annotation_ref = NULL;
	d->annotation_atrd = NULL;
	d->annotation_txt_num = 0;
	d->annotation_txt = NULL;
	d->annotation_list_num = 0;

	if (d->fib.lcbPlcfandRef != 0) {
		d->annotation_ref_num = (d->fib.lcbPlcfandRef-4)/34;
		d->annotation_ref = (U32*)malloc((d->annotation_ref_num+1) * sizeof(U32));
		d->annotation_atrd = (ATRD*)malloc(d->annotation_ref_num * sizeof(ATRD));
		if ((d->annotation_ref == NULL) || (d->annotation_atrd == NULL)) {
			error("NO MEM 3\n");
			return(-1);
		}
		
		fseek(tablefd,d->fib.fcPlcfandRef,SEEK_SET);
		for (i = 0; i < d->annotation_ref_num + 1; i++) {
			d->annotation_ref[i] = read_32ubit(tablefd);
			trace("annotation to be found at %x\n", d->annotation_ref[i]);
		}
		for (i = 0; i < d->annotation_ref_num; i++) {
			for (j = 0; j < 10; j++) 
				d->annotation_atrd[i].xstUsrInitl[j] = read_16ubit(tablefd);
			d->annotation_atrd[i].ibst = read_16ubit(tablefd);
			d->annotation_atrd[i].ak = read_16ubit(tablefd);
			d->annotation_atrd[i].grfbmc = read_16ubit(tablefd);
			d->annotation_atrd[i].lTagBkmk = read_32ubit(tablefd);
		}
		/* I've ignored the ANLD structure for now */
	}

	if (d->fib.lcbPlcfandTxt != 0) {
		d->annotation_txt_num = d->fib.lcbPlcfandTxt/4;
		d->annotation_txt = (U32*)malloc(d->annotation_txt_num * sizeof(U32));
		if (d->annotation_txt == NULL) {
			error("NO MEM 3\n");
			return(-1);
		}
		
		fseek(tablefd,d->fib.fcPlcfandTxt,SEEK_SET);
		for (i = 0; i < d->annotation_txt_num; i++) {
			d->annotation_txt[i] = read_32ubit(tablefd);
			trace("annotation text are %x\n", d->annotation_txt[i]);
		}
	}
	
	return (0);
}

S8 read_section_info(doc *d) {

	d->section_cps = NULL;
	d->section_fcs = NULL;
	
	if (d->fib.lcbPlcfsed >0)
		{
		d->section_nos = (d->fib.lcbPlcfsed-4)/16;
		trace("there are %d sections",d->section_nos);
		d->section_cps = (U32*)malloc((d->section_nos+1) * sizeof(U32));
		d->section_fcs = (U32*)malloc((d->section_nos) * sizeof(U32));
		if ((d->section_cps == NULL) || (d->section_fcs ==  NULL)) {
			error("no mem for section_cps\n");
			return(-1);
		}
		
		fseek(tablefd,d->fib.fcPlcfsed,SEEK_SET);
		for (i = 0; i < d->section_nos + 1; i++) {
			d->section_cps[i]=read_32ubit(tablefd);
			trace("section offsets are %x\n", d->section_cps[i]);
		}
		for (i = 0; i < d->section_nos; i++) {
			read_16ubit(tablefd); /*internal*/
			d->section_fcs[i] = read_32ubit(tablefd);
			trace("section file offsets are %x\n", d->section_fcs[i]);
			read_16ubit(tablefd); /*internal*/
			read_32ubit(tablefd);
		}
	}

	return (0);
}
