
UT_Bool PC_Column_Box::setAttributes(const XML_Char ** attrs)
{
	const char * pszLeft = UT_getAttribute("left", attrs);
	const char * pszTop = UT_getAttribute("top", attrs);
	const char * pszWidth = UT_getAttribute("width", attrs);
	const char * pszHeight = UT_getAttribute("height", attrs);

	if (!pszLeft || !pszTop || !pszWidth || !pszHeight)
		return UT_FALSE;
	
	// TODO validate these strings as dimensioned numbers.
	// TODO left,top should be numbers.
	// TODO width,height should be numbers or '*'.

	UT_cloneString(m_szLeft,pszLeft);
	UT_cloneString(m_szTop,pszTop);
	UT_cloneString(m_szWidth,pszWidth);
	UT_cloneString(m_szHeight,pszHeight);

	if (!m_szLeft || !m_szTop || !m_szWidth || !m_szHeight)
		return UT_FALSE;

	return UT_TRUE;
}
