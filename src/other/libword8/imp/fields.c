/* basically done for now. could use comments. */

S8 read_main_fields(doc *d) {

	U32 i;

	d->main_fields.cps = NULL;
	d->main_fields.flds = NULL;
	d->main_fields.no = -1;

	if (d->fib.lcbPlcffldMom > 0) {
		d->main_fields.no = (d->fib.lcbPlcffldMom-4)/6;
		
		trace("guessing that no of entries is %d\n", d->main_fields.no);
		d->main_fields.cps = (U32*)malloc((d->main_fields.no+1) * sizeof(U32));
		if (d->main_fields.cps == NULL) {
			error("no mem\n");
			return (-1);
		}

		d->main_fields.flds = (U8*)malloc((d->main_fields.no*2) * sizeof(U8));
		if ((d->main_fields.flds == NULL) && (d->fib.lcbPlcffldMom-4 > 0)) {
			error("no mem\n");
			return (-1);
		}

		fseek(tablefd,d->fib.fcPlcffldMom,SEEK_SET);

		for(i = 0; i < (d->main_fields.no+1); i++) {
			d->main_fields.cps[i] = read_32ubit(tablefd);
			trace("field cps are %x\n", d->main_fields.cps[i]);
		}
		for(i = 0; i < (d->main_fields.no*2); i++) {
			d->main_fields.flds[i] = read_8ubit(tablefd);
			trace("field vals are %d\n", d->main_fields.flds[i]);
		}
	}

	return (0);
}

S8 read_header_fields(doc *d) {

	U32 i;

	d->header_fields.cps = NULL;
	d->header_fields.flds = NULL;
	d->header_fields.no = -1;

	if (d->fib.lcbPlcffldHdr> 0) {
		d->header_fields.no = (d->fib.lcbPlcffldHdr-4)/6;

		trace("guessing that no of entries is %d\n", d->header_fields.no);
		d->header_fields.cps = (U32*)malloc((d->header_fields.no+1) * sizeof(U32));
		if (d->header_fields.cps == NULL) {
			error("no mem\n");
			return (-1);
		}

		d->header_fields.flds = (U8*)malloc((d->header_fields.no*2) * sizeof(U8));
		if ((d->header_fields.flds == NULL) && (d->fib.lcbPlcffldHdr-4 > 0)) {
			error("no mem\n");
			return (-1);
		}

		fseek(tablefd,d->fib.fcPlcffldHdr,SEEK_SET);
		
		for(i = 0; i < (d->header_fields.no+1); i++) {
			d->header_fields.cps[i] =  read_32ubit(tablefd);
			trace("header field cps are %x\n",d->header_fields.cps[i]);
		}
		for(i = 0; i < (d->header_fields.no*2); i++) {
			d->header_fields.flds[i] =  read_8ubit(tablefd);
			error("field vals are %d\n",d->header_fields.flds[i]);
		}
	}

	return (0);
}

S8 read_footnote_fields(doc *d) {

	U32 i;

	d->footnote_fields.cps = NULL;
	d->footnote_fields.flds = NULL;
	d->footnote_fields.no = -1;

	if (d->fib.lcbPlcffldFtn >0) {
		d->footnote_fields.no = (d->fib.lcbPlcffldFtn-4)/6;

		trace("guessing that no of entries is %d\n",d->footnote_fields.no);
		d->footnote_fields.cps = (U32*)malloc((d->footnote_fields.no+1) * sizeof(U32));
		if (d->footnote_fields.cps == NULL) {
			error("no mem\n");
			return (-1);
		}

		d->footnote_fields.flds = (U8*)malloc((d->footnote_fields.no*2) * sizeof(U8));
		if ((d->footnote_fields.flds == NULL) && (d->fib.lcbPlcffldFtn-4 > 0)) {
			error("no mem\n");
			return (-1);
		}

		fseek(tablefd,d->fib.fcPlcffldFtn,SEEK_SET);
		
		for(i = 0; i < (d->footnote_fields.no+1); i++) {
			d->footnote_fields.cps[i] =  read_32ubit(tablefd);
			trace("footnote field cps are %x\n",d->footnote_fields.cps[i]);
		}
		for(i = 0; i < (d->footnote_fields.no*2); i++) {
			d->footnote_fields.flds[i] = read_8ubit(tablefd);
			trace("field vals are %d\n",d->footnote_fields.flds[i]);
		}
	}

	return (0);
}

S8 read_annotation_fields(doc *d) {

	U32 i;

	d->annotation_fields.cps = NULL;
	d->annotation_fields.flds = NULL;
	d->annotation_fields.no = -1;

	if (d->fib.lcbPlcffldAtn>0) {
		d->annotation_fields.no = (d->fib.lcbPlcffldAtn-4)/6;

		trace("guessing that no of entries is %d\n",d->annotation_fields.no);
		d->annotation_fields.cps = (U32*)malloc((d->annotation_fields.no+1) * sizeof(U32));
		if (d->annotation_fields.cps == NULL) {
			error("no mem\n");
			return (-1);
		}

		d->annotation_fields.flds = (U8*)malloc((d->annotation_fields.no*2) * sizeof(U8));
		if ((d->annotation_fields.flds == NULL) && (d->fib.lcbPlcffldAtn-4 > 0)) {
			error("no mem\n");
			return (-1);
		}

		fseek(tablefd,d->fib.fcPlcffldAtn,SEEK_SET);
		
		for(i = 0; i < (d->annotation_fields.no+1); i++) {
			d->annotation_fields.cps[i] = read_32ubit(tablefd);
			trace("annotation field cps are %x\n",d->annotation_fields.cps[i]);
		}
		for(i = 0; i < (d->annotation_fields.no*2); i++) {
			d->annotation_fields.flds[i] = read_8ubit(tablefd);
			trace("field vals are %d\n",d->annotation_fields.flds[i]);
		}
	}

	return (0);
}

S8 read_endnote_fields(doc *d) {

	U32 i;
     
	d->endnote_fields.cps = NULL;
	d->endnote_fields.flds = NULL;
	d->endnote_fields.no = -1;

	if (d->fib.lcbPlcffldEdn > 0) {
		d->endnote_fields.no = (d->fib.lcbPlcffldEdn-4)/6;

		trace("guessing that no of entries is %d\n", d->endnote_fields.no);
		d->endnote_fields.cps = (U32*)malloc((d->endnote_fields.no+1) * sizeof(U32));
		if (d->endnote_fields.cps == NULL) {
			error("no mem\n");
			return (-1);
		}

		d->endnote_fields.flds = (U8*)malloc((d->endnote_fields.no*2) * sizeof(U8));
		if ((d->endnote_fields.flds == NULL) && (d->fib.lcbPlcffldEdn-4 > 0)) {
			error("no mem\n");
			return (-1);
		}

		fseek(tablefd,d->fib.fcPlcffldEdn,SEEK_SET);
		
		for(i = 0; i < ((d->endnote_fields.no+1); i++) {
			d->endnote_fields.cps[i] =  read_32ubit(tablefd);
			trace("endnote field cps are %x\n",d->endnote_fields.cps[i]);
		}
		for(i = 0; i < (d->endnote_fields.no*2); i++) {
			d->endnote_fields.flds[i] =  read_8ubit(tablefd);
			trace("field vals are %d\n",d->endnote_fields.flds[i]);
		}
	}

	return (0);
}
