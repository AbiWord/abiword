/* basically done for now, but could use comments */

S8 read_font_names(doc *d) {

	U32 i, k, j;
	U8 len, notfinished;
	ffn *tempnames;

	d->fontnames = (ffn*)malloc(sizeof(ffn));
	if (d->fontnames->next == NULL) {
		error("no memory, arrgh\n");
		return(-1);
	}
	
	if (d->fib.lcbSttbfffn > 0) {	
		tempnames = d->fontnames;
		trace("have a table of font names (%x)\n",d->fib.fcSttbfffn);
		
		fseek(tablefd, d->fib.fcSttbfffn,SEEK_SET);

		j=0;
		/*seems to be a count of font names, followed by three blanks ?*/
		read_16ubit(tablefd);
		read_16ubit(tablefd);
		j=4;
		tempnames->next=NULL;
		while(j < d->fib.lcbSttbfffn) {
			k=0;
			tempnames->name[0] = '\0';
			tempnames->next = NULL;
			len = read_8ubit(tablefd);
			trace("len is %d\n", len);
			i=1;
			notfinished=1;
			while( i <= len) {
				if ((i >= 40) && (notfinished)) {
					tempnames->name[k] = read_16ubit(tablefd);
					i+=2;
					trace("font name char is %X %c %x i is %d\n",
						tempnames->name[k], tempnames->name[k], ftell(tablefd), i);
					if (tempnames->name[k] == 0) notfinished=0;
					k++;
				} else {	
					if (i==4) {
						tempnames->chs = read_8ubit(tablefd);
						trace("chs is (%x)  %d ", tempnames->chs, tempnames->chs);
					} else {
						read_8ubit(tablefd);
					}
					i++;
				}
			}
			if (tempnames->name[0] != '\0') {
				tempnames->next = (ffn*)malloc(sizeof(ffn));
				if (tempnames->next == NULL) {
					error("no memory, arrgh\n");
					return(-1);
				}
				tempnames = tempnames->next;
				tempnames->next = NULL;
				tempnames->name[0] = '\0';
			}
			j+=i;	
		}
	}

	tempnames = d->fontnames;
	while (tempnames != NULL) {
		trace("font names are %s\n", tempnames->name);
		tempnames = tempnames->next;
	}

	return (0);
}
