/* basically done. needs comments */

S8 read_chpx_info(doc *d) {
	
	d->plcfbteChpx = (U32*)malloc(d->fib.lcbPlcfbteChpx);
	if (d->plcfbteChpx == NULL) {
		error("Out of memory\n");
		return (-1);
	}
	
	fseek(tablefd,d->fib.fcPlcfbteChpx,SEEK_SET);
	for (i = 0; i < d->fib.lcbPlcfbteChpx/4; i++)
		d->plcfbteChpx[i] = read_32ubit(tablefd);
	d->chp_intervals = ((d->fib.lcbPlcfbteChpx/4)-1)/2;
	
	trace(erroroutput,"there are %d charrun intervals ? ending at ", d->chp_intervals);
	for (i = 1; i < d->chp_intervals+1; i++)
		trace(erroroutput,"%d (%d)", d->plcfbteChpx[i], d->plcfbteChpx[i+d->chp_intervals]);
	trace(erroroutput,"\n");

	return(0);
}

S8 read_papx_info(doc *d) {

	/*go to location in table stream, */
	/*i believe that this is just an array of longs(4 bytes blocks)
	 */
	d->plcfbtePapx = (U32*)malloc(d->fib.lcbPlcfbtePapx);
	if (d->plcfbtePapx == NULL) {
		error("Out of memory\n");
		return (-1);
	}
		
	fseek(tablefd,d->fib.fcPlcfbtePapx,SEEK_SET);
	for (i = 0; i < d->fib.lcbPlcfbtePapx/4; i++) {
		d->plcfbtePapx[i] = read_32ubit(tablefd);
		trace("papx farting gives %x\n", d->plcfbtePapx[i]);
		}
	d->pap_intervals = ((d->fib.lcbPlcfbtePapx/4)-1)/2;
	trace("there are %d pragraph intervals ? ending at ",d->pap_intervals);
	for (i = 1; i < d->pap_intervals+1; i++)
		trace("%d %x (%d)", d->plcfbtePapx[i], d->plcfbtePapx[i], d->plcfbtePapx[i+d->pap_intervals]);
	trace("\n");

	return (0);
}
