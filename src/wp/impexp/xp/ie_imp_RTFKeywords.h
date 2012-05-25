_rtf_keyword rtfKeywords[] = {
	{"\n", false, false, NO_CONTEXT, RTF_KW_LF },
	{"\r", false, false, NO_CONTEXT, RTF_KW_CR },
	{"'", false, false, NO_CONTEXT, RTF_KW_QUOTE },
	{"*", false, false, NO_CONTEXT, RTF_KW_STAR },
	{"-", false, false, NO_CONTEXT, RTF_KW_HYPHEN },
	{":", false, false, NO_CONTEXT, RTF_KW_COLON },
	{"\\", false, false, NO_CONTEXT, RTF_KW_BACKSLASH },
	{"_", false, false, NO_CONTEXT, RTF_KW_UNDERSCORE },
	{"ab", false, false, NO_CONTEXT, RTF_KW_ab },
	{"abibotline", false, false, NO_CONTEXT, RTF_KW_abibotline },
	{"abicellprops", false, false, NO_CONTEXT, RTF_KW_abicellprops },
	{"abiembed", false, false, NO_CONTEXT, RTF_KW_abiembed },
	{"abiembeddata", false, false, NO_CONTEXT, RTF_KW_abiembeddata },
	{"abiendcell", false, false, NO_CONTEXT, RTF_KW_abiendcell },
	{"abiendtable", false, false, NO_CONTEXT, RTF_KW_abiendtable },
	{"abifieldD", false, false, NO_CONTEXT, RTF_KW_abifieldD },
	{"abiframeprops", false, false, NO_CONTEXT, RTF_KW_abiframeprops },
	{"abilatexdata",false,false,NO_CONTEXT,RTF_KW_abilatexdata },
	{"abilist", false, false, NO_CONTEXT, RTF_KW_abilist },
	{"abiltr", false, false, NO_CONTEXT, RTF_KW_abiltr },
	{"abimathml",false,false,NO_CONTEXT,RTF_KW_abimathml },
	{"abimathmldata",false,false,NO_CONTEXT,RTF_KW_abimathmldata },
	{"abinodiroverride", false, false, NO_CONTEXT, RTF_KW_abinodiroverride },
	{"abirtl", false, false, NO_CONTEXT, RTF_KW_abirtl },
	{"abirevision", false, false, NO_CONTEXT, RTF_KW_abirevision },
	{"abitableprops", false, false, NO_CONTEXT, RTF_KW_abitableprops },
	{"abitopline", false, false, NO_CONTEXT, RTF_KW_abitopline },
	{"absh", false, false, NO_CONTEXT, RTF_KW_absh },
	{"abslock", false, false, NO_CONTEXT, RTF_KW_abslock }, /* 7.0 */
	{"absnoovrlp", true, false, NO_CONTEXT, RTF_KW_absnoovrlp }, /* 2000 */
	{"absw", false, false, NO_CONTEXT, RTF_KW_absw },
	{"acaps", false, false, NO_CONTEXT, RTF_KW_acaps },
	{"acccomma", false, false, NO_CONTEXT, RTF_KW_acccomma }, /* 7.0 */
	{"accdot", false, false, NO_CONTEXT, RTF_KW_accdot }, /* 7.0 */
	{"accnone", false, false, NO_CONTEXT, RTF_KW_accnone }, /* 7.0 */
	{"acf", false, false, NO_CONTEXT, RTF_KW_acf },
	{"additive", false, false, NO_CONTEXT, RTF_KW_additive },
	{"adjustright", false, false, NO_CONTEXT, RTF_KW_adjustright }, /* 97 */
	{"adn", false, false, NO_CONTEXT, RTF_KW_adn },
	{"aenddoc", false, false, NO_CONTEXT, RTF_KW_aenddoc },
	{"aendnotes", false, false, NO_CONTEXT, RTF_KW_aendnotes },
	{"aexpnd", false, false, NO_CONTEXT, RTF_KW_aexpnd },
	{"af", false, false, NO_CONTEXT, RTF_KW_af },
	{"affixed", false, false, NO_CONTEXT, RTF_KW_affixed }, /* 7.0 */
	{"afs", false, false, NO_CONTEXT, RTF_KW_afs },
	{"aftnbj", false, false, NO_CONTEXT, RTF_KW_aftnbj },
	{"aftncn", false, false, NO_CONTEXT, RTF_KW_aftncn },
	{"aftnnalc", false, false, NO_CONTEXT, RTF_KW_aftnnalc },
	{"aftnnar", false, false, NO_CONTEXT, RTF_KW_aftnnar },
	{"aftnnauc", false, false, NO_CONTEXT, RTF_KW_aftnnauc },
	{"aftnnchi", false, false, NO_CONTEXT, RTF_KW_aftnnchi },
	{"aftnnchosung", false, false, NO_CONTEXT, RTF_KW_aftnnchosung }, /* 97 */
	{"aftnncnum", false, false, NO_CONTEXT, RTF_KW_aftnncnum }, /* 97 */
	{"aftnndbar", false, false, NO_CONTEXT, RTF_KW_aftnndbar }, /* 97 */
	{"aftnndbnum", false, false, NO_CONTEXT, RTF_KW_aftnndbnum }, /* 97 */
	{"aftnndbnumd", false, false, NO_CONTEXT, RTF_KW_aftnndbnumd }, /* 97 */
	{"aftnndbnumk", false, false, NO_CONTEXT, RTF_KW_aftnndbnumk }, /* 97 */
	{"aftnndbnumt", false, false, NO_CONTEXT, RTF_KW_aftnndbnumt }, /* 97 */
	{"aftnnganada", false, false, NO_CONTEXT, RTF_KW_aftnnganada }, /* 97 */
	{"aftnngbnum", false, false, NO_CONTEXT, RTF_KW_aftnngbnum }, /* 97 */
	{"aftnngbnumd", false, false, NO_CONTEXT, RTF_KW_aftnngbnumd }, /* 97 */
	{"aftnngbnumk", false, false, NO_CONTEXT, RTF_KW_aftnngbnumk }, /* 97 */
	{"aftnngbnuml", false, false, NO_CONTEXT, RTF_KW_aftnngbnuml }, /* 97 */
	{"aftnnrlc", false, false, NO_CONTEXT, RTF_KW_aftnnrlc },
	{"aftnnruc", false, false, NO_CONTEXT, RTF_KW_aftnnruc },
	{"aftnnzodiac", false, false, NO_CONTEXT, RTF_KW_aftnnzodiac }, /* 97 */
	{"aftnnzodiacd", false, false, NO_CONTEXT, RTF_KW_aftnnzodiacd }, /* 97 */
	{"aftnnzodiacl", false, false, NO_CONTEXT, RTF_KW_aftnnzodiacl }, /* 97 */
	{"aftnrestart", false, false, NO_CONTEXT, RTF_KW_aftnrestart },
	{"aftnrstcont", false, false, NO_CONTEXT, RTF_KW_aftnrstcont },
	{"aftnsep", false, false, NO_CONTEXT, RTF_KW_aftnsep },
	{"aftnsepc", false, false, NO_CONTEXT, RTF_KW_aftnsepc },
	{"aftnstart", false, false, NO_CONTEXT, RTF_KW_aftnstart },
	{"aftntj", false, false, NO_CONTEXT, RTF_KW_aftntj },
	{"ai", false, false, NO_CONTEXT, RTF_KW_ai },
	{"alang", false, false, NO_CONTEXT, RTF_KW_alang },
	{"allowfieldendsel", false, false, NO_CONTEXT, RTF_KW_allowfieldendsel }, /* 2002 */
	{"allprot", false, false, NO_CONTEXT, RTF_KW_allprot },
	{"alntblind", false, false, NO_CONTEXT, RTF_KW_alntblind }, /* 2000 */
	{"alt", false, false, NO_CONTEXT, RTF_KW_alt },
	{"animtext", true, false, NO_CONTEXT, RTF_KW_animtext }, /* 97 */
	{"annotation", false, false, NO_CONTEXT, RTF_KW_annotation },
	{"annotprot", false, false, NO_CONTEXT, RTF_KW_annotprot },
	{"ansi", false, false, NO_CONTEXT, RTF_KW_ansi },
	{"ansicpg", true, false, NO_CONTEXT, RTF_KW_ansicpg }, /* 97 */
	{"aoutl", false, false, NO_CONTEXT, RTF_KW_aoutl },
	{"ApplyBrkRules", false, false, NO_CONTEXT, RTF_KW_ApplyBrkRules }, /* 2002 */
	{"ascaps", false, false, NO_CONTEXT, RTF_KW_ascaps },
	{"ashad", false, false, NO_CONTEXT, RTF_KW_ashad },
	{"asianbrkrule", false, false, NO_CONTEXT, RTF_KW_asianbrkrule }, /* 2002 */
	{"aspalpha", false, false, NO_CONTEXT, RTF_KW_aspalpha }, /* 7.0 */
	{"aspnum", false, false, NO_CONTEXT, RTF_KW_aspnum }, /* 7.0 */
	{"astrike", false, false, NO_CONTEXT, RTF_KW_astrike },
	{"atnauthor", false, false, NO_CONTEXT, RTF_KW_atnauthor }, /* 2002 */
	{"atndate", false, false, NO_CONTEXT, RTF_KW_atndate },
	{"atnicn", false, false, NO_CONTEXT, RTF_KW_atnicn },
	{"atnid", false, false, NO_CONTEXT, RTF_KW_atnid },
	{"atnparent", false, false, NO_CONTEXT, RTF_KW_atnparent }, /* 2002 */
	{"atnref", false, false, NO_CONTEXT, RTF_KW_atnref },
	{"atntime", false, false, NO_CONTEXT, RTF_KW_atntime },
	{"atrfend", false, false, NO_CONTEXT, RTF_KW_atrfend },
	{"atrfstart", false, false, NO_CONTEXT, RTF_KW_atrfstart },
	{"aul", false, false, NO_CONTEXT, RTF_KW_aul },
	{"auld", false, false, NO_CONTEXT, RTF_KW_auld },
	{"auldb", false, false, NO_CONTEXT, RTF_KW_auldb },
	{"aulnone", false, false, NO_CONTEXT, RTF_KW_aulnone },
	{"aulw", false, false, NO_CONTEXT, RTF_KW_aulw },
	{"aup", false, false, NO_CONTEXT, RTF_KW_aup },
	{"author", false, false, NO_CONTEXT, RTF_KW_author },
	{"b", false, false, NO_CONTEXT, RTF_KW_b },
	{"background", false, false, NO_CONTEXT, RTF_KW_background }, /* 97 */
	{"bdbfhdr", false, false, NO_CONTEXT, RTF_KW_bdbfhdr }, /* 97 */
	{"bdrrlswsix", false, false, NO_CONTEXT, RTF_KW_bdrrlswsix }, /* 2000 */
	{"bgbdiag", false, false, NO_CONTEXT, RTF_KW_bgbdiag },
	{"bgcross", false, false, NO_CONTEXT, RTF_KW_bgcross },
	{"bgdcross", false, false, NO_CONTEXT, RTF_KW_bgdcross },
	{"bgdkbdiag", false, false, NO_CONTEXT, RTF_KW_bgdkbdiag },
	{"bgdkcross", false, false, NO_CONTEXT, RTF_KW_bgdkcross },
	{"bgdkdcross", false, false, NO_CONTEXT, RTF_KW_bgdkdcross },
	{"bgdkfdiag", false, false, NO_CONTEXT, RTF_KW_bgdkfdiag },
	{"bgdkhoriz", false, false, NO_CONTEXT, RTF_KW_bgdkhoriz },
	{"bgdkvert", false, false, NO_CONTEXT, RTF_KW_bgdkvert },
	{"bgfdiag", false, false, NO_CONTEXT, RTF_KW_bgfdiag },
	{"bghoriz", false, false, NO_CONTEXT, RTF_KW_bghoriz },
	{"bgvert", false, false, NO_CONTEXT, RTF_KW_bgvert },
	{"bin", false, false, NO_CONTEXT, RTF_KW_bin },
	{"binfsxn", false, false, NO_CONTEXT, RTF_KW_binfsxn },
	{"binsxn", false, false, NO_CONTEXT, RTF_KW_binsxn },
	{"bkmkcolf", false, false, NO_CONTEXT, RTF_KW_bkmkcolf },
	{"bkmkcoll", false, false, NO_CONTEXT, RTF_KW_bkmkcoll },
	{"bkmkend", false, false, NO_CONTEXT, RTF_KW_bkmkend },
	{"bkmkpub", false, false, NO_CONTEXT, RTF_KW_bkmkpub },
	{"bkmkstart", false, false, NO_CONTEXT, RTF_KW_bkmkstart },
	{"bliptag", true, false, NO_CONTEXT, RTF_KW_bliptag }, /* 97 */
	{"blipuid", false, false, NO_CONTEXT, RTF_KW_blipuid }, /* 97 */
	{"blipupi", true, false, NO_CONTEXT, RTF_KW_blipupi }, /* 97 */
	{"blue", false, false, NO_CONTEXT, RTF_KW_blue },
	{"bookfold", false, false, NO_CONTEXT, RTF_KW_bookfold }, /* 2002 */
	{"bookfoldrev", false, false, NO_CONTEXT, RTF_KW_bookfoldrev }, /* 2002 */
	{"bookfoldsheets", true, false, NO_CONTEXT, RTF_KW_bookfoldsheets }, /* 2002 */
	{"botline", false, false, NO_CONTEXT, RTF_KW_botline },
	{"box", false, false, NO_CONTEXT, RTF_KW_box },
	{"brdrart", true, false, NO_CONTEXT, RTF_KW_brdrart }, /* 97 */
	{"brdrb", false, false, NO_CONTEXT, RTF_KW_brdrb },
	{"brdrbar", false, false, NO_CONTEXT, RTF_KW_brdrbar },
	{"brdrbtw", false, false, NO_CONTEXT, RTF_KW_brdrbtw },
	{"brdrcf", false, false, NO_CONTEXT, RTF_KW_brdrcf },
	{"brdrdash", false, false, NO_CONTEXT, RTF_KW_brdrdash },
	{"brdrdashd", false, false, NO_CONTEXT, RTF_KW_brdrdashd }, /* 97 */
	{"brdrdashdd", false, false, NO_CONTEXT, RTF_KW_brdrdashdd }, /* 97 */
	{"brdrdashdotstr", false, false, NO_CONTEXT, RTF_KW_brdrdashdotstr }, /* 97 */
	{"brdrdashsm", false, false, NO_CONTEXT, RTF_KW_brdrdashsm }, /* 97 */
	{"brdrdb", false, false, NO_CONTEXT, RTF_KW_brdrdb },
	{"brdrdot", false, false, NO_CONTEXT, RTF_KW_brdrdot },
	{"brdremboss", false, false, NO_CONTEXT, RTF_KW_brdremboss }, /* 97 */
	{"brdrengrave", false, false, NO_CONTEXT, RTF_KW_brdrengrave }, /* 97 */
	{"brdrframe", false, false, NO_CONTEXT, RTF_KW_brdrframe }, /* 97 */
	{"brdrhair", false, false, NO_CONTEXT, RTF_KW_brdrhair },
	{"brdrinset", false, false, NO_CONTEXT, RTF_KW_brdrinset }, /* 2000 */
	{"brdrl", false, false, NO_CONTEXT, RTF_KW_brdrl },
	{"brdrnil", false, false, NO_CONTEXT, RTF_KW_brdrnil }, /* 2002 */
	{"brdrnone", false, false, NO_CONTEXT, RTF_KW_brdrnone }, /* 2002 */
	{"brdroutset", false, false, NO_CONTEXT, RTF_KW_brdroutset }, /* 2000 */
	{"brdrr", false, false, NO_CONTEXT, RTF_KW_brdrr },
	{"brdrs", false, false, NO_CONTEXT, RTF_KW_brdrs },
	{"brdrsh", false, false, NO_CONTEXT, RTF_KW_brdrsh },
	{"brdrt", false, false, NO_CONTEXT, RTF_KW_brdrt },
	{"brdrtbl", false, false, NO_CONTEXT, RTF_KW_brdrtbl }, /* 2002 */
	{"brdrth", false, false, NO_CONTEXT, RTF_KW_brdrth },
	{"brdrthtnlg", false, false, NO_CONTEXT, RTF_KW_brdrthtnlg }, /* 97 */
	{"brdrthtnmg", false, false, NO_CONTEXT, RTF_KW_brdrthtnmg }, /* 97 */
	{"brdrthtnsg", false, false, NO_CONTEXT, RTF_KW_brdrthtnsg }, /* 97 */
	{"brdrtnthlg", false, false, NO_CONTEXT, RTF_KW_brdrtnthlg }, /* 97 */
	{"brdrtnthmg", false, false, NO_CONTEXT, RTF_KW_brdrtnthmg }, /* 97 */
	{"brdrtnthsg", false, false, NO_CONTEXT, RTF_KW_brdrtnthsg }, /* 97 */
	{"brdrtnthtnlg", false, false, NO_CONTEXT, RTF_KW_brdrtnthtnlg }, /* 97 */
	{"brdrtnthtnmg", false, false, NO_CONTEXT, RTF_KW_brdrtnthtnmg }, /* 97 */
	{"brdrtnthtnsg", false, false, NO_CONTEXT, RTF_KW_brdrtnthtnsg }, /* 97 */
	{"brdrtriple", false, false, NO_CONTEXT, RTF_KW_brdrtriple }, /* 97 */
	{"brdrw", false, false, NO_CONTEXT, RTF_KW_brdrw },
	{"brdrwavy", false, false, NO_CONTEXT, RTF_KW_brdrwavy }, /* 97 */
	{"brdrwavydb", false, false, NO_CONTEXT, RTF_KW_brdrwavydb }, /* 97 */
	{"brkfrm", false, false, NO_CONTEXT, RTF_KW_brkfrm },
	{"brsp", false, false, NO_CONTEXT, RTF_KW_brsp },
	{"bullet", false, false, NO_CONTEXT, RTF_KW_bullet },
	{"buptim", false, false, NO_CONTEXT, RTF_KW_buptim },
	{"bxe", false, false, NO_CONTEXT, RTF_KW_bxe },
	{"caps", false, false, NO_CONTEXT, RTF_KW_caps },
	{"category", false, false, NO_CONTEXT, RTF_KW_category }, /* 7.0 */
	{"cb", false, false, NO_CONTEXT, RTF_KW_cb },
	{"cbpat", false, false, NO_CONTEXT, RTF_KW_cbpat },
	{"cchs", false, false, NO_CONTEXT, RTF_KW_cchs },
	{"cell", false, false, NO_CONTEXT, RTF_KW_cell },
	{"cellx", false, false, NO_CONTEXT, RTF_KW_cellx },
	{"cf", false, false, NO_CONTEXT, RTF_KW_cf },
	{"cfpat", false, false, NO_CONTEXT, RTF_KW_cfpat },
	{"cgrid", true, false, NO_CONTEXT, RTF_KW_cgrid }, /* 97 */
	{"charrsid", true, false, NO_CONTEXT, RTF_KW_charrsid }, /* 2002 */
	{"charscalex", false, false, NO_CONTEXT, RTF_KW_charscalex }, /* 7.0 */
	{"chatn", false, false, NO_CONTEXT, RTF_KW_chatn },
	{"chbgbdiag", false, false, NO_CONTEXT, RTF_KW_chbgbdiag }, /* 97 */
	{"chbgcross", false, false, NO_CONTEXT, RTF_KW_chbgcross }, /* 97 */
	{"chbgdcross", false, false, NO_CONTEXT, RTF_KW_chbgdcross }, /* 97 */
	{"chbgdkbdiag", false, false, NO_CONTEXT, RTF_KW_chbgdkbdiag }, /* 97 */
	{"chbgdkcross", false, false, NO_CONTEXT, RTF_KW_chbgdkcross }, /* 97 */
	{"chbgdkdcross", false, false, NO_CONTEXT, RTF_KW_chbgdkdcross }, /* 97 */
	{"chbgdkfdiag", false, false, NO_CONTEXT, RTF_KW_chbgdkfdiag }, /* 97 */
	{"chbgdkhoriz", false, false, NO_CONTEXT, RTF_KW_chbgdkhoriz }, /* 97 */
	{"chbgdkvert", false, false, NO_CONTEXT, RTF_KW_chbgdkvert }, /* 97 */
	{"chbgfdiag", false, false, NO_CONTEXT, RTF_KW_chbgfdiag }, /* 97 */
	{"chbghoriz", false, false, NO_CONTEXT, RTF_KW_chbghoriz }, /* 97 */
	{"chbgvert", false, false, NO_CONTEXT, RTF_KW_chbgvert }, /* 97 */
	{"chbrdr", false, false, NO_CONTEXT, RTF_KW_chbrdr }, /* 97 */
	{"chcbpat", true, false, NO_CONTEXT, RTF_KW_chcbpat }, /* 97 */
	{"chcfpat", true, false, NO_CONTEXT, RTF_KW_chcfpat }, /* 97 */
	{"chdate", false, false, NO_CONTEXT, RTF_KW_chdate },
	{"chdpa", false, false, NO_CONTEXT, RTF_KW_chdpa },
	{"chdpl", false, false, NO_CONTEXT, RTF_KW_chdpl },
	{"chftn", false, false, NO_CONTEXT, RTF_KW_chftn },
	{"chftnsep", false, false, NO_CONTEXT, RTF_KW_chftnsep },
	{"chftnsepc", false, false, NO_CONTEXT, RTF_KW_chftnsepc },
	{"chpgn", false, false, NO_CONTEXT, RTF_KW_chpgn },
	{"chshdng", true, false, NO_CONTEXT, RTF_KW_chshdng }, /* 97 */
	{"chtime", false, false, NO_CONTEXT, RTF_KW_chtime },
	{"clbgbdiag", false, false, NO_CONTEXT, RTF_KW_clbgbdiag },
	{"clbgcross", false, false, NO_CONTEXT, RTF_KW_clbgcross },
	{"clbgdcross", false, false, NO_CONTEXT, RTF_KW_clbgdcross },
	{"clbgdkbdiag", false, false, NO_CONTEXT, RTF_KW_clbgdkbdiag },
	{"clbgdkcross", false, false, NO_CONTEXT, RTF_KW_clbgdkcross },
	{"clbgdkdcross", false, false, NO_CONTEXT, RTF_KW_clbgdkdcross },
	{"clbgdkfdiag", false, false, NO_CONTEXT, RTF_KW_clbgdkfdiag },
	{"clbgdkhor", false, false, NO_CONTEXT, RTF_KW_clbgdkhor },
	{"clbgdkvert", false, false, NO_CONTEXT, RTF_KW_clbgdkvert },
	{"clbgfdiag", false, false, NO_CONTEXT, RTF_KW_clbgfdiag },
	{"clbghoriz", false, false, NO_CONTEXT, RTF_KW_clbghoriz },
	{"clbgvert", false, false, NO_CONTEXT, RTF_KW_clbgvert },
	{"clbrdrb", false, false, NO_CONTEXT, RTF_KW_clbrdrb },
	{"clbrdrl", false, false, NO_CONTEXT, RTF_KW_clbrdrl },
	{"clbrdrr", false, false, NO_CONTEXT, RTF_KW_clbrdrr },
	{"clbrdrt", false, false, NO_CONTEXT, RTF_KW_clbrdrt },
	{"clcbpat", false, false, NO_CONTEXT, RTF_KW_clcbpat },
	{"clcbpatraw", true, false, NO_CONTEXT, RTF_KW_clcbpatraw }, /* 2002 */
	{"clcfpat", false, false, NO_CONTEXT, RTF_KW_clcfpat },
	{"clcfpatraw", true, false, NO_CONTEXT, RTF_KW_clcfpatraw }, /* 2002 */
	{"cldgll", false, false, NO_CONTEXT, RTF_KW_cldgll }, /* 7.0 */
	{"cldglu", false, false, NO_CONTEXT, RTF_KW_cldglu }, /* 7.0 */
	{"clFitText", false, false, NO_CONTEXT, RTF_KW_clFitText }, /* 2000 */
	{"clftsWidth", true, false, NO_CONTEXT, RTF_KW_clftsWidth }, /* 2000 */
	{"clmgf", false, false, NO_CONTEXT, RTF_KW_clmgf },
	{"clmrg", false, false, NO_CONTEXT, RTF_KW_clmrg },
	{"clNoWrap", false, false, NO_CONTEXT, RTF_KW_clNoWrap }, /* 2000 */
	{"clpadb", true, false, NO_CONTEXT, RTF_KW_clpadb }, /* 2000 */
	{"clpadfb", true, false, NO_CONTEXT, RTF_KW_clpadfb }, /* 2000 */
	{"clpadfl", true, false, NO_CONTEXT, RTF_KW_clpadfl }, /* 2000 */
	{"clpadfr", true, false, NO_CONTEXT, RTF_KW_clpadfr }, /* 2000 */
	{"clpadft", true, false, NO_CONTEXT, RTF_KW_clpadft }, /* 2000 */
	{"clpadl", true, false, NO_CONTEXT, RTF_KW_clpadl }, /* 2000 */
	{"clpadr", true, false, NO_CONTEXT, RTF_KW_clpadr }, /* 2000 */
	{"clpadt", true, false, NO_CONTEXT, RTF_KW_clpadt }, /* 2000 */
	{"clshdng", false, false, NO_CONTEXT, RTF_KW_clshdng },
	{"clshdngraw", false, false, NO_CONTEXT, RTF_KW_clshdngraw }, /* 2002 */
	{"clshdrawnil", false, false, NO_CONTEXT, RTF_KW_clshdrawnil }, /* 2002 */
	{"cltxbtlr", false, false, NO_CONTEXT, RTF_KW_cltxbtlr }, /* 7.0 */
	{"cltxlrtb", false, false, NO_CONTEXT, RTF_KW_cltxlrtb }, /* 7.0 */
	{"cltxlrtbv", false, false, NO_CONTEXT, RTF_KW_cltxlrtbv }, /* 7.0 */
	{"cltxtbrl", false, false, NO_CONTEXT, RTF_KW_cltxtbrl }, /* 7.0 */
	{"cltxtbrlv", false, false, NO_CONTEXT, RTF_KW_cltxtbrlv }, /* 7.0 */
	{"clvertalb", false, false, NO_CONTEXT, RTF_KW_clvertalb }, /* 7.0 */
	{"clvertalc", false, false, NO_CONTEXT, RTF_KW_clvertalc }, /* 7.0 */
	{"clvertalt", false, false, NO_CONTEXT, RTF_KW_clvertalt }, /* 7.0 */
	{"clvmgf", false, false, NO_CONTEXT, RTF_KW_clvmgf }, /* 7.0 */
	{"clvmrg", false, false, NO_CONTEXT, RTF_KW_clvmrg }, /* 7.0 */
	{"clwWidth", true, false, NO_CONTEXT, RTF_KW_clwWidth }, /* 2000 */
	{"collapsed", false, false, NO_CONTEXT, RTF_KW_collapsed },
	{"colno", false, false, NO_CONTEXT, RTF_KW_colno },
	{"colortbl", false, false, NO_CONTEXT, RTF_KW_colortbl },
	{"cols", false, false, NO_CONTEXT, RTF_KW_cols },
	{"colsr", false, false, NO_CONTEXT, RTF_KW_colsr },
	{"colsx", false, false, NO_CONTEXT, RTF_KW_colsx },
	{"column", false, false, NO_CONTEXT, RTF_KW_column },
	{"colw", false, false, NO_CONTEXT, RTF_KW_colw },
	{"comment", false, false, NO_CONTEXT, RTF_KW_comment },
	{"company", false, false, NO_CONTEXT, RTF_KW_company }, /* 7.0 */
	{"cpg", false, false, NO_CONTEXT, RTF_KW_cpg },
	{"crauth", true, false, NO_CONTEXT, RTF_KW_crauth }, /* 97 */
	{"crdate", true, false, NO_CONTEXT, RTF_KW_crdate }, /* 97 */
	{"creatim", false, false, NO_CONTEXT, RTF_KW_creatim },
	{"cs", false, false, NO_CONTEXT, RTF_KW_cs },
	{"ctrl", false, false, NO_CONTEXT, RTF_KW_ctrl },
	{"cts", true, false, NO_CONTEXT, RTF_KW_cts }, /* 2000 */
	{"cufi", true, false, NO_CONTEXT, RTF_KW_cufi }, /* 2000 */
	{"culi", true, false, NO_CONTEXT, RTF_KW_culi }, /* 2000 */
	{"curi", true, false, NO_CONTEXT, RTF_KW_curi }, /* 2000 */
	{"cvmme", false, false, NO_CONTEXT, RTF_KW_cvmme },
	{"datafield", false, false, NO_CONTEXT, RTF_KW_datafield },
	{"date", false, false, NO_CONTEXT, RTF_KW_date }, /* 97 */
	{"dbch", false, false, NO_CONTEXT, RTF_KW_dbch }, /* 7.0 */
	{"deff", false, false, NO_CONTEXT, RTF_KW_deff },
	{"defformat", false, false, NO_CONTEXT, RTF_KW_defformat },
	{"deflang", false, false, NO_CONTEXT, RTF_KW_deflang },
	{"deflangfe", false, false, NO_CONTEXT, RTF_KW_deflangfe }, /* 97 */
	{"defshp", false, false, NO_CONTEXT, RTF_KW_defshp }, /* 2000 */
	{"deftab", false, false, NO_CONTEXT, RTF_KW_deftab },
	{"deleted", false, false, NO_CONTEXT, RTF_KW_deleted },
	{"delrsid", true, false, NO_CONTEXT, RTF_KW_delrsid }, /* 2002 */
	{"deltamoveid", false, false, NO_CONTEXT, RTF_KW_deltamoveid },
	{"dfrauth", true, false, NO_CONTEXT, RTF_KW_dfrauth }, /* 97 */
	{"dfrdate", true, false, NO_CONTEXT, RTF_KW_dfrdate }, /* 97 */
	{"dfrmtxtx", false, false, NO_CONTEXT, RTF_KW_dfrmtxtx },
	{"dfrmtxty", false, false, NO_CONTEXT, RTF_KW_dfrmtxty },
	{"dfrstart", false, false, NO_CONTEXT, RTF_KW_dfrstart }, /* 97 */
	{"dfrstop", false, false, NO_CONTEXT, RTF_KW_dfrstop }, /* 97 */
	{"dfrxst", false, false, NO_CONTEXT, RTF_KW_dfrxst }, /* 97 */
	{"dghorigin", true, false, NO_CONTEXT, RTF_KW_dghorigin }, /* 7.0 */
	{"dghshow", true, false, NO_CONTEXT, RTF_KW_dghshow }, /* 7.0 */
	{"dghspace", true, false, NO_CONTEXT, RTF_KW_dghspace }, /* 7.0 */
	{"dgmargin", false, false, NO_CONTEXT, RTF_KW_dgmargin }, /* 97 */
	{"dgsnap", false, false, NO_CONTEXT, RTF_KW_dgsnap }, /* 7.0 */
	{"dgvorigin", true, false, NO_CONTEXT, RTF_KW_dgvorigin }, /* 7.0 */
	{"dgvshow", true, false, NO_CONTEXT, RTF_KW_dgvshow }, /* 7.0 */
	{"dgvspace", true, false, NO_CONTEXT, RTF_KW_dgvspace }, /* 7.0 */
	{"dibitmap", false, false, NO_CONTEXT, RTF_KW_dibitmap },
	{"dn", false, false, NO_CONTEXT, RTF_KW_dn },
	{"dntblnsbdb", false, false, NO_CONTEXT, RTF_KW_dntblnsbdb }, /* 97 */
	{"do", false, false, NO_CONTEXT, RTF_KW_do },
	{"dobxcolumn", false, false, NO_CONTEXT, RTF_KW_dobxcolumn },
	{"dobxmargin", false, false, NO_CONTEXT, RTF_KW_dobxmargin },
	{"dobxpage", false, false, NO_CONTEXT, RTF_KW_dobxpage },
	{"dobymargin", false, false, NO_CONTEXT, RTF_KW_dobymargin },
	{"dobypage", false, false, NO_CONTEXT, RTF_KW_dobypage },
	{"dobypara", false, false, NO_CONTEXT, RTF_KW_dobypara },
	{"doccomm", false, false, NO_CONTEXT, RTF_KW_doccomm },
	{"doctemp", false, false, NO_CONTEXT, RTF_KW_doctemp },
	{"doctype", true, false, NO_CONTEXT, RTF_KW_doctype }, /* 97 */
	{"docvar", false, false, NO_CONTEXT, RTF_KW_docvar }, /* 7.0 */
	{"dodhgt", false, false, NO_CONTEXT, RTF_KW_dodhgt },
	{"dolock", false, false, NO_CONTEXT, RTF_KW_dolock },
	{"donotshowcomments", false, false, NO_CONTEXT, RTF_KW_donotshowcomments }, /* 2002 */
	{"donotshowinsdel", false, false, NO_CONTEXT, RTF_KW_donotshowinsdel }, /* 2002 */
	{"donotshowmarkup", false, false, NO_CONTEXT, RTF_KW_donotshowmarkup }, /* 2002 */
	{"donotshowprops", false, false, NO_CONTEXT, RTF_KW_donotshowprops }, /* 2002 */
	{"dpaendhol", false, false, NO_CONTEXT, RTF_KW_dpaendhol },
	{"dpaendl", false, false, NO_CONTEXT, RTF_KW_dpaendl },
	{"dpaendsol", false, false, NO_CONTEXT, RTF_KW_dpaendsol },
	{"dpaendw", false, false, NO_CONTEXT, RTF_KW_dpaendw },
	{"dparc", false, false, NO_CONTEXT, RTF_KW_dparc },
	{"dparcflipx", false, false, NO_CONTEXT, RTF_KW_dparcflipx },
	{"dparcflipy", false, false, NO_CONTEXT, RTF_KW_dparcflipy },
	{"dpastarthol", false, false, NO_CONTEXT, RTF_KW_dpastarthol },
	{"dpastartl", false, false, NO_CONTEXT, RTF_KW_dpastartl },
	{"dpastartsol", false, false, NO_CONTEXT, RTF_KW_dpastartsol },
	{"dpastartw", false, false, NO_CONTEXT, RTF_KW_dpastartw },
	{"dpcallout", false, false, NO_CONTEXT, RTF_KW_dpcallout },
	{"dpcoa", false, false, NO_CONTEXT, RTF_KW_dpcoa },
	{"dpcoaccent", false, false, NO_CONTEXT, RTF_KW_dpcoaccent },
	{"dpcobestfit", false, false, NO_CONTEXT, RTF_KW_dpcobestfit },
	{"dpcoborder", false, false, NO_CONTEXT, RTF_KW_dpcoborder },
	{"dpcodabs", false, false, NO_CONTEXT, RTF_KW_dpcodabs },
	{"dpcodbottom", false, false, NO_CONTEXT, RTF_KW_dpcodbottom },
	{"dpcodcenter", false, false, NO_CONTEXT, RTF_KW_dpcodcenter },
	{"dpcodescent", false, false, NO_CONTEXT, RTF_KW_dpcodescent },
	{"dpcodtop", false, false, NO_CONTEXT, RTF_KW_dpcodtop },
	{"dpcolength", false, false, NO_CONTEXT, RTF_KW_dpcolength },
	{"dpcominusx", false, false, NO_CONTEXT, RTF_KW_dpcominusx },
	{"dpcominusy", false, false, NO_CONTEXT, RTF_KW_dpcominusy },
	{"dpcooffset", false, false, NO_CONTEXT, RTF_KW_dpcooffset },
	{"dpcosmarta", false, false, NO_CONTEXT, RTF_KW_dpcosmarta },
	{"dpcotdouble", false, false, NO_CONTEXT, RTF_KW_dpcotdouble },
	{"dpcotright", false, false, NO_CONTEXT, RTF_KW_dpcotright },
	{"dpcotsingle", false, false, NO_CONTEXT, RTF_KW_dpcotsingle },
	{"dpcottriple", false, false, NO_CONTEXT, RTF_KW_dpcottriple },
	{"dpcount", false, false, NO_CONTEXT, RTF_KW_dpcount },
	{"dpellipse", false, false, NO_CONTEXT, RTF_KW_dpellipse },
	{"dpendgroup", false, false, NO_CONTEXT, RTF_KW_dpendgroup },
	{"dpfillbgcb", false, false, NO_CONTEXT, RTF_KW_dpfillbgcb },
	{"dpfillbgcg", false, false, NO_CONTEXT, RTF_KW_dpfillbgcg },
	{"dpfillbgcr", false, false, NO_CONTEXT, RTF_KW_dpfillbgcr },
	{"dpfillbggray", false, false, NO_CONTEXT, RTF_KW_dpfillbggray },
	{"dpfillbgpal", false, false, NO_CONTEXT, RTF_KW_dpfillbgpal },
	{"dpfillfgcb", false, false, NO_CONTEXT, RTF_KW_dpfillfgcb },
	{"dpfillfgcg", false, false, NO_CONTEXT, RTF_KW_dpfillfgcg },
	{"dpfillfgcr", false, false, NO_CONTEXT, RTF_KW_dpfillfgcr },
	{"dpfillfggray", false, false, NO_CONTEXT, RTF_KW_dpfillfggray },
	{"dpfillfgpal", false, false, NO_CONTEXT, RTF_KW_dpfillfgpal },
	{"dpfillpat", false, false, NO_CONTEXT, RTF_KW_dpfillpat },
	{"dpgroup", false, false, NO_CONTEXT, RTF_KW_dpgroup },
	{"dpline", false, false, NO_CONTEXT, RTF_KW_dpline },
	{"dplinecob", false, false, NO_CONTEXT, RTF_KW_dplinecob },
	{"dplinecog", false, false, NO_CONTEXT, RTF_KW_dplinecog },
	{"dplinecor", false, false, NO_CONTEXT, RTF_KW_dplinecor },
	{"dplinedado", false, false, NO_CONTEXT, RTF_KW_dplinedado },
	{"dplinedadodo", false, false, NO_CONTEXT, RTF_KW_dplinedadodo },
	{"dplinedash", false, false, NO_CONTEXT, RTF_KW_dplinedash },
	{"dplinedot", false, false, NO_CONTEXT, RTF_KW_dplinedot },
	{"dplinegray", false, false, NO_CONTEXT, RTF_KW_dplinegray },
	{"dplinehollow", false, false, NO_CONTEXT, RTF_KW_dplinehollow },
	{"dplinepal", false, false, NO_CONTEXT, RTF_KW_dplinepal },
	{"dplinesolid", false, false, NO_CONTEXT, RTF_KW_dplinesolid },
	{"dplinew", false, false, NO_CONTEXT, RTF_KW_dplinew },
	{"dppolycount", false, false, NO_CONTEXT, RTF_KW_dppolycount },
	{"dppolygon", false, false, NO_CONTEXT, RTF_KW_dppolygon },
	{"dppolyline", false, false, NO_CONTEXT, RTF_KW_dppolyline },
	{"dpptx", false, false, NO_CONTEXT, RTF_KW_dpptx },
	{"dppty", false, false, NO_CONTEXT, RTF_KW_dppty },
	{"dprect", false, false, NO_CONTEXT, RTF_KW_dprect },
	{"dproundr", false, false, NO_CONTEXT, RTF_KW_dproundr },
	{"dpshadow", false, false, NO_CONTEXT, RTF_KW_dpshadow },
	{"dpshadx", false, false, NO_CONTEXT, RTF_KW_dpshadx },
	{"dpshady", false, false, NO_CONTEXT, RTF_KW_dpshady },
	{"dptxbtlr", false, false, NO_CONTEXT, RTF_KW_dptxbtlr }, /* 7.0 */
	{"dptxbx", false, false, NO_CONTEXT, RTF_KW_dptxbx },
	{"dptxbxmar", false, false, NO_CONTEXT, RTF_KW_dptxbxmar },
	{"dptxbxtext", false, false, NO_CONTEXT, RTF_KW_dptxbxtext },
	{"dptxlrtb", false, false, NO_CONTEXT, RTF_KW_dptxlrtb }, /* 7.0 */
	{"dptxlrtbv", false, false, NO_CONTEXT, RTF_KW_dptxlrtbv }, /* 7.0 */
	{"dptxtbrl", false, false, NO_CONTEXT, RTF_KW_dptxtbrl }, /* 7.0 */
	{"dptxtbrlv", false, false, NO_CONTEXT, RTF_KW_dptxtbrlv }, /* 7.0 */
	{"dpx", false, false, NO_CONTEXT, RTF_KW_dpx },
	{"dpxsize", false, false, NO_CONTEXT, RTF_KW_dpxsize },
	{"dpy", false, false, NO_CONTEXT, RTF_KW_dpy },
	{"dpysize", false, false, NO_CONTEXT, RTF_KW_dpysize },
	{"dropcapli", false, false, NO_CONTEXT, RTF_KW_dropcapli },
	{"dropcapt", false, false, NO_CONTEXT, RTF_KW_dropcapt },
	{"ds", false, false, NO_CONTEXT, RTF_KW_ds },
	{"dxfrtext", false, false, NO_CONTEXT, RTF_KW_dxfrtext },
	{"dy", false, false, NO_CONTEXT, RTF_KW_dy },
	{"edmins", false, false, NO_CONTEXT, RTF_KW_edmins },
	{"embo", false, false, NO_CONTEXT, RTF_KW_embo }, /* 97 */
	{"emdash", false, false, NO_CONTEXT, RTF_KW_emdash },
	{"emfblip", false, false, NO_CONTEXT, RTF_KW_emfblip }, /* 97 */
	{"emspace", false, false, NO_CONTEXT, RTF_KW_emspace },
	{"endash", false, false, NO_CONTEXT, RTF_KW_endash },
	{"enddoc", false, false, NO_CONTEXT, RTF_KW_enddoc },
	{"endnhere", false, false, NO_CONTEXT, RTF_KW_endnhere },
	{"endnotes", false, false, NO_CONTEXT, RTF_KW_endnotes },
	{"enspace", false, false, NO_CONTEXT, RTF_KW_enspace },
	{"expnd", false, false, NO_CONTEXT, RTF_KW_expnd },
	{"expndtw", false, false, NO_CONTEXT, RTF_KW_expndtw },
	{"expshrtn", false, false, NO_CONTEXT, RTF_KW_expshrtn }, /* 97 */
	{"f", false, false, NO_CONTEXT, RTF_KW_f },
	{"faauto", false, false, NO_CONTEXT, RTF_KW_faauto }, /* 97 */
	{"facenter", false, false, NO_CONTEXT, RTF_KW_facenter }, /* 7.0 */
	{"facingp", false, false, NO_CONTEXT, RTF_KW_facingp },
	{"fahang", false, false, NO_CONTEXT, RTF_KW_fahang }, /* 7.0 */
	{"falt", false, false, NO_CONTEXT, RTF_KW_falt },
	{"faroman", false, false, NO_CONTEXT, RTF_KW_faroman }, /* 7.0 */
	{"favar", false, false, NO_CONTEXT, RTF_KW_favar }, /* 7.0 */
	{"fbias", true, false, NO_CONTEXT, RTF_KW_fbias }, /* 97 */
	{"fbidi", false, false, NO_CONTEXT, RTF_KW_fbidi },
	{"fchars", false, false, NO_CONTEXT, RTF_KW_fchars }, /* 7.0 */
	{"fcharset", false, false, NO_CONTEXT, RTF_KW_fcharset },
	{"fdecor", false, false, NO_CONTEXT, RTF_KW_fdecor },
	{"fet", false, false, NO_CONTEXT, RTF_KW_fet },
	{"fetch", false, false, NO_CONTEXT, RTF_KW_fetch },
	{"ffdefres", false, false, NO_CONTEXT, RTF_KW_ffdefres }, /* 97 */
	{"ffdeftext", false, false, NO_CONTEXT, RTF_KW_ffdeftext }, /* 97 */
	{"ffentrymcr", false, false, NO_CONTEXT, RTF_KW_ffentrymcr }, /* 97 */
	{"ffexitmcr", false, false, NO_CONTEXT, RTF_KW_ffexitmcr }, /* 97 */
	{"ffformat", false, false, NO_CONTEXT, RTF_KW_ffformat }, /* 97 */
	{"ffhaslistbox", true, false, NO_CONTEXT, RTF_KW_ffhaslistbox }, /* 97 */
	{"ffhelptext", false, false, NO_CONTEXT, RTF_KW_ffhelptext }, /* 97 */
	{"ffhps", true, false, NO_CONTEXT, RTF_KW_ffhps }, /* 97 */
	{"ffl", false, false, NO_CONTEXT, RTF_KW_ffl }, /* 97 */
	{"ffmaxlen", false, false, NO_CONTEXT, RTF_KW_ffmaxlen }, /* 97 */
	{"ffname", false, false, NO_CONTEXT, RTF_KW_ffname }, /* 97 */
	{"ffownhelp", true, false, NO_CONTEXT, RTF_KW_ffownhelp }, /* 97 */
	{"ffownstat", true, false, NO_CONTEXT, RTF_KW_ffownstat }, /* 97 */
	{"ffprot", true, false, NO_CONTEXT, RTF_KW_ffprot }, /* 97 */
	{"ffrecalc", true, false, NO_CONTEXT, RTF_KW_ffrecalc }, /* 97 */
	{"ffres", true, false, NO_CONTEXT, RTF_KW_ffres }, /* 97 */
	{"ffsize", true, false, NO_CONTEXT, RTF_KW_ffsize }, /* 97 */
	{"ffstattext", false, false, NO_CONTEXT, RTF_KW_ffstattext }, /* 97 */
	{"fftype", true, false, NO_CONTEXT, RTF_KW_fftype }, /* 97 */
	{"fftypetxt", true, false, NO_CONTEXT, RTF_KW_fftypetxt }, /* 97 */
	{"fi", false, false, NO_CONTEXT, RTF_KW_fi },
	{"fid", false, false, NO_CONTEXT, RTF_KW_fid },
	{"field", false, false, NO_CONTEXT, RTF_KW_field },
	{"file", false, false, NO_CONTEXT, RTF_KW_file },
	{"filetbl", false, false, NO_CONTEXT, RTF_KW_filetbl },
	{"fillColor", false, false, NO_CONTEXT, RTF_KW_fillColor },
	{"fittext", true, false, NO_CONTEXT, RTF_KW_fittext }, /* 2000 */
	{"fldalt", false, false, NO_CONTEXT, RTF_KW_fldalt },
	{"flddirty", false, false, NO_CONTEXT, RTF_KW_flddirty },
	{"fldedit", false, false, NO_CONTEXT, RTF_KW_fldedit },
	{"fldinst", false, false, NO_CONTEXT, RTF_KW_fldinst },
	{"fldlock", false, false, NO_CONTEXT, RTF_KW_fldlock },
	{"fldpriv", false, false, NO_CONTEXT, RTF_KW_fldpriv },
	{"fldrslt", false, false, NO_CONTEXT, RTF_KW_fldrslt },
	{"fldtype", false, false, NO_CONTEXT, RTF_KW_fldtype }, /* 97 */
	{"fmodern", false, false, NO_CONTEXT, RTF_KW_fmodern },
	{"fn", false, false, NO_CONTEXT, RTF_KW_fn },
	{"fname", false, false, NO_CONTEXT, RTF_KW_fname }, /* 7.0 */
	{"fnetwork", false, false, NO_CONTEXT, RTF_KW_fnetwork },
	{"fnil", false, false, NO_CONTEXT, RTF_KW_fnil },
	{"fnonfilesys", false, false, NO_CONTEXT, RTF_KW_fnonfilesys }, /* 2002 */
	{"fontemb", false, false, NO_CONTEXT, RTF_KW_fontemb },
	{"fontfile", false, false, NO_CONTEXT, RTF_KW_fontfile },
	{"fonttbl", false, false, NO_CONTEXT, RTF_KW_fonttbl },
	{"footer", false, false, NO_CONTEXT, RTF_KW_footer },
	{"footerf", false, false, NO_CONTEXT, RTF_KW_footerf },
	{"footerl", false, false, NO_CONTEXT, RTF_KW_footerl },
	{"footerr", false, false, NO_CONTEXT, RTF_KW_footerr },
	{"footery", false, false, NO_CONTEXT, RTF_KW_footery },
	{"footnote", false, false, NO_CONTEXT, RTF_KW_footnote },
	{"formdisp", false, false, NO_CONTEXT, RTF_KW_formdisp },
	{"formfield", false, false, NO_CONTEXT, RTF_KW_formfield }, /* 97 */
	{"formprot", false, false, NO_CONTEXT, RTF_KW_formprot },
	{"formshade", false, false, NO_CONTEXT, RTF_KW_formshade },
	{"fosnum", false, false, NO_CONTEXT, RTF_KW_fosnum },
	{"fprq", false, false, NO_CONTEXT, RTF_KW_fprq },
	{"fracwidth", false, false, NO_CONTEXT, RTF_KW_fracwidth },
	{"frelative", false, false, NO_CONTEXT, RTF_KW_frelative },
	{"frmtxbtlr", false, false, NO_CONTEXT, RTF_KW_frmtxbtlr }, /* 7.0 */
	{"frmtxlrtb", false, false, NO_CONTEXT, RTF_KW_frmtxlrtb }, /* 7.0 */
	{"frmtxlrtbv", false, false, NO_CONTEXT, RTF_KW_frmtxlrtbv }, /* 7.0 */
	{"frmtxtbrl", false, false, NO_CONTEXT, RTF_KW_frmtxtbrl }, /* 7.0 */
	{"frmtxtbrlv", false, false, NO_CONTEXT, RTF_KW_frmtxtbrlv }, /* 7.0 */
	{"froman", false, false, NO_CONTEXT, RTF_KW_froman },
	{"fromhtml", false, false, NO_CONTEXT, RTF_KW_fromhtml }, /* 97 */
	{"fromtext", false, false, NO_CONTEXT, RTF_KW_fromtext }, /* 97 */
	{"fs", false, false, NO_CONTEXT, RTF_KW_fs },
	{"fscript", false, false, NO_CONTEXT, RTF_KW_fscript },
	{"fswiss", false, false, NO_CONTEXT, RTF_KW_fswiss },
	{"ftech", false, false, NO_CONTEXT, RTF_KW_ftech },
	{"ftnalt", false, false, NO_CONTEXT, RTF_KW_ftnalt },
	{"ftnbj", false, false, NO_CONTEXT, RTF_KW_ftnbj },
	{"ftncn", false, false, NO_CONTEXT, RTF_KW_ftncn },
	{"ftnil", false, false, NO_CONTEXT, RTF_KW_ftnil },
	{"ftnlytwnine", false, false, NO_CONTEXT, RTF_KW_ftnlytwnine }, /* 2000 */
	{"ftnnalc", false, false, NO_CONTEXT, RTF_KW_ftnnalc },
	{"ftnnar", false, false, NO_CONTEXT, RTF_KW_ftnnar },
	{"ftnnauc", false, false, NO_CONTEXT, RTF_KW_ftnnauc },
	{"ftnnchi", false, false, NO_CONTEXT, RTF_KW_ftnnchi },
	{"ftnnchosung", false, false, NO_CONTEXT, RTF_KW_ftnnchosung }, /* 97 */
	{"ftnncnum", false, false, NO_CONTEXT, RTF_KW_ftnncnum }, /* 97 */
	{"ftnndbar", false, false, NO_CONTEXT, RTF_KW_ftnndbar }, /* 97 */
	{"ftnndbnum", false, false, NO_CONTEXT, RTF_KW_ftnndbnum }, /* 97 */
	{"ftnndbnumd", false, false, NO_CONTEXT, RTF_KW_ftnndbnumd }, /* 97 */
	{"ftnndbnumk", false, false, NO_CONTEXT, RTF_KW_ftnndbnumk }, /* 97 */
	{"ftnndbnumt", false, false, NO_CONTEXT, RTF_KW_ftnndbnumt }, /* 97 */
	{"ftnnganada", false, false, NO_CONTEXT, RTF_KW_ftnnganada }, /* 97 */
	{"ftnngbnum", false, false, NO_CONTEXT, RTF_KW_ftnngbnum }, /* 97 */
	{"ftnngbnumd", false, false, NO_CONTEXT, RTF_KW_ftnngbnumd }, /* 97 */
	{"ftnngbnumk", false, false, NO_CONTEXT, RTF_KW_ftnngbnumk }, /* 97 */
	{"ftnngbnuml", false, false, NO_CONTEXT, RTF_KW_ftnngbnuml }, /* 97 */
	{"ftnnrlc", false, false, NO_CONTEXT, RTF_KW_ftnnrlc },
	{"ftnnruc", false, false, NO_CONTEXT, RTF_KW_ftnnruc },
	{"ftnnzodiac", false, false, NO_CONTEXT, RTF_KW_ftnnzodiac }, /* 97 */
	{"ftnnzodiacd", false, false, NO_CONTEXT, RTF_KW_ftnnzodiacd }, /* 97 */
	{"ftnnzodiacl", false, false, NO_CONTEXT, RTF_KW_ftnnzodiacl }, /* 97 */
	{"ftnrestart", false, false, NO_CONTEXT, RTF_KW_ftnrestart },
	{"ftnrstcont", false, false, NO_CONTEXT, RTF_KW_ftnrstcont },
	{"ftnrstpg", false, false, NO_CONTEXT, RTF_KW_ftnrstpg },
	{"ftnsep", false, false, NO_CONTEXT, RTF_KW_ftnsep },
	{"ftnsepc", false, false, NO_CONTEXT, RTF_KW_ftnsepc },
	{"ftnstart", false, false, NO_CONTEXT, RTF_KW_ftnstart },
	{"ftntj", false, false, NO_CONTEXT, RTF_KW_ftntj },
	{"fttruetype", false, false, NO_CONTEXT, RTF_KW_fttruetype },
	{"fvaliddos", false, false, NO_CONTEXT, RTF_KW_fvaliddos },
	{"fvalidhpfs", false, false, NO_CONTEXT, RTF_KW_fvalidhpfs },
	{"fvalidmac", false, false, NO_CONTEXT, RTF_KW_fvalidmac },
	{"fvalidntfs", false, false, NO_CONTEXT, RTF_KW_fvalidntfs },
	{"g", false, false, NO_CONTEXT, RTF_KW_g }, /* 97 */
	{"gcw", false, false, NO_CONTEXT, RTF_KW_gcw }, /* 97 */
	{"generator", false, false, NO_CONTEXT, RTF_KW_generator }, /* 2002 */
	{"green", false, false, NO_CONTEXT, RTF_KW_green },
	{"gridtbl", false, false, NO_CONTEXT, RTF_KW_gridtbl }, /* 97 */
	{"gutter", false, false, NO_CONTEXT, RTF_KW_gutter },
	{"gutterprl", false, false, NO_CONTEXT, RTF_KW_gutterprl }, /* 7.0 */
	{"guttersxn", false, false, NO_CONTEXT, RTF_KW_guttersxn },
	{"header", false, false, NO_CONTEXT, RTF_KW_header },
	{"headerf", false, false, NO_CONTEXT, RTF_KW_headerf },
	{"headerl", false, false, NO_CONTEXT, RTF_KW_headerl },
	{"headerr", false, false, NO_CONTEXT, RTF_KW_headerr },
	{"headery", false, false, NO_CONTEXT, RTF_KW_headery },
	{"hich", false, false, NO_CONTEXT, RTF_KW_hich }, /* 7.0 */
	{"highlight", false, false, NO_CONTEXT, RTF_KW_highlight }, /* 7.0 */
	{"hlfr", false, false, NO_CONTEXT, RTF_KW_hlfr }, /* 97 */
	{"hlinkbase", false, false, NO_CONTEXT, RTF_KW_hlinkbase }, /* 97 */
	{"hlloc", false, false, NO_CONTEXT, RTF_KW_hlloc }, /* 97 */
	{"hlsrc", false, false, NO_CONTEXT, RTF_KW_hlsrc }, /* 97 */
	{"horzdoc", false, false, NO_CONTEXT, RTF_KW_horzdoc }, /* 7.0 */
	{"horzsect", false, false, NO_CONTEXT, RTF_KW_horzsect }, /* 7.0 */
	{"hr", false, false, NO_CONTEXT, RTF_KW_hr },
	{"htmautsp", false, false, NO_CONTEXT, RTF_KW_htmautsp }, /* 2000 */
	{"htmlbase", false, false, NO_CONTEXT, RTF_KW_htmlbase },
	{"htmlrtf", false, false, NO_CONTEXT, RTF_KW_htmlrtf },
	{"htmltag", false, false, NO_CONTEXT, RTF_KW_htmltag },
	{"hyphauto", false, false, NO_CONTEXT, RTF_KW_hyphauto },
	{"hyphcaps", false, false, NO_CONTEXT, RTF_KW_hyphcaps },
	{"hyphconsec", false, false, NO_CONTEXT, RTF_KW_hyphconsec },
	{"hyphhotz", false, false, NO_CONTEXT, RTF_KW_hyphhotz },
	{"hyphpar", false, false, NO_CONTEXT, RTF_KW_hyphpar },
	{"i", false, false, NO_CONTEXT, RTF_KW_i },
	{"id", false, false, NO_CONTEXT, RTF_KW_id },
	{"ilvl", false, false, NO_CONTEXT, RTF_KW_ilvl }, /* 97 */
	{"impr", false, false, NO_CONTEXT, RTF_KW_impr }, /* 97 */
	{"info", false, false, NO_CONTEXT, RTF_KW_info },
	{"insrsid", true, false, NO_CONTEXT, RTF_KW_insrsid }, /* 2002 */
	{"intbl", false, false, NO_CONTEXT, RTF_KW_intbl },
	{"ipgp", true, false, NO_CONTEXT, RTF_KW_ipgp }, /* 2002 */
	{"irowband", true, false, NO_CONTEXT, RTF_KW_irowband }, /* 2002 */
	{"irow", true, false, NO_CONTEXT, RTF_KW_irow }, /* 2002 */
	{"itap", true, false, NO_CONTEXT, RTF_KW_itap }, /* 2000 */
	{"ixe", false, false, NO_CONTEXT, RTF_KW_ixe },
	{"jcompress", false, false, NO_CONTEXT, RTF_KW_jcompress }, /* 7.0 */
	{"jexpand", false, false, NO_CONTEXT, RTF_KW_jexpand }, /* 7.0 */
	{"jpegblip", false, false, NO_CONTEXT, RTF_KW_jpegblip }, /* 97 */
	{"jsksu", false, false, NO_CONTEXT, RTF_KW_jsksu }, /* 2000 */
	{"keep", false, false, NO_CONTEXT, RTF_KW_keep },
	{"keepn", false, false, NO_CONTEXT, RTF_KW_keepn },
	{"kerning", false, false, NO_CONTEXT, RTF_KW_kerning },
	{"keycode", false, false, NO_CONTEXT, RTF_KW_keycode },
	{"keywords", false, false, NO_CONTEXT, RTF_KW_keywords },
	{"ksulang", true, false, NO_CONTEXT, RTF_KW_ksulang }, /* 2000 */
	{"landscape", false, false, NO_CONTEXT, RTF_KW_landscape },
	{"lang", false, false, NO_CONTEXT, RTF_KW_lang },
	{"langfe", true, false, NO_CONTEXT, RTF_KW_langfe }, /* 2000 */
	{"langfenp", true, false, NO_CONTEXT, RTF_KW_langfenp }, /* 2000 */
	{"langnp", true, false, NO_CONTEXT, RTF_KW_langnp }, /* 2000 */
	{"lastrow", false, false, NO_CONTEXT, RTF_KW_lastrow }, /* 2002 */
	{"lbr", true, false, NO_CONTEXT, RTF_KW_lbr }, /* 2000 */
	{"lchars", false, false, NO_CONTEXT, RTF_KW_lchars }, /* 7.0 */
	{"ldblquote", false, false, NO_CONTEXT, RTF_KW_ldblquote },
	{"level", false, false, NO_CONTEXT, RTF_KW_level },
	{"levelfollow", true, false, NO_CONTEXT, RTF_KW_levelfollow }, /* 97 */
	{"levelindent", true, false, NO_CONTEXT, RTF_KW_levelindent }, /* 97 */
	{"leveljc", true, false, NO_CONTEXT, RTF_KW_leveljc }, /* 97 */
	{"leveljcn", true, false, NO_CONTEXT, RTF_KW_leveljcn }, /* 2000 */
	{"levellegal", true, false, NO_CONTEXT, RTF_KW_levellegal }, /* 97 */
	{"levelnfc", true, false, NO_CONTEXT, RTF_KW_levelnfc }, /* 97 */
	{"levelnfcn", true, false, NO_CONTEXT, RTF_KW_levelnfcn }, /* 2000 */
	{"levelnorestart", true, false, NO_CONTEXT, RTF_KW_levelnorestart }, /* 97 */
	{"levelnumbers", false, false, NO_CONTEXT, RTF_KW_levelnumbers }, /* 97 */
	{"levelold", true, false, NO_CONTEXT, RTF_KW_levelold }, /* 97 */
	{"levelpicture", true, false, NO_CONTEXT, RTF_KW_levelpicture }, /* 2002 */
	{"levelprev", true, false, NO_CONTEXT, RTF_KW_levelprev }, /* 97 */
	{"levelprevspace", true, false, NO_CONTEXT, RTF_KW_levelprevspace }, /* 97 */
	{"levelspace", true, false, NO_CONTEXT, RTF_KW_levelspace }, /* 97 */
	{"levelstartat", true, false, NO_CONTEXT, RTF_KW_levelstartat }, /* 97 */
	{"leveltemplateid", true, false, NO_CONTEXT, RTF_KW_leveltemplateid }, /* 2000 */
	{"leveltext", false, false, NO_CONTEXT, RTF_KW_leveltext }, /* 97 */
	{"li", false, false, NO_CONTEXT, RTF_KW_li },
	{"line", false, false, NO_CONTEXT, RTF_KW_line },
	{"linebetcol", false, false, NO_CONTEXT, RTF_KW_linebetcol },
	{"linecont", false, false, NO_CONTEXT, RTF_KW_linecont },
	{"linemod", false, false, NO_CONTEXT, RTF_KW_linemod },
	{"lineppage", false, false, NO_CONTEXT, RTF_KW_lineppage },
	{"linerestart", false, false, NO_CONTEXT, RTF_KW_linerestart },
	{"linestart", false, false, NO_CONTEXT, RTF_KW_linestart },
	{"linestarts", false, false, NO_CONTEXT, RTF_KW_linestarts },
	{"linex", false, false, NO_CONTEXT, RTF_KW_linex },
	{"linkself", false, false, NO_CONTEXT, RTF_KW_linkself },
	{"linkstyles", false, false, NO_CONTEXT, RTF_KW_linkstyles },
	{"linkval", false, false, NO_CONTEXT, RTF_KW_linkval }, /* 7.0 */
	{"lin", true, false, NO_CONTEXT, RTF_KW_lin }, /* 2000 */
	{"lisa", true, false, NO_CONTEXT, RTF_KW_lisa }, /* 2000 */
	{"lisb", true, false, NO_CONTEXT, RTF_KW_lisb }, /* 2000 */
	{"listhybrid", false, false, NO_CONTEXT, RTF_KW_listhybrid }, /* 2000 */
	{"listid", true, false, NO_CONTEXT, RTF_KW_listid }, /* 97 */
	{"listname", false, false, NO_CONTEXT, RTF_KW_listname }, /* 97 */
	{"listoverridecount", true, false, NO_CONTEXT, RTF_KW_listoverridecount }, /* 97 */
	{"listoverrideformat", true, false, NO_CONTEXT, RTF_KW_listoverrideformat }, /* 97 */
	{"listoverridestart", true, false, NO_CONTEXT, RTF_KW_listoverridestart }, /* 97 */
	{"listoverridetable", false, false, NO_CONTEXT, RTF_KW_listoverridetable },
	{"listpicture", true, false, NO_CONTEXT, RTF_KW_listpicture }, /* 2002 */
	{"listrestarthdn", true, false, NO_CONTEXT, RTF_KW_listrestarthdn }, /* 97 */
	{"listsimple", true, false, NO_CONTEXT, RTF_KW_listsimple }, /* 97 */
	{"liststyleid", true, false, NO_CONTEXT, RTF_KW_liststyleid }, /* 2002 */
	{"liststylename", false, false, NO_CONTEXT, RTF_KW_liststylename }, /* 2002 */
	{"listtable", false, false, NO_CONTEXT, RTF_KW_listtable },
	{"listtag", false, false, NO_CONTEXT, RTF_KW_listtag },
	{"listtemplateid", true, false, NO_CONTEXT, RTF_KW_listtemplateid }, /* 97 */
	{"listtext", false, false, NO_CONTEXT, RTF_KW_listtext }, /* 97 */
	{"lnbrkrule", false, false, NO_CONTEXT, RTF_KW_lnbrkrule }, /* 2000 */
	{"lndscpsxn", false, false, NO_CONTEXT, RTF_KW_lndscpsxn },
	{"lnongrid", false, false, NO_CONTEXT, RTF_KW_lnongrid }, /* 7.0 */
	{"loch", false, false, NO_CONTEXT, RTF_KW_loch }, /* 7.0 */
	{"lquote", false, false, NO_CONTEXT, RTF_KW_lquote },
	{"ls", false, false, NO_CONTEXT, RTF_KW_ls }, /* 97 */
	{"ltrch", false, false, NO_CONTEXT, RTF_KW_ltrch },
	{"ltrdoc", false, false, NO_CONTEXT, RTF_KW_ltrdoc },
	{"ltrmark", false, false, NO_CONTEXT, RTF_KW_ltrmark }, /* 2002 */
	{"ltrpar", false, false, NO_CONTEXT, RTF_KW_ltrpar },
	{"ltrrow", false, false, NO_CONTEXT, RTF_KW_ltrrow },
	{"ltrsect", false, false, NO_CONTEXT, RTF_KW_ltrsect },
	{"lytcalctblwd", false, false, NO_CONTEXT, RTF_KW_lytcalctblwd }, /* 2000 */
	{"lytexcttp", false, false, NO_CONTEXT, RTF_KW_lytexcttp }, /* 97 */
	{"lytprtmet", false, false, NO_CONTEXT, RTF_KW_lytprtmet }, /* 97 */
	{"lyttblrtgr", false, false, NO_CONTEXT, RTF_KW_lyttblrtgr }, /* 2000 */
	{"mac", false, false, NO_CONTEXT, RTF_KW_mac },
	{"macpict", false, false, NO_CONTEXT, RTF_KW_macpict },
	{"makebackup", false, false, NO_CONTEXT, RTF_KW_makebackup },
	{"manager", false, false, NO_CONTEXT, RTF_KW_manager }, /* 7.0 */
	{"margb", false, false, NO_CONTEXT, RTF_KW_margb },
	{"margbsxn", false, false, NO_CONTEXT, RTF_KW_margbsxn },
	{"margl", false, false, NO_CONTEXT, RTF_KW_margl },
	{"marglsxn", false, false, NO_CONTEXT, RTF_KW_marglsxn },
	{"margmirror", false, false, NO_CONTEXT, RTF_KW_margmirror },
	{"margr", false, false, NO_CONTEXT, RTF_KW_margr },
	{"margrsxn", false, false, NO_CONTEXT, RTF_KW_margrsxn },
	{"margt", false, false, NO_CONTEXT, RTF_KW_margt },
	{"margtsxn", false, false, NO_CONTEXT, RTF_KW_margtsxn },
	{"mhtmltag", false, false, NO_CONTEXT, RTF_KW_mhtmltag },
	{"min", false, false, NO_CONTEXT, RTF_KW_min },
	{"mo", false, false, NO_CONTEXT, RTF_KW_mo },
	{"msmcap", false, false, NO_CONTEXT, RTF_KW_msmcap }, /* 97 */
	{"nestcell", false, false, NO_CONTEXT, RTF_KW_nestcell }, /* 2000 */
	{"nestrow", false, false, NO_CONTEXT, RTF_KW_nestrow }, /* 2000 */
	{"nesttableprops", false, false, NO_CONTEXT, RTF_KW_nesttableprops }, /* 2000 */
	{"nextfile", false, false, NO_CONTEXT, RTF_KW_nextfile },
	{"nobrkwrptbl", false, false, NO_CONTEXT, RTF_KW_nobrkwrptbl }, /* 2002 */
	{"nocolbal", false, false, NO_CONTEXT, RTF_KW_nocolbal },
	{"nocompatoptions", false, false, NO_CONTEXT, RTF_KW_nocompatoptions }, /* 2002 */
	{"nocwrap", false, false, NO_CONTEXT, RTF_KW_nocwrap }, /* 7.0 */
	{"noextrasprl", false, false, NO_CONTEXT, RTF_KW_noextrasprl },
	{"nofchars", false, false, NO_CONTEXT, RTF_KW_nofchars },
	{"nofcharsws", false, false, NO_CONTEXT, RTF_KW_nofcharsws }, /* 97 */
	{"nofpages", false, false, NO_CONTEXT, RTF_KW_nofpages },
	{"nofwords", false, false, NO_CONTEXT, RTF_KW_nofwords },
	{"nolead", false, false, NO_CONTEXT, RTF_KW_nolead }, /* 97 */
	{"noline", false, false, NO_CONTEXT, RTF_KW_noline },
	{"nolnhtadjtbl", false, false, NO_CONTEXT, RTF_KW_nolnhtadjtbl }, /* 2000 */
	{"nonesttables", false, false, NO_CONTEXT, RTF_KW_nonesttables }, /* 2000 */
	{"nonshppict", false, false, NO_CONTEXT, RTF_KW_nonshppict }, /* 97 */
	{"nooverflow", false, false, NO_CONTEXT, RTF_KW_nooverflow }, /* 7.0 */
	{"noproof", false, false, NO_CONTEXT, RTF_KW_noproof }, /* 2000 */
	{"nosectexpand", false, false, NO_CONTEXT, RTF_KW_nosectexpand }, /* 97 */
	{"nosnaplinegrid", false, false, NO_CONTEXT, RTF_KW_nosnaplinegrid }, /* 97 */
	{"nospaceforul", false, false, NO_CONTEXT, RTF_KW_nospaceforul }, /* 97 */
	{"nosupersub", false, false, NO_CONTEXT, RTF_KW_nosupersub },
	{"notabind", false, false, NO_CONTEXT, RTF_KW_notabind },
	{"noultrlspc", false, false, NO_CONTEXT, RTF_KW_noultrlspc }, /* 97 */
	{"nowidctlpar", false, false, NO_CONTEXT, RTF_KW_nowidctlpar },
	{"nowrap", false, false, NO_CONTEXT, RTF_KW_nowrap },
	{"nowwrap", false, false, NO_CONTEXT, RTF_KW_nowwrap }, /* 7.0 */
	{"noxlattoyen", false, false, NO_CONTEXT, RTF_KW_noxlattoyen }, /* 97 */
	{"objalias", false, false, NO_CONTEXT, RTF_KW_objalias },
	{"objalign", false, false, NO_CONTEXT, RTF_KW_objalign },
	{"objattph", false, false, NO_CONTEXT, RTF_KW_objattph }, /* 7.0 */
	{"objautlink", false, false, NO_CONTEXT, RTF_KW_objautlink },
	{"objclass", false, false, NO_CONTEXT, RTF_KW_objclass },
	{"objcropb", false, false, NO_CONTEXT, RTF_KW_objcropb },
	{"objcropl", false, false, NO_CONTEXT, RTF_KW_objcropl },
	{"objcropr", false, false, NO_CONTEXT, RTF_KW_objcropr },
	{"objcropt", false, false, NO_CONTEXT, RTF_KW_objcropt },
	{"objdata", false, false, NO_CONTEXT, RTF_KW_objdata },
	{"object", false, false, NO_CONTEXT, RTF_KW_object },
	{"objemb", false, false, NO_CONTEXT, RTF_KW_objemb },
	{"objh", false, false, NO_CONTEXT, RTF_KW_objh },
	{"objhtml", false, false, NO_CONTEXT, RTF_KW_objhtml }, /* 97 */
	{"objicemb", false, false, NO_CONTEXT, RTF_KW_objicemb },
	{"objlink", false, false, NO_CONTEXT, RTF_KW_objlink },
	{"objlock", false, false, NO_CONTEXT, RTF_KW_objlock },
	{"objname", false, false, NO_CONTEXT, RTF_KW_objname },
	{"objocx", false, false, NO_CONTEXT, RTF_KW_objocx }, /* 97 */
	{"objpub", false, false, NO_CONTEXT, RTF_KW_objpub },
	{"objscalex", false, false, NO_CONTEXT, RTF_KW_objscalex },
	{"objscaley", false, false, NO_CONTEXT, RTF_KW_objscaley },
	{"objsect", false, false, NO_CONTEXT, RTF_KW_objsect },
	{"objsetsize", false, false, NO_CONTEXT, RTF_KW_objsetsize },
	{"objsub", false, false, NO_CONTEXT, RTF_KW_objsub },
	{"objtime", false, false, NO_CONTEXT, RTF_KW_objtime },
	{"objtransy", false, false, NO_CONTEXT, RTF_KW_objtransy },
	{"objupdate", false, false, NO_CONTEXT, RTF_KW_objupdate },
	{"objw", false, false, NO_CONTEXT, RTF_KW_objw },
	{"ol", false, false, NO_CONTEXT, RTF_KW_ol },
	{"oldas", false, false, NO_CONTEXT, RTF_KW_oldas }, /* 2000 */
	{"oldcprops", false, false, NO_CONTEXT, RTF_KW_oldcprops }, /* 2002 */
	{"oldlinewrap", false, false, NO_CONTEXT, RTF_KW_oldlinewrap }, /* 97 */
	{"oldpprops", false, false, NO_CONTEXT, RTF_KW_oldpprops }, /* 2002 */
	{"oldsprops", false, false, NO_CONTEXT, RTF_KW_oldsprops }, /* 2002 */
	{"oldtprops", false, false, NO_CONTEXT, RTF_KW_oldtprops }, /* 2002 */
	{"operator", false, false, NO_CONTEXT, RTF_KW_operator },
	{"otblrul", false, false, NO_CONTEXT, RTF_KW_otblrul },
	{"outl", false, false, NO_CONTEXT, RTF_KW_outl },
	{"outlinelevel", true, false, NO_CONTEXT, RTF_KW_outlinelevel }, /* 97 */
	{"overlay", false, false, NO_CONTEXT, RTF_KW_overlay }, /* 97 */
	{"page", false, false, NO_CONTEXT, RTF_KW_page },
	{"pagebb", false, false, NO_CONTEXT, RTF_KW_pagebb },
	{"panose", false, false, NO_CONTEXT, RTF_KW_panose }, /* 97 */
	{"paperh", false, false, NO_CONTEXT, RTF_KW_paperh },
	{"paperw", false, false, NO_CONTEXT, RTF_KW_paperw },
	{"par", false, false, NO_CONTEXT, RTF_KW_par },
	{"pararsid", true, false, NO_CONTEXT, RTF_KW_pararsid }, /* 2002 */
	{"pard", false, false, NO_CONTEXT, RTF_KW_pard },
	{"pc", false, false, NO_CONTEXT, RTF_KW_pc },
	{"pca", false, false, NO_CONTEXT, RTF_KW_pca },
	{"pgbrdrb", false, false, NO_CONTEXT, RTF_KW_pgbrdrb }, /* 97 */
	{"pgbrdrfoot", false, false, NO_CONTEXT, RTF_KW_pgbrdrfoot }, /* 97 */
	{"pgbrdrhead", false, false, NO_CONTEXT, RTF_KW_pgbrdrhead }, /* 97 */
	{"pgbrdrl", false, false, NO_CONTEXT, RTF_KW_pgbrdrl }, /* 97 */
	{"pgbrdropt", true, false, NO_CONTEXT, RTF_KW_pgbrdropt }, /* 97 */
	{"pgbrdrr", false, false, NO_CONTEXT, RTF_KW_pgbrdrr }, /* 97 */
	{"pgbrdrsnap", false, false, NO_CONTEXT, RTF_KW_pgbrdrsnap }, /* 97 */
	{"pgbrdrt", false, false, NO_CONTEXT, RTF_KW_pgbrdrt }, /* 97 */
	{"pghsxn", false, false, NO_CONTEXT, RTF_KW_pghsxn },
	{"pgnbidia", false, false, NO_CONTEXT, RTF_KW_pgnbidia }, /* 2000 */
	{"pgnbidib", false, false, NO_CONTEXT, RTF_KW_pgnbidib }, /* 2000 */
	{"pgnchosung", false, false, NO_CONTEXT, RTF_KW_pgnchosung }, /* 97 */
	{"pgncnum", false, false, NO_CONTEXT, RTF_KW_pgncnum }, /* 97 */
	{"pgncont", false, false, NO_CONTEXT, RTF_KW_pgncont },
	{"pgndbnum", false, false, NO_CONTEXT, RTF_KW_pgndbnum }, /* 7.0 */
	{"pgndbnumd", false, false, NO_CONTEXT, RTF_KW_pgndbnumd }, /* 7.0 */
	{"pgndbnumk", false, false, NO_CONTEXT, RTF_KW_pgndbnumk }, /* 97 */
	{"pgndbnumt", false, false, NO_CONTEXT, RTF_KW_pgndbnumt }, /* 97 */
	{"pgndec", false, false, NO_CONTEXT, RTF_KW_pgndec },
	{"pgndecd", false, false, NO_CONTEXT, RTF_KW_pgndecd }, /* 7.0 */
	{"pgnganada", false, false, NO_CONTEXT, RTF_KW_pgnganada }, /* 97 */
	{"pgngbnum", false, false, NO_CONTEXT, RTF_KW_pgngbnum }, /* 97 */
	{"pgngbnumd", false, false, NO_CONTEXT, RTF_KW_pgngbnumd }, /* 97 */
	{"pgngbnumk", false, false, NO_CONTEXT, RTF_KW_pgngbnumk }, /* 97 */
	{"pgngbnuml", false, false, NO_CONTEXT, RTF_KW_pgngbnuml }, /* 97 */
	{"pgnhindia", false, false, NO_CONTEXT, RTF_KW_pgnhindia }, /* 2002 */
	{"pgnhindib", false, false, NO_CONTEXT, RTF_KW_pgnhindib }, /* 2002 */
	{"pgnhindic", false, false, NO_CONTEXT, RTF_KW_pgnhindic }, /* 2002 */
	{"pgnhindid", false, false, NO_CONTEXT, RTF_KW_pgnhindid }, /* 2002 */
	{"pgnhn", false, false, NO_CONTEXT, RTF_KW_pgnhn },
	{"pgnhnsc", false, false, NO_CONTEXT, RTF_KW_pgnhnsc },
	{"pgnhnsh", false, false, NO_CONTEXT, RTF_KW_pgnhnsh },
	{"pgnhnsm", false, false, NO_CONTEXT, RTF_KW_pgnhnsm },
	{"pgnhnsn", false, false, NO_CONTEXT, RTF_KW_pgnhnsn },
	{"pgnhnsp", false, false, NO_CONTEXT, RTF_KW_pgnhnsp },
	{"pgnid", true, false, NO_CONTEXT, RTF_KW_pgnid }, /* 2002 */
	{"pgnlcltr", false, false, NO_CONTEXT, RTF_KW_pgnlcltr },
	{"pgnlcrm", false, false, NO_CONTEXT, RTF_KW_pgnlcrm },
	{"pgnrestart", false, false, NO_CONTEXT, RTF_KW_pgnrestart },
	{"pgnstart", false, false, NO_CONTEXT, RTF_KW_pgnstart },
	{"pgnstarts", false, false, NO_CONTEXT, RTF_KW_pgnstarts },
	{"pgnthaia", false, false, NO_CONTEXT, RTF_KW_pgnthaia }, /* 2002 */
	{"pgnthaib", false, false, NO_CONTEXT, RTF_KW_pgnthaib }, /* 2002 */
	{"pgnthaic", false, false, NO_CONTEXT, RTF_KW_pgnthaic }, /* 2002 */
	{"pgnucltr", false, false, NO_CONTEXT, RTF_KW_pgnucltr },
	{"pgnucrm", false, false, NO_CONTEXT, RTF_KW_pgnucrm },
	{"pgnvieta", false, false, NO_CONTEXT, RTF_KW_pgnvieta }, /* 2002 */
	{"pgnx", false, false, NO_CONTEXT, RTF_KW_pgnx },
	{"pgny", false, false, NO_CONTEXT, RTF_KW_pgny },
	{"pgnzodiac", false, false, NO_CONTEXT, RTF_KW_pgnzodiac }, /* 97 */
	{"pgnzodiacd", false, false, NO_CONTEXT, RTF_KW_pgnzodiacd }, /* 97 */
	{"pgnzodiacl", false, false, NO_CONTEXT, RTF_KW_pgnzodiacl }, /* 97 */
	{"pgp", false, false, NO_CONTEXT, RTF_KW_pgp }, /* 2002 */
	{"pgptbl", false, false, NO_CONTEXT, RTF_KW_pgptbl }, /* 2002 */
	{"pgwsxn", false, false, NO_CONTEXT, RTF_KW_pgwsxn },
	{"phcol", false, false, NO_CONTEXT, RTF_KW_phcol },
	{"phmrg", false, false, NO_CONTEXT, RTF_KW_phmrg },
	{"phpg", false, false, NO_CONTEXT, RTF_KW_phpg },
	{"picbmp", false, false, NO_CONTEXT, RTF_KW_picbmp },
	{"picbpp", false, false, NO_CONTEXT, RTF_KW_picbpp },
	{"piccropb", false, false, NO_CONTEXT, RTF_KW_piccropb },
	{"piccropl", false, false, NO_CONTEXT, RTF_KW_piccropl },
	{"piccropr", false, false, NO_CONTEXT, RTF_KW_piccropr },
	{"piccropt", false, false, NO_CONTEXT, RTF_KW_piccropt },
	{"pich", false, false, NO_CONTEXT, RTF_KW_pich },
	{"pichgoal", false, false, NO_CONTEXT, RTF_KW_pichgoal },
	{"picprop", false, false, NO_CONTEXT, RTF_KW_picprop }, /* 97 */
	{"picscaled", false, false, NO_CONTEXT, RTF_KW_picscaled },
	{"picscalex", false, false, NO_CONTEXT, RTF_KW_picscalex },
	{"picscaley", false, false, NO_CONTEXT, RTF_KW_picscaley },
	{"pict", false, false, NO_CONTEXT, RTF_KW_pict },
	{"picw", false, false, NO_CONTEXT, RTF_KW_picw },
	{"picwgoal", false, false, NO_CONTEXT, RTF_KW_picwgoal },
	{"plain", false, false, NO_CONTEXT, RTF_KW_plain },
	{"pmmetafile", false, false, NO_CONTEXT, RTF_KW_pmmetafile },
	{"pn", false, false, NO_CONTEXT, RTF_KW_pn },
	{"pnacross", false, false, NO_CONTEXT, RTF_KW_pnacross },
	{"pnaiu", false, false, NO_CONTEXT, RTF_KW_pnaiu }, /* 7.0 */
	{"pnaiud", false, false, NO_CONTEXT, RTF_KW_pnaiud }, /* 7.0 */
	{"pnaiueo", false, false, NO_CONTEXT, RTF_KW_pnaiueo }, /* 97 */
	{"pnaiueod", false, false, NO_CONTEXT, RTF_KW_pnaiueod }, /* 97 */
	{"pnb", false, false, NO_CONTEXT, RTF_KW_pnb },
	{"pnbidia", false, false, NO_CONTEXT, RTF_KW_pnbidia }, /* 2000 */
	{"pnbidib", false, false, NO_CONTEXT, RTF_KW_pnbidib }, /* 2000 */
	{"pncaps", false, false, NO_CONTEXT, RTF_KW_pncaps },
	{"pncard", false, false, NO_CONTEXT, RTF_KW_pncard },
	{"pncf", false, false, NO_CONTEXT, RTF_KW_pncf },
	{"pnchosung", false, false, NO_CONTEXT, RTF_KW_pnchosung }, /* 97 */
	{"pncnum", false, false, NO_CONTEXT, RTF_KW_pncnum }, /* 7.0 */
	{"pndbnum", false, false, NO_CONTEXT, RTF_KW_pndbnum }, /* 7.0 */
	{"pndbnumd", false, false, NO_CONTEXT, RTF_KW_pndbnumd }, /* 97 */
	{"pndbnumk", false, false, NO_CONTEXT, RTF_KW_pndbnumk }, /* 97 */
	{"pndbnuml", false, false, NO_CONTEXT, RTF_KW_pndbnuml }, /* 97 */
	{"pndbnumt", false, false, NO_CONTEXT, RTF_KW_pndbnumt }, /* 97 */
	{"pndec", false, false, NO_CONTEXT, RTF_KW_pndec },
	{"pndecd", false, false, NO_CONTEXT, RTF_KW_pndecd }, /* 7.0 */
	{"pnf", false, false, NO_CONTEXT, RTF_KW_pnf },
	{"pnfs", false, false, NO_CONTEXT, RTF_KW_pnfs },
	{"pnganada", false, false, NO_CONTEXT, RTF_KW_pnganada }, /* 97 */
	{"pngblip", false, false, NO_CONTEXT, RTF_KW_pngblip }, /* 97 */
	{"pngbnum", false, false, NO_CONTEXT, RTF_KW_pngbnum }, /* 97 */
	{"pngbnumd", false, false, NO_CONTEXT, RTF_KW_pngbnumd }, /* 97 */
	{"pngbnumk", false, false, NO_CONTEXT, RTF_KW_pngbnumk }, /* 97 */
	{"pngbnuml", false, false, NO_CONTEXT, RTF_KW_pngbnuml }, /* 97 */
	{"pnhang", false, false, NO_CONTEXT, RTF_KW_pnhang },
	{"pni", false, false, NO_CONTEXT, RTF_KW_pni },
	{"pnindent", false, false, NO_CONTEXT, RTF_KW_pnindent },
	{"pniroha", false, false, NO_CONTEXT, RTF_KW_pniroha }, /* 7.0 */
	{"pnirohad", false, false, NO_CONTEXT, RTF_KW_pnirohad }, /* 7.0 */
	{"pnlcltr", false, false, NO_CONTEXT, RTF_KW_pnlcltr },
	{"pnlcrm", false, false, NO_CONTEXT, RTF_KW_pnlcrm },
	{"pnlvl", false, false, NO_CONTEXT, RTF_KW_pnlvl },
	{"pnlvlblt", false, false, NO_CONTEXT, RTF_KW_pnlvlblt },
	{"pnlvlbody", false, false, NO_CONTEXT, RTF_KW_pnlvlbody },
	{"pnlvlcont", false, false, NO_CONTEXT, RTF_KW_pnlvlcont },
	{"pnnumonce", false, false, NO_CONTEXT, RTF_KW_pnnumonce },
	{"pnord", false, false, NO_CONTEXT, RTF_KW_pnord },
	{"pnordt", false, false, NO_CONTEXT, RTF_KW_pnordt },
	{"pnprev", false, false, NO_CONTEXT, RTF_KW_pnprev },
	{"pnqc", false, false, NO_CONTEXT, RTF_KW_pnqc },
	{"pnql", false, false, NO_CONTEXT, RTF_KW_pnql },
	{"pnqr", false, false, NO_CONTEXT, RTF_KW_pnqr },
	{"pnrauth", true, false, NO_CONTEXT, RTF_KW_pnrauth }, /* 97 */
	{"pnrdate", true, false, NO_CONTEXT, RTF_KW_pnrdate }, /* 97 */
	{"pnrestart", false, false, NO_CONTEXT, RTF_KW_pnrestart },
	{"pnrnfc", true, false, NO_CONTEXT, RTF_KW_pnrnfc }, /* 97 */
	{"pnrnot", false, false, NO_CONTEXT, RTF_KW_pnrnot }, /* 97 */
	{"pnrpnbr", true, false, NO_CONTEXT, RTF_KW_pnrpnbr }, /* 97 */
	{"pnrrgb", true, false, NO_CONTEXT, RTF_KW_pnrrgb }, /* 97 */
	{"pnrstart", true, false, NO_CONTEXT, RTF_KW_pnrstart }, /* 97 */
	{"pnrstop", true, false, NO_CONTEXT, RTF_KW_pnrstop }, /* 97 */
	{"pnrxst", true, false, NO_CONTEXT, RTF_KW_pnrxst }, /* 97 */
	{"pnscaps", false, false, NO_CONTEXT, RTF_KW_pnscaps },
	{"pnseclvl", false, false, NO_CONTEXT, RTF_KW_pnseclvl },
	{"pnsp", false, false, NO_CONTEXT, RTF_KW_pnsp },
	{"pnstart", false, false, NO_CONTEXT, RTF_KW_pnstart },
	{"pnstrike", false, false, NO_CONTEXT, RTF_KW_pnstrike },
	{"pntext", false, false, NO_CONTEXT, RTF_KW_pntext },
	{"pntxta", false, false, NO_CONTEXT, RTF_KW_pntxta },
	{"pntxtb", false, false, NO_CONTEXT, RTF_KW_pntxtb },
	{"pnucltr", false, false, NO_CONTEXT, RTF_KW_pnucltr },
	{"pnucrm", false, false, NO_CONTEXT, RTF_KW_pnucrm },
	{"pnul", false, false, NO_CONTEXT, RTF_KW_pnul },
	{"pnuld", false, false, NO_CONTEXT, RTF_KW_pnuld },
	{"pnuldash", false, false, NO_CONTEXT, RTF_KW_pnuldash }, /* 7.0 */
	{"pnuldashd", false, false, NO_CONTEXT, RTF_KW_pnuldashd }, /* 7.0 */
	{"pnuldashdd", false, false, NO_CONTEXT, RTF_KW_pnuldashdd }, /* 7.0 */
	{"pnuldb", false, false, NO_CONTEXT, RTF_KW_pnuldb },
	{"pnulhair", false, false, NO_CONTEXT, RTF_KW_pnulhair }, /* 7.0 */
	{"pnulnone", false, false, NO_CONTEXT, RTF_KW_pnulnone },
	{"pnulth", false, false, NO_CONTEXT, RTF_KW_pnulth }, /* 7.0 */
	{"pnulw", false, false, NO_CONTEXT, RTF_KW_pnulw },
	{"pnulwave", false, false, NO_CONTEXT, RTF_KW_pnulwave }, /* 7.0 */
	{"pnzodiac", false, false, NO_CONTEXT, RTF_KW_pnzodiac }, /* 97 */
	{"pnzodiacd", false, false, NO_CONTEXT, RTF_KW_pnzodiacd }, /* 97 */
	{"pnzodiacl", false, false, NO_CONTEXT, RTF_KW_pnzodiacl }, /* 97 */
	{"posnegx", false, false, NO_CONTEXT, RTF_KW_posnegx },
	{"posnegy", false, false, NO_CONTEXT, RTF_KW_posnegy },
	{"posx", false, false, NO_CONTEXT, RTF_KW_posx },
	{"posxc", false, false, NO_CONTEXT, RTF_KW_posxc },
	{"posxi", false, false, NO_CONTEXT, RTF_KW_posxi },
	{"posxl", false, false, NO_CONTEXT, RTF_KW_posxl },
	{"posxo", false, false, NO_CONTEXT, RTF_KW_posxo },
	{"posxr", false, false, NO_CONTEXT, RTF_KW_posxr },
	{"posy", false, false, NO_CONTEXT, RTF_KW_posy },
	{"posyb", false, false, NO_CONTEXT, RTF_KW_posyb },
	{"posyc", false, false, NO_CONTEXT, RTF_KW_posyc },
	{"posyil", false, false, NO_CONTEXT, RTF_KW_posyil },
	{"posyin", false, false, NO_CONTEXT, RTF_KW_posyin }, /* 97 */
	{"posyout", false, false, NO_CONTEXT, RTF_KW_posyout }, /* 97 */
	{"posyt", false, false, NO_CONTEXT, RTF_KW_posyt },
	{"prcolbl", false, false, NO_CONTEXT, RTF_KW_prcolbl },
	{"printdata", false, false, NO_CONTEXT, RTF_KW_printdata },
	{"printim", false, false, NO_CONTEXT, RTF_KW_printim },
	{"private", false, false, NO_CONTEXT, RTF_KW_private }, /* 97 */
	{"propname", false, false, NO_CONTEXT, RTF_KW_propname }, /* 7.0 */
	{"proptype", false, false, NO_CONTEXT, RTF_KW_proptype }, /* 7.0 */
	{"psover", false, false, NO_CONTEXT, RTF_KW_psover },
	{"psz", false, false, NO_CONTEXT, RTF_KW_psz },
	{"pubauto", false, false, NO_CONTEXT, RTF_KW_pubauto },
	{"pvmrg", false, false, NO_CONTEXT, RTF_KW_pvmrg },
	{"pvpara", false, false, NO_CONTEXT, RTF_KW_pvpara },
	{"pvpg", false, false, NO_CONTEXT, RTF_KW_pvpg },
	{"pwd", true, false, NO_CONTEXT, RTF_KW_pwd },
	{"pxe", false, false, NO_CONTEXT, RTF_KW_pxe }, /* 7.0 */
	{"qc", false, false, NO_CONTEXT, RTF_KW_qc },
	{"qd", false, false, NO_CONTEXT, RTF_KW_qd }, /* 7.0 */
	{"qj", false, false, NO_CONTEXT, RTF_KW_qj },
	{"qk", false, false, NO_CONTEXT, RTF_KW_qk }, /* 2002 */
	{"ql", false, false, NO_CONTEXT, RTF_KW_ql },
	{"qmspace", false, false, NO_CONTEXT, RTF_KW_qmspace }, /* 7.0 */
	{"qr", false, false, NO_CONTEXT, RTF_KW_qr },
	{"qt", false, false, NO_CONTEXT, RTF_KW_qt }, /* 2002 */
	{"rawbgdkbdiag", false, false, NO_CONTEXT, RTF_KW_rawbgdkbdiag }, /* 2002 */
	{"rawclbgbdiag", false, false, NO_CONTEXT, RTF_KW_rawclbgbdiag }, /* 2002 */
	{"rawclbgcross", false, false, NO_CONTEXT, RTF_KW_rawclbgcross }, /* 2002 */
	{"rawclbgdcross", false, false, NO_CONTEXT, RTF_KW_rawclbgdcross }, /* 2002 */
	{"rawclbgdkcross", false, false, NO_CONTEXT, RTF_KW_rawclbgdkcross }, /* 2002 */
	{"rawclbgdkdcross", false, false, NO_CONTEXT, RTF_KW_rawclbgdkdcross }, /* 2002 */
	{"rawclbgdkfdiag", false, false, NO_CONTEXT, RTF_KW_rawclbgdkfdiag }, /* 2002 */
	{"rawclbgdkhor", false, false, NO_CONTEXT, RTF_KW_rawclbgdkhor }, /* 2002 */
	{"rawclbgdkvert", false, false, NO_CONTEXT, RTF_KW_rawclbgdkvert }, /* 2002 */
	{"rawclbgfdiag", false, false, NO_CONTEXT, RTF_KW_rawclbgfdiag }, /* 2002 */
	{"rawclbghoriz", false, false, NO_CONTEXT, RTF_KW_rawclbghoriz }, /* 2002 */
	{"rawclbgvert", false, false, NO_CONTEXT, RTF_KW_rawclbgvert }, /* 2002 */
	{"rdblquote", false, false, NO_CONTEXT, RTF_KW_rdblquote },
	{"rdf", false, false, NO_CONTEXT, RTF_KW_rdf },
	{"rdfanchorstart", false, false, NO_CONTEXT, RTF_KW_rdfanchorstart },
	{"rdfanchorend", false, false, NO_CONTEXT, RTF_KW_rdfanchorend },
	{"red", false, false, NO_CONTEXT, RTF_KW_red },
	{"rempersonalinfo", false, false, NO_CONTEXT, RTF_KW_rempersonalinfo }, /* 2002 */
	{"result", false, false, NO_CONTEXT, RTF_KW_result },
	{"revauth", false, false, NO_CONTEXT, RTF_KW_revauth },
	{"revauthdel", true, false, NO_CONTEXT, RTF_KW_revauthdel }, /* 97 */
	{"revbar", false, false, NO_CONTEXT, RTF_KW_revbar },
	{"revdttm", false, false, NO_CONTEXT, RTF_KW_revdttm },
	{"revdttmdel", true, false, NO_CONTEXT, RTF_KW_revdttmdel }, /* 97 */
	{"revised", false, false, NO_CONTEXT, RTF_KW_revised },
	{"revisions", false, false, NO_CONTEXT, RTF_KW_revisions },
	{"revprop", false, false, NO_CONTEXT, RTF_KW_revprop },
	{"revprot", false, false, NO_CONTEXT, RTF_KW_revprot },
	{"revtbl", false, false, NO_CONTEXT, RTF_KW_revtbl },
	{"revtim", false, false, NO_CONTEXT, RTF_KW_revtim },
	{"ri", false, false, NO_CONTEXT, RTF_KW_ri },
	{"rin", true, false, NO_CONTEXT, RTF_KW_rin }, /* 2000 */
	{"row", false, false, NO_CONTEXT, RTF_KW_row },
	{"rquote", false, false, NO_CONTEXT, RTF_KW_rquote },
	{"rsid", true, false, NO_CONTEXT, RTF_KW_rsid }, /* 2002 */
	{"rsidroot", true, false, NO_CONTEXT, RTF_KW_rsidroot }, /* 2002 */
	{"rsidtbl", false, false, NO_CONTEXT, RTF_KW_rsidtbl }, /* 2002 */
	{"rsltbmp", false, false, NO_CONTEXT, RTF_KW_rsltbmp },
	{"rslthtml", false, false, NO_CONTEXT, RTF_KW_rslthtml }, /* 2000 */
	{"rsltmerge", false, false, NO_CONTEXT, RTF_KW_rsltmerge },
	{"rsltpict", false, false, NO_CONTEXT, RTF_KW_rsltpict },
	{"rsltrtf", false, false, NO_CONTEXT, RTF_KW_rsltrtf },
	{"rslttxt", false, false, NO_CONTEXT, RTF_KW_rslttxt },
	{"rtf", false, false, NO_CONTEXT, RTF_KW_rtf },
	{"rtlch", false, false, NO_CONTEXT, RTF_KW_rtlch },
	{"rtldoc", false, false, NO_CONTEXT, RTF_KW_rtldoc },
	{"rtlgutter", false, false, NO_CONTEXT, RTF_KW_rtlgutter }, /* 2000 */
	{"rtlmark", false, false, NO_CONTEXT, RTF_KW_rtlmark }, /* 2002 */
	{"rtlpar", false, false, NO_CONTEXT, RTF_KW_rtlpar },
	{"rtlrow", false, false, NO_CONTEXT, RTF_KW_rtlrow },
	{"rtlsect", false, false, NO_CONTEXT, RTF_KW_rtlsect },
	{"rxe", false, false, NO_CONTEXT, RTF_KW_rxe },
	{"s", false, false, NO_CONTEXT, RTF_KW_s },
	{"sa", false, false, NO_CONTEXT, RTF_KW_sa },
	{"saauto", true, false, NO_CONTEXT, RTF_KW_saauto }, /* 2000 */
	{"saftnnalc", false, false, NO_CONTEXT, RTF_KW_saftnnalc }, /* 2002 */
	{"saftnnar", false, false, NO_CONTEXT, RTF_KW_saftnnar }, /* 2002 */
	{"saftnnauc", false, false, NO_CONTEXT, RTF_KW_saftnnauc }, /* 2002 */
	{"saftnnchi", false, false, NO_CONTEXT, RTF_KW_saftnnchi }, /* 2002 */
	{"saftnnchosung", false, false, NO_CONTEXT, RTF_KW_saftnnchosung }, /* 2002 */
	{"saftnncnum", false, false, NO_CONTEXT, RTF_KW_saftnncnum }, /* 2002 */
	{"saftnndbar", false, false, NO_CONTEXT, RTF_KW_saftnndbar }, /* 2002 */
	{"saftnndbnum", false, false, NO_CONTEXT, RTF_KW_saftnndbnum }, /* 2002 */
	{"saftnndbnumd", false, false, NO_CONTEXT, RTF_KW_saftnndbnumd }, /* 2002 */
	{"saftnndbnumk", false, false, NO_CONTEXT, RTF_KW_saftnndbnumk }, /* 2002 */
	{"saftnndbnumt", false, false, NO_CONTEXT, RTF_KW_saftnndbnumt }, /* 2002 */
	{"saftnnganada", false, false, NO_CONTEXT, RTF_KW_saftnnganada }, /* 2002 */
	{"saftnngbnum", false, false, NO_CONTEXT, RTF_KW_saftnngbnum }, /* 2002 */
	{"saftnngbnumd", false, false, NO_CONTEXT, RTF_KW_saftnngbnumd }, /* 2002 */
	{"saftnngbnumk", false, false, NO_CONTEXT, RTF_KW_saftnngbnumk }, /* 2002 */
	{"saftnngbnuml", false, false, NO_CONTEXT, RTF_KW_saftnngbnuml }, /* 2002 */
	{"saftnnrlc", false, false, NO_CONTEXT, RTF_KW_saftnnrlc }, /* 2002 */
	{"saftnnruc", false, false, NO_CONTEXT, RTF_KW_saftnnruc }, /* 2002 */
	{"saftnnzodiac", false, false, NO_CONTEXT, RTF_KW_saftnnzodiac }, /* 2002 */
	{"saftnnzodiacd", false, false, NO_CONTEXT, RTF_KW_saftnnzodiacd }, /* 2002 */
	{"saftnnzodiacl", false, false, NO_CONTEXT, RTF_KW_saftnnzodiacl }, /* 2002 */
	{"saftnrestart", false, false, NO_CONTEXT, RTF_KW_saftnrestart }, /* 2002 */
	{"saftnrstcont", false, false, NO_CONTEXT, RTF_KW_saftnrstcont }, /* 2002 */
	{"saftnstart", false, false, NO_CONTEXT, RTF_KW_saftnstart }, /* 2002 */
	{"sautoupd", false, false, NO_CONTEXT, RTF_KW_sautoupd }, /* 97 */
	{"sb", false, false, NO_CONTEXT, RTF_KW_sb },
	{"sbasedon", false, false, NO_CONTEXT, RTF_KW_sbasedon },
	{"sbauto", true, false, NO_CONTEXT, RTF_KW_sbauto }, /* 2000 */
	{"sbkcol", false, false, NO_CONTEXT, RTF_KW_sbkcol },
	{"sbkeven", false, false, NO_CONTEXT, RTF_KW_sbkeven },
	{"sbknone", false, false, NO_CONTEXT, RTF_KW_sbknone },
	{"sbkodd", false, false, NO_CONTEXT, RTF_KW_sbkodd },
	{"sbkpage", false, false, NO_CONTEXT, RTF_KW_sbkpage },
	{"sbys", false, false, NO_CONTEXT, RTF_KW_sbys },
	{"scaps", false, false, NO_CONTEXT, RTF_KW_scaps },
	{"scompose", false, false, NO_CONTEXT, RTF_KW_scompose }, /* 2000 */
	{"sec", false, false, NO_CONTEXT, RTF_KW_sec },
	{"sect", false, false, NO_CONTEXT, RTF_KW_sect },
	{"sectd", false, false, NO_CONTEXT, RTF_KW_sectd },
	{"sectdefaultcl", false, false, NO_CONTEXT, RTF_KW_sectdefaultcl }, /* 97 */
	{"sectexpand", true, false, NO_CONTEXT, RTF_KW_sectexpand }, /* 97 */
	{"sectlinegrid", true, false, NO_CONTEXT, RTF_KW_sectlinegrid }, /* 97 */
	{"sectnum", false, false, NO_CONTEXT, RTF_KW_sectnum },
	{"sectrsid", true, false, NO_CONTEXT, RTF_KW_sectrsid }, /* 2002 */
	{"sectspecifycl", false, false, NO_CONTEXT, RTF_KW_sectspecifycl }, /* 97 */
	{"sectspecifygen", true, false, NO_CONTEXT, RTF_KW_sectspecifygen },
	{"sectspecifyl", false, false, NO_CONTEXT, RTF_KW_sectspecifyl }, /* 97 */
	{"sectunlocked", false, false, NO_CONTEXT, RTF_KW_sectunlocked },
	{"sftnbj", false, false, NO_CONTEXT, RTF_KW_sftnbj }, /* 2002 */
	{"sftnnalc", false, false, NO_CONTEXT, RTF_KW_sftnnalc }, /* 2002 */
	{"sftnnar", false, false, NO_CONTEXT, RTF_KW_sftnnar }, /* 2002 */
	{"sftnnauc", false, false, NO_CONTEXT, RTF_KW_sftnnauc }, /* 2002 */
	{"sftnnchi", false, false, NO_CONTEXT, RTF_KW_sftnnchi }, /* 2002 */
	{"sftnnchosung", false, false, NO_CONTEXT, RTF_KW_sftnnchosung }, /* 2002 */
	{"sftnncnum", false, false, NO_CONTEXT, RTF_KW_sftnncnum }, /* 2002 */
	{"sftnndbar", false, false, NO_CONTEXT, RTF_KW_sftnndbar }, /* 2002 */
	{"sftnndbnum", false, false, NO_CONTEXT, RTF_KW_sftnndbnum }, /* 2002 */
	{"sftnndbnumd", false, false, NO_CONTEXT, RTF_KW_sftnndbnumd }, /* 2002 */
	{"sftnndbnumk", false, false, NO_CONTEXT, RTF_KW_sftnndbnumk }, /* 2002 */
	{"sftnndbnumt", false, false, NO_CONTEXT, RTF_KW_sftnndbnumt }, /* 2002 */
	{"sftnnganada", false, false, NO_CONTEXT, RTF_KW_sftnnganada }, /* 2002 */
	{"sftnngbnum", false, false, NO_CONTEXT, RTF_KW_sftnngbnum }, /* 2002 */
	{"sftnngbnumd", false, false, NO_CONTEXT, RTF_KW_sftnngbnumd }, /* 2002 */
	{"sftnngbnumk", false, false, NO_CONTEXT, RTF_KW_sftnngbnumk }, /* 2002 */
	{"sftnngbnuml", false, false, NO_CONTEXT, RTF_KW_sftnngbnuml }, /* 2002 */
	{"sftnnrlc", false, false, NO_CONTEXT, RTF_KW_sftnnrlc }, /* 2002 */
	{"sftnnruc", false, false, NO_CONTEXT, RTF_KW_sftnnruc }, /* 2002 */
	{"sftnnzodiac", false, false, NO_CONTEXT, RTF_KW_sftnnzodiac }, /* 2002 */
	{"sftnnzodiacd", false, false, NO_CONTEXT, RTF_KW_sftnnzodiacd }, /* 2002 */
	{"sftnnzodiacl", false, false, NO_CONTEXT, RTF_KW_sftnnzodiacl }, /* 2002 */
	{"sftnrestart", false, false, NO_CONTEXT, RTF_KW_sftnrestart }, /* 2002 */
	{"sftnrstcont", false, false, NO_CONTEXT, RTF_KW_sftnrstcont }, /* 2002 */
	{"sftnrstpg", false, false, NO_CONTEXT, RTF_KW_sftnrstpg }, /* 2002 */
	{"sftnstart", false, false, NO_CONTEXT, RTF_KW_sftnstart }, /* 2002 */
	{"sftntj", false, false, NO_CONTEXT, RTF_KW_sftntj }, /* 2002 */
	{"shad", false, false, NO_CONTEXT, RTF_KW_shad },
	{"shading", false, false, NO_CONTEXT, RTF_KW_shading },
	{"shidden", false, false, NO_CONTEXT, RTF_KW_shidden }, /* 97 */
	{"shift", false, false, NO_CONTEXT, RTF_KW_shift },
	{"shp", false, false, NO_CONTEXT, RTF_KW_shp },
	{"shpbottom", true, false, NO_CONTEXT, RTF_KW_shpbottom }, /* 97 */
	{"shpbxcolumn", false, false, NO_CONTEXT, RTF_KW_shpbxcolumn }, /* 97 */
	{"shpbxignore", false, false, NO_CONTEXT, RTF_KW_shpbxignore }, /* 2000 */
	{"shpbxmargin", false, false, NO_CONTEXT, RTF_KW_shpbxmargin }, /* 97 */
	{"shpbxpage", false, false, NO_CONTEXT, RTF_KW_shpbxpage }, /* 97 */
	{"shpbyignore", false, false, NO_CONTEXT, RTF_KW_shpbyignore }, /* 2000 */
	{"shpbymargin", false, false, NO_CONTEXT, RTF_KW_shpbymargin }, /* 97 */
	{"shpbypage", false, false, NO_CONTEXT, RTF_KW_shpbypage }, /* 97 */
	{"shpbypara", false, false, NO_CONTEXT, RTF_KW_shpbypara }, /* 97 */
	{"shpfblwtxt", true, false, NO_CONTEXT, RTF_KW_shpfblwtxt }, /* 97 */
	{"shpfhdr", true, false, NO_CONTEXT, RTF_KW_shpfhdr }, /* 97 */
	{"shpgrp", false, false, NO_CONTEXT, RTF_KW_shpgrp }, /* 97 */
	{"shpinst", false, false, NO_CONTEXT, RTF_KW_shpinst }, /* 97 */
	{"shpleft", true, false, NO_CONTEXT, RTF_KW_shpleft }, /* 97 */
	{"shplid", true, false, NO_CONTEXT, RTF_KW_shplid }, /* 97 */
	{"shplockanchor", false, false, NO_CONTEXT, RTF_KW_shplockanchor }, /* 97 */
	{"shppict", false, false, NO_CONTEXT, RTF_KW_shppict }, /* 97 */
	{"shpright", true, false, NO_CONTEXT, RTF_KW_shpright }, /* 97 */
	{"shprslt", false, false, NO_CONTEXT, RTF_KW_shprslt }, /* 97 */
	{"shptop", true, false, NO_CONTEXT, RTF_KW_shptop }, /* 97 */
	{"shptxt", false, false, NO_CONTEXT, RTF_KW_shptxt }, /* 97 */
	{"shpwrk", true, false, NO_CONTEXT, RTF_KW_shpwrk }, /* 97 */
	{"shpwr", true, false, NO_CONTEXT, RTF_KW_shpwr }, /* 97 */
	{"shpz", true, false, NO_CONTEXT, RTF_KW_shpz }, /* 97 */
	{"sl", false, false, NO_CONTEXT, RTF_KW_sl },
	{"slmult", false, false, NO_CONTEXT, RTF_KW_slmult },
	{"sn", false, false, NO_CONTEXT, RTF_KW_sn },
	{"snaptogridincell", false, false, NO_CONTEXT, RTF_KW_snaptogridincell }, /* 2002 */
	{"snext", false, false, NO_CONTEXT, RTF_KW_snext },
	{"softcol", false, false, NO_CONTEXT, RTF_KW_softcol },
	{"softlheight", false, false, NO_CONTEXT, RTF_KW_softlheight },
	{"softline", false, false, NO_CONTEXT, RTF_KW_softline },
	{"softpage", false, false, NO_CONTEXT, RTF_KW_softpage },
	{"sp", false, false, NO_CONTEXT, RTF_KW_sp },
	{"spersonal", false, false, NO_CONTEXT, RTF_KW_spersonal }, /* 2000 */
	{"splytwnine", false, false, NO_CONTEXT, RTF_KW_splytwnine }, /* 2000 */
	{"sprsbsp", false, false, NO_CONTEXT, RTF_KW_sprsbsp }, /* 97 */
	{"sprslnsp", false, false, NO_CONTEXT, RTF_KW_sprslnsp }, /* 7.0 */
	{"sprsspbf", false, false, NO_CONTEXT, RTF_KW_sprsspbf },
	{"sprstsm", false, false, NO_CONTEXT, RTF_KW_sprstsm }, /* 97 */
	{"sprstsp", false, false, NO_CONTEXT, RTF_KW_sprstsp },
	{"spv", false, false, NO_CONTEXT, RTF_KW_spv }, /* 2002 */
	{"sreply", false, false, NO_CONTEXT, RTF_KW_sreply }, /* 2000 */
	{"ssemihidden", false, false, NO_CONTEXT, RTF_KW_ssemihidden }, /* 2002 */
	{"staticval", false, false, NO_CONTEXT, RTF_KW_staticval }, /* 7.0 */
	{"stextflow", false, false, NO_CONTEXT, RTF_KW_stextflow }, /* 97 */
	{"strike", false, false, NO_CONTEXT, RTF_KW_strike },
	{"striked", false, false, NO_CONTEXT, RTF_KW_striked },
	{"stshfbi", true, false, NO_CONTEXT, RTF_KW_stshfbi }, /* 2002 */
	{"stshfdbch", true, false, NO_CONTEXT, RTF_KW_stshfdbch }, /* 2002 */
	{"stshfhich", true, false, NO_CONTEXT, RTF_KW_stshfhich }, /* 2002 */
	{"stshfloch", true, false, NO_CONTEXT, RTF_KW_stshfloch }, /* 2002 */
	{"stylesheet", false, false, NO_CONTEXT, RTF_KW_stylesheet },
	{"styrsid", true, false, NO_CONTEXT, RTF_KW_styrsid }, /* 2002 */
	{"sub", false, false, NO_CONTEXT, RTF_KW_sub },
	{"subdocument", false, false, NO_CONTEXT, RTF_KW_subdocument },
	{"subfontbysize", false, false, NO_CONTEXT, RTF_KW_subfontbysize }, /* 7.0 */
	{"subject", false, false, NO_CONTEXT, RTF_KW_subject },
	{"super", false, false, NO_CONTEXT, RTF_KW_super },
	{"sv", false, false, NO_CONTEXT, RTF_KW_sv },
	{"svgblip", false, false, NO_CONTEXT, RTF_KW_svgblip }, /* abiword! */
	{"swpbdr", false, false, NO_CONTEXT, RTF_KW_swpbdr },
	{"tab", false, false, NO_CONTEXT, RTF_KW_tab },
	{"tabsnoovrlp", false, false, NO_CONTEXT, RTF_KW_tabsnoovrlp }, /* 2000 */
	{"taprtl", false, false, NO_CONTEXT, RTF_KW_taprtl }, /* 2000 */
	{"tb", false, false, NO_CONTEXT, RTF_KW_tb },
	{"tbllkbestfit", false, false, NO_CONTEXT, RTF_KW_tbllkbestfit }, /* 2002 */
	{"tbllkborder", false, false, NO_CONTEXT, RTF_KW_tbllkborder }, /* 2002 */
	{"tbllkcolor", false, false, NO_CONTEXT, RTF_KW_tbllkcolor }, /* 2002 */
	{"tbllkfont", false, false, NO_CONTEXT, RTF_KW_tbllkfont }, /* 2002 */
	{"tbllkhdrcols", false, false, NO_CONTEXT, RTF_KW_tbllkhdrcols }, /* 2002 */
	{"tbllkhdrrows", false, false, NO_CONTEXT, RTF_KW_tbllkhdrrows }, /* 2002 */
	{"tbllklastcol", false, false, NO_CONTEXT, RTF_KW_tbllklastcol }, /* 2002 */
	{"tbllklastrow", false, false, NO_CONTEXT, RTF_KW_tbllklastrow }, /* 2002 */
	{"tbllkshading", false, false, NO_CONTEXT, RTF_KW_tbllkshading }, /* 2002 */
	{"tblrsid", true, false, NO_CONTEXT, RTF_KW_tblrsid }, /* 2002 */
	{"tc", false, false, NO_CONTEXT, RTF_KW_tc },
	{"tcelld", false, false, NO_CONTEXT, RTF_KW_tcelld }, /* 97 */
	{"tcf", false, false, NO_CONTEXT, RTF_KW_tcf },
	{"tcl", false, false, NO_CONTEXT, RTF_KW_tcl },
	{"tcn", false, false, NO_CONTEXT, RTF_KW_tcn },
	{"tdfrmtxtBottom", true, false, NO_CONTEXT, RTF_KW_tdfrmtxtBottom }, /* 2000 */
	{"tdfrmtxtLeft", true, false, NO_CONTEXT, RTF_KW_tdfrmtxtLeft }, /* 2000 */
	{"tdfrmtxtRight", true, false, NO_CONTEXT, RTF_KW_tdfrmtxtRight }, /* 2000 */
	{"tdfrmtxtTop", true, false, NO_CONTEXT, RTF_KW_tdfrmtxtTop }, /* 2000 */
	{"template", false, false, NO_CONTEXT, RTF_KW_template },
	{"time", false, false, NO_CONTEXT, RTF_KW_time }, /* 97 */
	{"title", false, false, NO_CONTEXT, RTF_KW_title },
	{"titlepg", false, false, NO_CONTEXT, RTF_KW_titlepg },
	{"tldot", false, false, NO_CONTEXT, RTF_KW_tldot },
	{"tleq", false, false, NO_CONTEXT, RTF_KW_tleq },
	{"tlhyph", false, false, NO_CONTEXT, RTF_KW_tlhyph },
	{"tlmdot", false, false, NO_CONTEXT, RTF_KW_tlmdot }, /* 7.0 */
	{"tlth", false, false, NO_CONTEXT, RTF_KW_tlth },
	{"tlul", false, false, NO_CONTEXT, RTF_KW_tlul },
	{"topline", false, false, NO_CONTEXT, RTF_KW_topline },
	{"toplinepunct", false, false, NO_CONTEXT, RTF_KW_toplinepunct }, /* 2002 */
	{"tphcol", false, false, NO_CONTEXT, RTF_KW_tphcol }, /* 2000 */
	{"tphmrg", false, false, NO_CONTEXT, RTF_KW_tphmrg }, /* 2000 */
	{"tphpg", false, false, NO_CONTEXT, RTF_KW_tphpg }, /* 2000 */
	{"tposnegx", true, false, NO_CONTEXT, RTF_KW_tposnegx }, /* 2000 */
	{"tposnegy", true, false, NO_CONTEXT, RTF_KW_tposnegy }, /* 2000 */
	{"tposxc", false, false, NO_CONTEXT, RTF_KW_tposxc }, /* 2000 */
	{"tposxi", false, false, NO_CONTEXT, RTF_KW_tposxi }, /* 2000 */
	{"tposxl", false, false, NO_CONTEXT, RTF_KW_tposxl }, /* 2000 */
	{"tposx", true, false, NO_CONTEXT, RTF_KW_tposx }, /* 2000 */
	{"tposxo", false, false, NO_CONTEXT, RTF_KW_tposxo }, /* 2000 */
	{"tposxr", false, false, NO_CONTEXT, RTF_KW_tposxr }, /* 2000 */
	{"tposy", false, false, NO_CONTEXT, RTF_KW_tposy }, /* 2000 */
	{"tposyb", false, false, NO_CONTEXT, RTF_KW_tposyb }, /* 2000 */
	{"tposyc", false, false, NO_CONTEXT, RTF_KW_tposyc }, /* 2000 */
	{"tposyil", false, false, NO_CONTEXT, RTF_KW_tposyil }, /* 2000 */
	{"tposyin", false, false, NO_CONTEXT, RTF_KW_tposyin }, /* 2000 */
	{"tposyoutv", false, false, NO_CONTEXT, RTF_KW_tposyoutv }, /* 2000 */
	{"tposyt", false, false, NO_CONTEXT, RTF_KW_tposyt }, /* 2000 */
	{"tpvmrg", false, false, NO_CONTEXT, RTF_KW_tpvmrg }, /* 2000 */
	{"tpvpara", false, false, NO_CONTEXT, RTF_KW_tpvpara }, /* 2000 */
	{"tpvpg", false, false, NO_CONTEXT, RTF_KW_tpvpg }, /* 2000 */
	{"tqc", false, false, NO_CONTEXT, RTF_KW_tqc },
	{"tqdec", false, false, NO_CONTEXT, RTF_KW_tqdec },
	{"tqr", false, false, NO_CONTEXT, RTF_KW_tqr },
	{"transmf", false, false, NO_CONTEXT, RTF_KW_transmf },
	{"trauth", true, false, NO_CONTEXT, RTF_KW_trauth }, /* 2002 */
	{"trautofit", true, false, NO_CONTEXT, RTF_KW_trautofit }, /* 2000 */
	{"trbgbdiag", false, false, NO_CONTEXT, RTF_KW_trbgbdiag }, /* 2002 */
	{"trbgcross", false, false, NO_CONTEXT, RTF_KW_trbgcross }, /* 2002 */
	{"trbgdcross", false, false, NO_CONTEXT, RTF_KW_trbgdcross }, /* 2002 */
	{"trbgdkbdiag", false, false, NO_CONTEXT, RTF_KW_trbgdkbdiag }, /* 2002 */
	{"trbgdkcross", false, false, NO_CONTEXT, RTF_KW_trbgdkcross }, /* 2002 */
	{"trbgdkdcross", false, false, NO_CONTEXT, RTF_KW_trbgdkdcross }, /* 2002 */
	{"trbgdkfdiag", false, false, NO_CONTEXT, RTF_KW_trbgdkfdiag }, /* 2002 */
	{"trbgdkhor", false, false, NO_CONTEXT, RTF_KW_trbgdkhor }, /* 2002 */
	{"trbgdkvert", false, false, NO_CONTEXT, RTF_KW_trbgdkvert }, /* 2002 */
	{"trbgfdiag", false, false, NO_CONTEXT, RTF_KW_trbgfdiag }, /* 2002 */
	{"trbghoriz", false, false, NO_CONTEXT, RTF_KW_trbghoriz }, /* 2002 */
	{"trbgvert", false, false, NO_CONTEXT, RTF_KW_trbgvert }, /* 2002 */
	{"trbrdrb", false, false, NO_CONTEXT, RTF_KW_trbrdrb },
	{"trbrdrh", false, false, NO_CONTEXT, RTF_KW_trbrdrh },
	{"trbrdrl", false, false, NO_CONTEXT, RTF_KW_trbrdrl },
	{"trbrdrr", false, false, NO_CONTEXT, RTF_KW_trbrdrr },
	{"trbrdrt", false, false, NO_CONTEXT, RTF_KW_trbrdrt },
	{"trbrdrv", false, false, NO_CONTEXT, RTF_KW_trbrdrv },
	{"trcbpat", true, false, NO_CONTEXT, RTF_KW_trcbpat }, /* 2002 */
	{"trcfpat", true, false, NO_CONTEXT, RTF_KW_trcfpat }, /* 2002 */
	{"trdate", true, false, NO_CONTEXT, RTF_KW_trdate },
	{"trftsWidthA", true, false, NO_CONTEXT, RTF_KW_trftsWidthA }, /* 2000 */
	{"trftsWidthB", true, false, NO_CONTEXT, RTF_KW_trftsWidthB }, /* 2000 */
	{"trftsWidth", true, false, NO_CONTEXT, RTF_KW_trftsWidth }, /* 2000 */
	{"trgaph", false, false, NO_CONTEXT, RTF_KW_trgaph },
	{"trhdr", false, false, NO_CONTEXT, RTF_KW_trhdr },
	{"trkeep", false, false, NO_CONTEXT, RTF_KW_trkeep },
	{"trleft", false, false, NO_CONTEXT, RTF_KW_trleft },
	{"trowd", false, false, NO_CONTEXT, RTF_KW_trowd },
	{"trpaddb", true, false, NO_CONTEXT, RTF_KW_trpaddb }, /* 2000 */
	{"trpaddfb", true, false, NO_CONTEXT, RTF_KW_trpaddfb }, /* 2000 */
	{"trpaddfl", true, false, NO_CONTEXT, RTF_KW_trpaddfl }, /* 2000 */
	{"trpaddfr", true, false, NO_CONTEXT, RTF_KW_trpaddfr }, /* 2000 */
	{"trpaddft", true, false, NO_CONTEXT, RTF_KW_trpaddft }, /* 2000 */
	{"trpaddl", true, false, NO_CONTEXT, RTF_KW_trpaddl }, /* 2000 */
	{"trpaddr", true, false, NO_CONTEXT, RTF_KW_trpaddr }, /* 2000 */
	{"trpaddt", true, false, NO_CONTEXT, RTF_KW_trpaddt }, /* 2000 */
	{"trpat", true, false, NO_CONTEXT, RTF_KW_trpat }, /* 2002 */
	{"trqc", false, false, NO_CONTEXT, RTF_KW_trqc },
	{"trql", false, false, NO_CONTEXT, RTF_KW_trql },
	{"trqr", false, false, NO_CONTEXT, RTF_KW_trqr },
	{"trrh", false, false, NO_CONTEXT, RTF_KW_trrh },
	{"trshdng", true, false, NO_CONTEXT, RTF_KW_trshdng }, /* 2002 */
	{"trspdb", true, false, NO_CONTEXT, RTF_KW_trspdb }, /* 2000 */
	{"trspdfb", true, false, NO_CONTEXT, RTF_KW_trspdfb }, /* 2000 */
	{"trspdfl", true, false, NO_CONTEXT, RTF_KW_trspdfl }, /* 2000 */
	{"trspdfr", true, false, NO_CONTEXT, RTF_KW_trspdfr }, /* 2000 */
	{"trspdft", true, false, NO_CONTEXT, RTF_KW_trspdft }, /* 2000 */
	{"trspdl", true, false, NO_CONTEXT, RTF_KW_trspdl }, /* 2000 */
	{"trspdr", true, false, NO_CONTEXT, RTF_KW_trspdr }, /* 2000 */
	{"trspdt", true, false, NO_CONTEXT, RTF_KW_trspdt }, /* 2000 */
	{"truncatefontheight", false, false, NO_CONTEXT, RTF_KW_truncatefontheight },
	{"trwWidthA", true, false, NO_CONTEXT, RTF_KW_trwWidthA }, /* 2000 */
	{"trwWidthB", true, false, NO_CONTEXT, RTF_KW_trwWidthB }, /* 2000 */
	{"trwWidth", true, false, NO_CONTEXT, RTF_KW_trwWidth }, /* 2000 */
	{"ts", false, false, NO_CONTEXT, RTF_KW_ts }, /* 2002 */
	{"tsbgbdiag", false, false, NO_CONTEXT, RTF_KW_tsbgbdiag }, /* 2002 */
	{"tsbgcross", false, false, NO_CONTEXT, RTF_KW_tsbgcross }, /* 2002 */
	{"tsbgdcross", false, false, NO_CONTEXT, RTF_KW_tsbgdcross }, /* 2002 */
	{"tsbgdkbdiag", false, false, NO_CONTEXT, RTF_KW_tsbgdkbdiag }, /* 2002 */
	{"tsbgdkcross", false, false, NO_CONTEXT, RTF_KW_tsbgdkcross }, /* 2002 */
	{"tsbgdkdcross", false, false, NO_CONTEXT, RTF_KW_tsbgdkdcross }, /* 2002 */
	{"tsbgdkfdiag", false, false, NO_CONTEXT, RTF_KW_tsbgdkfdiag }, /* 2002 */
	{"tsbgdkhor", false, false, NO_CONTEXT, RTF_KW_tsbgdkhor }, /* 2002 */
	{"tsbgdkvert", false, false, NO_CONTEXT, RTF_KW_tsbgdkvert }, /* 2002 */
	{"tsbgfdiag", false, false, NO_CONTEXT, RTF_KW_tsbgfdiag }, /* 2002 */
	{"tsbghoriz", false, false, NO_CONTEXT, RTF_KW_tsbghoriz }, /* 2002 */
	{"tsbgvert", false, false, NO_CONTEXT, RTF_KW_tsbgvert }, /* 2002 */
	{"tsbrdrb", false, false, NO_CONTEXT, RTF_KW_tsbrdrb }, /* 2002 */
	{"tsbrdrdgl", false, false, NO_CONTEXT, RTF_KW_tsbrdrdgl }, /* 2002 */
	{"tsbrdrdgr", false, false, NO_CONTEXT, RTF_KW_tsbrdrdgr }, /* 2002 */
	{"tsbrdrh", false, false, NO_CONTEXT, RTF_KW_tsbrdrh }, /* 2002 */
	{"tsbrdrl", false, false, NO_CONTEXT, RTF_KW_tsbrdrl }, /* 2002 */
	{"tsbrdrr", false, false, NO_CONTEXT, RTF_KW_tsbrdrr }, /* 2002 */
	{"tsbrdrt", false, false, NO_CONTEXT, RTF_KW_tsbrdrt }, /* 2002 */
	{"tsbrdrv", false, false, NO_CONTEXT, RTF_KW_tsbrdrv }, /* 2002 */
	{"tscbandhorzeven", false, false, NO_CONTEXT, RTF_KW_tscbandhorzeven }, /* 2002 */
	{"tscbandhorzodd", false, false, NO_CONTEXT, RTF_KW_tscbandhorzodd }, /* 2002 */
	{"tscbandsh", false, false, NO_CONTEXT, RTF_KW_tscbandsh }, /* 2002 */
	{"tscbandsv", false, false, NO_CONTEXT, RTF_KW_tscbandsv }, /* 2002 */
	{"tscbandverteven", false, false, NO_CONTEXT, RTF_KW_tscbandverteven }, /* 2002 */
	{"tscbandvertodd", false, false, NO_CONTEXT, RTF_KW_tscbandvertodd }, /* 2002 */
	{"tscellcbpat", true, false, NO_CONTEXT, RTF_KW_tscellcbpat }, /* 2002 */
	{"tscellcfpat", true, false, NO_CONTEXT, RTF_KW_tscellcfpat }, /* 2002 */
	{"tscellpaddb", true, false, NO_CONTEXT, RTF_KW_tscellpaddb }, /* 2002 */
	{"tscellpaddfb", true, false, NO_CONTEXT, RTF_KW_tscellpaddfb }, /* 2002 */
	{"tscellpaddfl", true, false, NO_CONTEXT, RTF_KW_tscellpaddfl }, /* 2002 */
	{"tscellpaddfr", true, false, NO_CONTEXT, RTF_KW_tscellpaddfr }, /* 2002 */
	{"tscellpaddft", true, false, NO_CONTEXT, RTF_KW_tscellpaddft }, /* 2002 */
	{"tscellpaddl", true, false, NO_CONTEXT, RTF_KW_tscellpaddl }, /* 2002 */
	{"tscellpaddr", true, false, NO_CONTEXT, RTF_KW_tscellpaddr }, /* 2002 */
	{"tscellpaddt", true, false, NO_CONTEXT, RTF_KW_tscellpaddt }, /* 2002 */
	{"tscellpct", true, false, NO_CONTEXT, RTF_KW_tscellpct }, /* 2002 */
	{"tscellwidth", false, false, NO_CONTEXT, RTF_KW_tscellwidth }, /* 2002 */
	{"tscellwidthfts", false, false, NO_CONTEXT, RTF_KW_tscellwidthfts }, /* 2002 */
	{"tscfirstcol", false, false, NO_CONTEXT, RTF_KW_tscfirstcol }, /* 2002 */
	{"tscfirstrow", false, false, NO_CONTEXT, RTF_KW_tscfirstrow }, /* 2002 */
	{"tsclastcol", false, false, NO_CONTEXT, RTF_KW_tsclastcol }, /* 2002 */
	{"tsclastrow", false, false, NO_CONTEXT, RTF_KW_tsclastrow }, /* 2002 */
	{"tscnecell", false, false, NO_CONTEXT, RTF_KW_tscnecell }, /* 2002 */
	{"tscnwcell", false, false, NO_CONTEXT, RTF_KW_tscnwcell }, /* 2002 */
	{"tscsecell", false, false, NO_CONTEXT, RTF_KW_tscsecell }, /* 2002 */
	{"tscswcell", false, false, NO_CONTEXT, RTF_KW_tscswcell }, /* 2002 */
	{"tsd", false, false, NO_CONTEXT, RTF_KW_tsd }, /* 2002 */
	{"tsnowrap", false, false, NO_CONTEXT, RTF_KW_tsnowrap }, /* 2002 */
	{"tsrowd", false, false, NO_CONTEXT, RTF_KW_tsrowd }, /* 2002 */
	{"tsvertalb", false, false, NO_CONTEXT, RTF_KW_tsvertalb }, /* 2002 */
	{"tsvertalc", false, false, NO_CONTEXT, RTF_KW_tsvertalc }, /* 2002 */
	{"tsvertalt", false, false, NO_CONTEXT, RTF_KW_tsvertalt }, /* 2002 */
	{"twoonone", false, false, NO_CONTEXT, RTF_KW_twoonone }, /* 7.0 */
	{"tx", false, false, NO_CONTEXT, RTF_KW_tx },
	{"txe", false, false, NO_CONTEXT, RTF_KW_txe },
	{"uc", true, false, NO_CONTEXT, RTF_KW_uc }, /* 97 */
	{"ud", false, false, NO_CONTEXT, RTF_KW_ud }, /* 97 */
	{"ul", false, false, NO_CONTEXT, RTF_KW_ul },
	{"ulc", true, false, NO_CONTEXT, RTF_KW_ulc }, /* 2000 */
	{"uld", false, false, NO_CONTEXT, RTF_KW_uld },
	{"uldash", false, false, NO_CONTEXT, RTF_KW_uldash }, /* 7.0 */
	{"uldashd", false, false, NO_CONTEXT, RTF_KW_uldashd }, /* 7.0 */
	{"uldashdd", false, false, NO_CONTEXT, RTF_KW_uldashdd }, /* 7.0 */
	{"uldb", false, false, NO_CONTEXT, RTF_KW_uldb },
	{"ulhair", false, false, NO_CONTEXT, RTF_KW_ulhair }, /* 7.0 */
	{"ulhwave", false, false, NO_CONTEXT, RTF_KW_ulhwave }, /* 2000 */
	{"ulldash", false, false, NO_CONTEXT, RTF_KW_ulldash }, /* 2000 */
	{"ulnone", false, false, NO_CONTEXT, RTF_KW_ulnone },
	{"ulth", false, false, NO_CONTEXT, RTF_KW_ulth }, /* 7.0 */
	{"ulthd", false, false, NO_CONTEXT, RTF_KW_ulthd }, /* 2000 */
	{"ulthdash", false, false, NO_CONTEXT, RTF_KW_ulthdash }, /* 2000 */
	{"ulthdashd", false, false, NO_CONTEXT, RTF_KW_ulthdashd }, /* 2000 */
	{"ulthdashdd", false, false, NO_CONTEXT, RTF_KW_ulthdashdd }, /* 2000 */
	{"ulthldash", false, false, NO_CONTEXT, RTF_KW_ulthldash }, /* 2000 */
	{"ululdbwave", false, false, NO_CONTEXT, RTF_KW_ululdbwave }, /* 2000 */
	{"ulw", false, false, NO_CONTEXT, RTF_KW_ulw },
	{"ulwave", false, false, NO_CONTEXT, RTF_KW_ulwave }, /* 7.0 */
	{"u", true, false, NO_CONTEXT, RTF_KW_u }, /* 97 */
	{"up", false, false, NO_CONTEXT, RTF_KW_up },
	{"upr", false, false, NO_CONTEXT, RTF_KW_upr }, /* 97 */
	{"urtf", true, false, NO_CONTEXT, RTF_KW_urtf },
	{"useltbaln", false, false, NO_CONTEXT, RTF_KW_useltbaln }, /* 2000 */
	{"userprops", false, false, NO_CONTEXT, RTF_KW_userprops }, /* 7.0 */
	{"v", false, false, NO_CONTEXT, RTF_KW_v },
	{"vern", false, false, NO_CONTEXT, RTF_KW_vern },
	{"version", false, false, NO_CONTEXT, RTF_KW_version },
	{"vertalb", false, false, NO_CONTEXT, RTF_KW_vertalb },
	{"vertalc", false, false, NO_CONTEXT, RTF_KW_vertalc },
	{"vertalj", false, false, NO_CONTEXT, RTF_KW_vertalj },
	{"vertalt", false, false, NO_CONTEXT, RTF_KW_vertalt },
	{"vertdoc", false, false, NO_CONTEXT, RTF_KW_vertdoc }, /* 7.0 */
	{"vertsect", false, false, NO_CONTEXT, RTF_KW_vertsect }, /* 7.0 */
	{"viewkind", true, false, NO_CONTEXT, RTF_KW_viewkind }, /* 97 */
	{"viewnobound", false, false, NO_CONTEXT, RTF_KW_viewnobound }, /* 2002 */
	{"viewscale", true, false, NO_CONTEXT, RTF_KW_viewscale }, /* 97 */
	{"viewzk", true, false, NO_CONTEXT, RTF_KW_viewzk }, /* 97 */
	{"wbitmap", false, false, NO_CONTEXT, RTF_KW_wbitmap },
	{"wbmbitspixel", false, false, NO_CONTEXT, RTF_KW_wbmbitspixel },
	{"wbmplanes", false, false, NO_CONTEXT, RTF_KW_wbmplanes },
	{"wbmwidthbytes", false, false, NO_CONTEXT, RTF_KW_wbmwidthbytes },
	{"webhidden", false, false, NO_CONTEXT, RTF_KW_webhidden }, /* 2000 */
	{"widctlpar", false, false, NO_CONTEXT, RTF_KW_widctlpar },
	{"widowctrl", false, false, NO_CONTEXT, RTF_KW_widowctrl },
	{"windowcaption", false, false, NO_CONTEXT, RTF_KW_windowcaption }, /* 97 */
	{"wmetafile", false, false, NO_CONTEXT, RTF_KW_wmetafile },
	{"wpeqn", false, false, NO_CONTEXT, RTF_KW_wpeqn }, /* 97 */
	{"wpjst", false, false, NO_CONTEXT, RTF_KW_wpjst }, /* 97 */
	{"wpsp", false, false, NO_CONTEXT, RTF_KW_wpsp }, /* 97 */
	{"wraptrsp", false, false, NO_CONTEXT, RTF_KW_wraptrsp },
	{"wrppunct", false, false, NO_CONTEXT, RTF_KW_wrppunct }, /* 2002 */
	{"xe", false, false, NO_CONTEXT, RTF_KW_xe },
	{"xef", false, false, NO_CONTEXT, RTF_KW_xef },
	{"yr", false, false, NO_CONTEXT, RTF_KW_yr },
	{"yts", true, false, NO_CONTEXT, RTF_KW_yts }, /* 2002 */
	{"yxe", false, false, NO_CONTEXT, RTF_KW_yxe }, /* 97 */
	{"zwbo", false, false, NO_CONTEXT, RTF_KW_zwbo }, /* 7.0 */
	{"zwj", false, false, NO_CONTEXT, RTF_KW_zwj }, /* 2002 */
	{"zwnbo", false, false, NO_CONTEXT, RTF_KW_zwnbo }, /* 7.0 */
	{"zwnj", false, false, NO_CONTEXT, RTF_KW_zwnj }, /* 2002 */
	{"{", false, false, NO_CONTEXT, RTF_KW_OPENCBRACE },
	{"|", false, false, NO_CONTEXT, RTF_KW_PIPE },
	{"}", false, false, NO_CONTEXT, RTF_KW_CLOSECBRACE },
	{"~", false, false, NO_CONTEXT, RTF_KW_TILDE }
};
