
#ifndef SPELL_H
#define SPELL_H

/*
  TODO stuff we need to do for this spell module:

  eliminate all the stderr fprintfs
  rip out the support for ICHAR_IS_CHAR
*/

#ifdef __cplusplus
extern "C"
{
#endif
	
int SpellCheckInit(char *hashname);
int SpellCheckWord16(unsigned short  *word16);
int SpellCheckWord(char *word);

#ifdef __cplusplus
}
#endif
	
#endif /* SPELL_H */
