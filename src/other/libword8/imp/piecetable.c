/* haven't touched. needs converting */


S8 get_piecetable(FILE *in,U32 **rgfc,U32 **avalrgfc,U16 **sprm,U32 *clxcount) {
	U32 lcb;
	int nopieces=0;
	int i;
	U32 aval;

	lcb = read_32ubit(in);
	(*clxcount)+=(lcb+4);
	nopieces = (lcb-4)/12;
	error(erroroutput,"lcb is %ld, theres %d pieces\n",lcb,nopieces);
	
	*rgfc = (U32 *) malloc((nopieces +1) * sizeof(U32));
	*avalrgfc = (U32 *) malloc((nopieces) * sizeof(U32));
	*sprm = (U16 *)  malloc((nopieces) * sizeof(U16));

	if ( ((*rgfc) == NULL) || ((*avalrgfc) == NULL) || ((*sprm) == NULL) )
		{
		error(erroroutput,"aborting due lack to lack of memory\n");
		exit(-1);
		}

	for(i=0;i<nopieces+1;i++)
		{
		(*rgfc)[i] = read_32ubit(in);
		error(erroroutput," array entry is %x\n",(*rgfc)[i]);
		}

	for(i=0;i<nopieces;i++)
		{
		aval = read_16ubit(in);
		(*avalrgfc)[i] = read_32ubit(in);
		error(erroroutput,"mpiece: current piece is %d file offsets of pieces are (%x)\n",i,(*avalrgfc)[i]);
		error(erroroutput,"mpiece: end of pieces is (%x)\n",(*avalrgfc)[i]+(*rgfc)[i+1]-(*rgfc)[i]);
		(*sprm)[i] = read_16ubit(in);
		error(erroroutput,"TOUGH: the sprm referenced here is %d\n",(*sprm)[i]);
		if ((*sprm)[i] & 0x01)
			error(erroroutput,"sprm varient 2\n");
		else
			{
			error(erroroutput,"sprm varient 1, isprm is %x, val is %d\n",((*sprm)[i]&0x00fe)>>1,((*sprm)[i]&0xff00)>>8);
			}
		}
	error(erroroutput,"NOPIECES is %d\n",nopieces);
	return(nopieces);
}
