
#ifndef SPELL_H
#define SPELL_H

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
