pictures
still in old form. pulled from the old fib decoder in mswordview
i don't understand pictures yet...

	portions.noofficedraw=0;
	portions.officedrawcps=NULL;
	portions.spids=NULL;
	portions.noofblipdata=0;

	if (lcbPlcspaMom!=0)
		{
		fseek(tablefd,fcPlcspaMom,SEEK_SET);
		portions.noofficedraw = (lcbPlcspaMom-4)/28;
		portions.officedrawcps = (U32 *) malloc(sizeof(U32) * (portions.noofficedraw+1));
		portions.spids=(U32 *) malloc(sizeof(U32) * portions.noofficedraw);
		if ((portions.officedrawcps == NULL) || ((portions.spids == NULL) && (portions.noofficedraw >0)) )
			{
			error(erroroutput,"no mem for spids\n");
			return(-1);
			}
		for(i=0;i<portions.noofficedraw+1;i++)
			{
			portions.officedrawcps[i] = read_32ubit(tablefd);
			error(erroroutput,"office draw cp is %x\n",portions.officedrawcps[i]);
			}
		for(i=0;i<portions.noofficedraw;i++)
			{
			portions.spids[i] = read_32ubit(tablefd);
			error(erroroutput,"spid id %x\n",portions.spids[i]);
			for (j=0;j<22;j++)
				getc(tablefd);
			}
		}
