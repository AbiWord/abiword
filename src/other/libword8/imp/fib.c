/* 
 * just about entirely converted, with the exception of the streams issue (see README)
 * also, we'll probably want to move the extraneous import functions out, since they
 * don't really have anything to do with the FIB directly
 */

S8 read_fib(doc *d) {

	U8 fields; /* for parsing bitfields */

	/* read file info */
	d->fib.wIdent=read_16ubit(mainfd);
	d->fib.nFib=read_16ubit(mainfd);
	d->fib.nProduct=read_16ubit(mainfd);

	if (nFib >= 101) trace("written by word >= 6, exactly %x\n", d->fib.nProduct);
	else trace("written by word < 6, exactly %x\n", d->fib.nProduct);

	/* is this a Word 8 doc? */
	if ((d->fib.nProduct >> 8) == 0xe0) {
		error("this is an unsupported word 7 doc, sorry\nthis converter is solely for word8 at the moment\ntry laola (http://wwwwbs.cs.tu-berlin.de/~schwartz/pmh/laola.html) it might get some text out of it\n");
		return(10);
	} else if ((nProduct >> 8) == 0xc0) {
		error("this is an unsupported (as of yet) word 6 doc, sorry\nthis converter is solely for word8 at the moment\ntry word2x (http://word2x.astra.co.uk/) for that\n");
		return(10);
	} else if ((nProduct >> 8) != 0x00) {
		/* what should we do here? */
		trace("this doesnt appear to be a word 8 doc, but going ahead anyway, expect disaster!!\n");
	}

	d->fib.lid=read_16ubit(mainfd);
	d->fib.pnNext=read_16ubit(mainfd);
	
	trace("wIdent = %d\nnFib = %d\nnProduct = %d\nlid = %d\npnNext? = %d\n",
		d->fib.wIdent, d->fib.nFib, d->fib.nProduct, d->fib.lid, d->fib.pnNext);

	fields = read_8ubit(mainfd);
	d->fib.fFields.fDot = fields & 0x01;
	d->fib.fFields.fGlsy = fields & 0x02;
	d->fib.fFields.fComplex = fields & 0x04;
	d->fib.fFields.fHasPic = fields & 0x08;
     fields &= 0xf0;
	d->fib.fFields.cQuickSaves = fields >> 4;

	if (d->fib.fFields.fDot) trace("fDot\n");
	if (d->fib.fFields.fGlsy) trace("fGlsy\n");
	if (d->fib.fFields.fComplex) trace("fComplex\n");
	if (d->fib.fFields.fHasPic) trace("fHasPic\n");
	trace("cQuickSave = %d\n", d->fib.fFields.cQuickSaves);

	fields = read_8ubit(mainfd);
     d->fib.fFields.fEncrypted = fields & 0x01;
	d->fib.fFields.fWhichTblStream = fields & 0x02;
	d->fib.fFields.fReadOnlyRecommended = fields & 0x04;
	d->fib.fFields.fWriteReservation = fields & 0x08;
	d->fib.fFields.fExtChar = fields & 0x10;
	d->fib.fFields.fLoadOverride = fields & 0x20;
	d->fib.fFields.fFarEast = fields & 0x40;
	d->fib.fFields.fCrypto = fields & 0x80;
	
	/* check for an encrypted file, which this library doesn't support */
	if (d->fib.fFields.fEncrypted) {
		error("This file is encrypted, mswordview currently cannot handle encrypted files\n");
		return(10);
	}

	/* select the appropriate table stream to use (either 0Table or 1Table) */
	if (d->fib.fFields.fWhichTblStream) {
		d->streams.table = d->streams.table1;
		trace("use 1Table stream\n");
	} else {
		d->streams.table = d->streams.table0;
		trace("use 0Table stream\n");
	}

	if (d->fib.fFields.fReadOnlyRecommended) trace("fReadOnlyRecommended\n");
	if (d->fib.fFields.fWriteReservation) trace("fWriteReservation\n");
	if (d->fib.fFields.fExtChar) trace("fExtChar\n");
	if (d->fib.fFields.fLoadOverride) trace("fLoadOverride\n");
	if (d->fib.fFields.fFarEast) trace("fFarEast\n");
	if (d->fib.fFields.fCrypto) trace("fCrypto\n");

	d->fib.nFibBack = read_16ubit(mainfd);
	d->fib.lKey = read_32ubit(mainfd);
	
	trace("nFibBack = %d\nlKey = %ld\n", d->fib.nFibBack, d->fib.lKey);

	d->fib.envr = read_8ubit(mainfd);
	
	if (d->fib.envr == 0) trace("created in WinWord\n");
	else if (d->fib.envr == 1) trace("created in MacWord\n");
	
	d->fib.fMac = read_8ubit(mainfd);
	d->fib.chs = read_16ubit(mainfd);
	
	if (d->fib.chs == 0) trace("windows ansi\n");
	else if (d->fib.chs == 256) trace("mac char set\n");
	
	d->fib.chsTables = read_16ubit(mainfd);
	
	if (d->fib.chsTables == 0) trace("windows ansi\n");
	else if (d->fib.chsTables == 256) trace("mac char set\n");

	d->fib.fcMin = read_32ubit(mainfd);
	d->fib.fcMac = read_32ubit(mainfd);

	trace("first char at %ld (%x)\n", d->fib.fcMin, d->fib.fcMin);
	trace("last char at %ld (%x)\n", d->fib.fcMac, d->fib.fcMac);

	fseek(mainfd,0x0040,SEEK_SET);
	
	d->fib.cbMac = read_32ubit(mainfd);
	d->fib.lProductCreated = read_32ubit(mainfd);
	d->fib.lProductRevised = read_32ubit(mainfd);

     d->fib.ccpText = read_32ubit(mainfd);
	d->fib.ccpFtn = read_32ubit(mainfd);
	d->fib.ccpHdd = read_32ubit(mainfd);
	d->fib.ccpMcr = read_32ubit(mainfd);
	d->fib.ccpAtn = read_32ubit(mainfd);
	d->fib.ccpEdn = read_32ubit(mainfd);
	d->fib.ccpTxbx = read_32ubit(mainfd);
	d->fib.ccpHdrTxbx = read_32ubit(mainfd);
	
	trace("the main doc text is of size %ld\n", d->fib.ccpText);
	trace("the footer text is of size %ld\n", d->fib.ccpFtn);
	trace("the header text is of size %ld\n", d->fib.ccpHdd);
	trace("the annotation text is size %ld\n", d->fib.ccpAtn);
	
	/* attempt to list all paragraph bounds */

	fseek(mainfd,112,SEEK_SET);	
	d->fib.pnChpFirst = read_32ubit(mainfd);
	d->fib.cpnBteChp = read_32ubit(mainfd);
	fseek(mainfd,124,SEEK_SET);	
	d->fib.pnPapFirst = read_32ubit(mainfd);
	d->fib.cpnBtePap = read_32ubit(mainfd);
	fseek(mainfd,136,SEEK_SET);	
	d->fib.pnLvcFirst = read_32ubit(mainfd);
	d->fib.cpnBteLvc = read_32ubit(mainfd);
	
	trace("\n page of first different char type %d\nnumber of different character types %d\n",
		d->fib.pnChpFirst, d->fib.cpnBteChp);
     trace("\n page of first different para type %d\nnumber of different para types %d\n",
		d->fib.pnPapFirst, d->fib.cpnBtePap);
	trace("\n page of first different lvc type %d\nnumber of different lvc types %d\n",
		d->fib.pnLvcFirst, d->fib.cpnBteLvc);
	
	fseek(mainfd,154,SEEK_SET);

	/* stylesheets */
	d->fib.fcStshfOrig = read_32ubit(mainfd);
	d->fib.lcbStshfOrig = read_32ubit(mainfd);
	d->fib.fcStshf = read_32ubit(mainfd);
	d->fib.lcbStshf = read_32ubit(mainfd);
	
	trace("the orig stsh is (%x), len (%x)\n",
		d->fib.fcStshfOrig, d->fib.lcbStshfOrig);
	trace("the new stsh is (%x), len (%x)\n",
		d->fib.fcStshf, d->fib.lcbStshf);

	/* read stylesheets (stylesheet.c) */
	if (decode_stylesheet(d) != 0) return(10);

	/* footnotes??? */
	d->fib.fcPlcffndRef=read_32ubit(mainfd);
	d->fib.lcbPlcffndRef=read_32ubit(mainfd);
	d->fib.fcPlcffndTxt=read_32ubit(mainfd);
	d->fib.lcbPlcffndTxt=read_32ubit(mainfd);

	trace("footnote: table offset of frd thingies (%x) of len %d\n",
		d->fib.fcPlcffndRef, d->fib.lcbPlcffndRef);
	trace("there are %d footnotes\n", (d->fib.lcbPlcffndRef-4)/6);
	trace("table offset for footnote text (%x) of len %d\n",
		d->fib.fcPlcffndTxt, d->fib.lcbPlcffndTxt);

	/* process footnotes (footanno.c) */
	if (read_footnote_info(d) != 0) return(10);

	/* annotations */
	d->fib.fcPlcfandRef=read_32ubit(mainfd);
	d->fib.lcbPlcfandRef=read_32ubit(mainfd);
	d->fib.fcPlcfandTxt=read_32ubit(mainfd);
	d->fib.lcbPlcfandTxt=read_32ubit(mainfd);

	trace("annotation: table offset of anno thingies(?) (%x) of len %d\n",
		d->fib.fcPlcfandRef, d->fib.lcbPlcfandRef);
	trace("there are %d annotations\n", (d->fib.lcbPlcfandRef-4)/34);
	trace("table offset for annotation text (%x) of len %d\n",
		d->fib.fcPlcfandTxt, d->fib.lcbPlcfandTxt);
	
	/* process annotations (footanno.c) */
	if (read_annotation_info(d) != 0) return(10);

	/* section table */
	d->fib.fcPlcfsed=read_32ubit(mainfd);
	d->fib.lcbPlcfsed=read_32ubit(mainfd);
	trace("section: table offset for section table (%x) of len %d\n",
		d->fib.fcPlcfsed, d->fib.lcbPlcfsed);

	/* process sections (currently in footanno.c, should move?) */
	if (read_section_info(d) != 0) return(10);

	fseek(mainfd,242,SEEK_SET);	
	
	/*these point to the header/footer information thing*/
	d->fib.fcPlcfhdd = read_32ubit(mainfd);
	d->fib.lcbPlcfhdd = read_32ubit(mainfd);
	
	trace("header in table offset of (%x), len is %d\n",
		d->fib.fcPlcfhdd, d->fib.lcbPlcfhdd);
	
	/* chpx */
	d->fib.fcPlcfbteChpx = read_32ubit(mainfd);
	d->fib.lcbPlcfbteChpx = read_32ubit(mainfd);
	
	trace("\nlocation of char description in table stream is %x\nsize is %ld\n",
		d->fib.fcPlcfbteChpx, d->fib.lcbPlcfbteChpx);
	
	/* load chpx (papchp.c) */
	if (read_chpx_info(d) != 0) return(10);

	/* papx */
	d->fib.fcPlcfbtePapx = read_32ubit(mainfd);
	d->fib.lcbPlcfbtePapx = read_32ubit(mainfd);
	error("\nlocation of para description in table stream is %ld\nsize is %ld\n",
		d->fib.fcPlcfbtePapx, d->fib.lcbPlcfbtePapx);

	/* load papx (papchp.c) */
	if (read_papx_info(d) != 0) return(10);

	fseek(mainfd,274,SEEK_SET);

	/* fonts */
	d->fib.fcSttbfffn=read_32ubit(mainfd);
	d->fib.lcbSttbfffn=read_32ubit(mainfd);
     
	/* load font names (fonts.c) */
	if (read_font_names(d) != 0) return(10);

	/* determine field plc */
	/* main fields */
	d->fib.fcPlcffldMom=read_32ubit(mainfd);
	d->fib.lcbPlcffldMom=read_32ubit(mainfd);
	
	trace("in table stream field plc is %ld, and len is %ld\n",
		d->fib.fcPlcffldMom, d->fib.lcbPlcffldMom);

	/* read main fields (fields.c) */
	if (read_main_fields(d) != 0) return(10);

	/* header fields */
	d->fib.fcPlcffldHdr=read_32ubit(mainfd);
	d->fib.lcbPlcffldHdr=read_32ubit(mainfd);
	
	trace("in table stream field header plc is (%x), and len is %ld\n",
		d->fib.fcPlcffldHdr, d->fib.lcbPlcffldHdr);

	/* read header fields (fields.c) */
	if (read_header_fields(d) != 0) return(10);

	/* footnote fields */
	d->fib.fcPlcffldFtn=read_32ubit(mainfd);
	d->fib.lcbPlcffldFtn=read_32ubit(mainfd);
	
	trace("in table stream field footnote plc is (%x), and len is %ld\n",
		d->fib.fcPlcffldFtn, d->fib.lcbPlcffldFtn);
	
	/* read footnote fields (fields.c) */
	if (read_footnoe_fields(d) != 0) return(10);

	/* annotation fields */
	d->fib.fcPlcffldAtn=read_32ubit(mainfd);
	d->fib.lcbPlcffldAtn=read_32ubit(mainfd);
	trace("in table stream field annotation plc is (%x), and len is %ld\n",
		d->fib.fcPlcffldAtn, d->fib.lcbPlcffldAtn);

	/* read annotation fields (fields.c) */
	if (read_annotation_fields(d) != 0) return(10);

	/* move this? */
	decode_bookmarks(mainfd,tablefd,&portions);
     
	fseek(mainfd,418,SEEK_SET);
	
	/* complex info bit */
	d->fib.fcClx =  read_32ubit(mainfd);
	d->fib.lcbClx = read_32ubit(mainfd);
	
	trace("complex bit begins at %X, and it %d long",
		d->fib.fcClx, d->fib.lcbClx);

	fseek(mainfd,442,SEEK_SET);

	/* author information */
	d->fib.fcGrpXstAtnOwners = read_32ubit(mainfd);
	d->fib.lcbGrpXstAtnOwners = read_32ubit(mainfd);
	
	trace("fcGrpXstAtnOwners %x, lcbGrpXstAtnOwners %d\n",
		d->fib.fcGrpXstAtnOwners, d->fib.lcbGrpXstAtnOwners);
	
	/* move these? */
	portions.authors = extract_authors(tablefd, d->fib.fcGrpXstAtnOwners,
		d->fib.lcbGrpXstAtnOwners);
	decode_annotations(mainfd,tablefd,&portions);

	fseek(mainfd,474,SEEK_SET);

	/* pictures */
	d->fib.fcPlcspaMom = read_32ubit(mainfd);
	d->fib.lcbPlcspaMom = read_32ubit(mainfd);
	
	trace("error: pictures offset %x len %d\n",
		d->fib.fcPlcspaMom, d->fib.lcbPlcspaMom);

	/* read picture info (pictures.c) */
	/* need to do picture info reading function */

	fseek(mainfd,522,SEEK_SET);
	
	/* endnotes */
	d->fib.fcPlcfendRef = read_32ubit(mainfd);
	d->fib.lcbPlcfendRef = read_32ubit(mainfd);
	d->fib.fcPlcfendTxt = read_32ubit(mainfd);
	d->fib.lcbPlcfendTxt = read_32ubit(mainfd);

	trace("endnote: table offset of frd thingies (%x) of len %d\n",
		d->fib.fcPlcfendRef, d->fib.lcbPlcfendRef);
	trace("endnote: table offset for endnote text (%x) of len %d\n",
		d->fib.fcPlcfendTxt, d->fib.lcbPlcfendTxt);

	/* read endnotes (endnote.c) */
	if (read_endnote_info(d) != 0) return(10);
	
	/* endnote fields */
	d->fib.fcPlcffldEdn = read_32ubit(mainfd);
	d->fib.lcbPlcffldEdn = read_32ubit(mainfd);

	/* read endnote fields (fields.c) */
	if (read_endnote_fields(d) != 0) return(10);

	fseek(mainfd,554,SEEK_SET);
	
	/* something to do with pictures (maybe?) */
	d->fib.fcDggInfo = read_32ubit(mainfd);
	d->fib.lcbDggInfo = read_32ubit(mainfd);
	
	trace("fcDggInfo is %x and len is %d\n",
		d->fib.fcDggInfo, d->fib.lcbDggInfo);

	/* move this? */
	/* it normally extracts embedded images to files */
	portions.ablipdata = get_blips(fcDggInfo,lcbDggInfo,tablefd,mainfd,&(portions.noofblipdata),0x08,NULL);

	fseek(mainfd,730,SEEK_SET);
	
	/* table info (piece table?) */
	d->fib.fcSttbFnm =  read_32ubit(mainfd);
	d->fib.lcbSttbFnm = read_32ubit(mainfd);
	d->fib.fcPlcfLst =  read_32ubit(mainfd);
	d->fib.lcbPlcfLst = read_32ubit(mainfd);
	d->fib.fcPlfLfo =  read_32ubit(mainfd);
	d->fib.lcbPlfLfo = read_32ubit(mainfd);

	/* move this? */
	get_table_info(tablefd,&a_list_info,fcSttbFnm,lcbSttbFnm,fcPlcfLst,lcbPlcfLst,fcPlfLfo,lcbPlfLfo,masterstylesheet);

	return 0;
}
