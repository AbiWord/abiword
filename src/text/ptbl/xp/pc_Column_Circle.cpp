
UT_Bool PC_Column_Circle::setAttributes(const XML_Char ** attrs)
{
	const char * pszLeft = UT_getAttribute("left", attrs);
	const char * pszTop = UT_getAttribute("top", attrs);
	const char * pszRadius = UT_getAttribute("radius", attrs);

	if (!pszLeft || !pszTop || !pszRadius)
		return UT_FALSE;
	
	// TODO validate these strings as dimensioned numbers.
	// TODO left,top should be numbers.
	// TODO radius should be a number or '*'.

	UT_cloneString(m_szLeft,pszLeft);
	UT_cloneString(m_szTop,pszTop);
	UT_cloneString(m_szRadius,pszRadius);

	if (!m_szLeft || !m_szTop || !m_szRadius)
		return UT_FALSE;

	return UT_TRUE;
}
