<?xml version="1.0" encoding="ISO-8859-1"?>
  <xsl:stylesheet version="1.0" 
		xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:output indent="yes" method="html" omit-xml-declaration="yes" doctype-public="-//W3C//DTD HTML 4.01 Transitional//EN" doctype-system="http://www.w3.org/TR/html4/loose.dtd"/>

<xsl:template match="matrices">

<html lang="{@xml:lang}">
  
<head>
    <meta http-equiv="Content-Language" content="{@xml:lang}"/>
    
    <title><xsl:value-of select="metainfo/title"/></title>

    <meta name="description" content="{metainfo/shortdesc}"/>

    <xsl:if test="count(//section) != 1">
      <xsl:for-each select="//section">
        <link rel="Bookmark" href="#{generate-id(info/heading)}" title="{info/heading}"/>
      </xsl:for-each>
    </xsl:if>

    <link rel="Bookmark" href="#column-explanation" title="Column headings explanation"/>

<style type="text/css">
<xsl:text disable-output-escaping="yes">
body {
color: black;
background: white;
font-family: "Myriad Web", "Arial Unicode MS", Helvetica, Arial, Geneva, sans-serif;
padding: 1em 5%;
margin: 0;
}

p, ul     { max-width: 32em; margin-left: auto; margin-right: auto; }
p#updated { text-align: right; margin: 1em 0; }
li        { margin-left: 3em; }

h1 { text-align: center; }
dt { font-weight: bold; }

table      { margin: 0 auto 2em auto; }
td, th     { text-align: center; }
td.percent-column     { text-align: right; }
td.first-column       { text-align: left; padding-right: 1em; }
td.legend-explanation { text-align: left; padding-left: 1em; }

td.yes       { background: #73ab73; color: white; border-color: #73ab73; }
td.no        { background: #9a6262; color: white; border-color: #9a6262; }
td.partially { background: #d8d89c; color: black; border-color: #d8d89c; }
td.unknown   { background: #b38fbf; color: black; border-color: #b38fbf; }
td.buggy     { background: #bea888; color: black; border-color: #bea888; }
td.later     { background: #519b84; color: white; border-color: #519b84; }
td.na        { background: white; color: black; border-color: white; }

td :link, td :visited                     { color: white !important; background: transparent; }
td.partially :link, td.partially :visited, td.buggy :link, td.buggy :visited,
td.na :link, td.na :visited               { color: black !important; background: transparent; }

td.yes, td.no, td.partially, td.unknown, td.na, td.buggy, td.later { border-style: outset; border-width: 1px; }
</xsl:text>
</style>

</head>

<body>

    <h1><xsl:apply-templates select="metainfo/title"/></h1>

    <xsl:apply-templates select="metainfo"/>

    <p>An explanation of the <a href="#column-explanation" title="Column headings explanation">column headings</a> is available at the bottom of the document.</p>

    <hr/>

    <h2>Legend</h2>

    <table class="table" cellpadding="4" cellspacing="0">

    <colgroup/>
    <colgroup/>

    <tbody>

    <tr><td class="yes">yes</td> <td class="legend-explanation">This feature is done/working.</td></tr>
    <tr><td class="later">later</td> <td class="legend-explanation">This feature is not planned for 1.0. If you send us code, you will change our minds.</td></tr>
    <tr><td class="partially">partially</td> <td class="legend-explanation">This feature is partially done, but needs work.</td></tr>
    <tr><td class="buggy">buggy</td> <td class="legend-explanation">This feature is done, but is too bug-ridden to be usable.</td></tr>
    <tr><td class="no">no</td> <td class="legend-explanation">This feature is not implemented.</td></tr>
    <tr><td class="unknown">unknown</td> <td class="legend-explanation">The status of this feature is unknown.</td></tr>
    <tr><td class="na"><abbr title="not applicable">n/a</abbr></td> <td class="legend-explanation">Not applicable.</td></tr>

    </tbody>

    </table>

    <xsl:for-each select="section">

      <hr/>

      <xsl:apply-templates select="info"/>

      <xsl:for-each select="matrix">
        <xsl:apply-templates/>
      </xsl:for-each>

    </xsl:for-each>

    <hr/>

    <h2><a name="column-explanation">Column explanation</a></h2>

    <dl>
        <xsl:for-each select="//header/row/cell">
            <xsl:if test="text()">
                <dt><xsl:value-of select="@value"/></dt>
                <dd><xsl:apply-templates/></dd>
            </xsl:if>
        </xsl:for-each>
          <dt>Total</dt>
          <dd>The percentage of cells on the current row where this feature is completely done/working.</dd>
    </dl>

    <hr/>
    
    <p id="updated">Last updated: <xsl:value-of select="@updated"/></p>

</body>

</html>

</xsl:template>

<xsl:template match="header">
    <thead>
    <xsl:for-each select="row">
        <tr><xsl:apply-templates/><th title="The percentage of cells on the current row where this feature is completely done/working.">Total</th></tr>
    </xsl:for-each>
    </thead>
</xsl:template>


<xsl:template match="cell">
  <xsl:apply-templates/>
</xsl:template>

<xsl:template match="header/row/cell">
    <th title="{.}"><xsl:value-of select="@value"/></th>
</xsl:template>

<xsl:template match="emph">
    <em><xsl:apply-templates/></em>
</xsl:template>



<xsl:template match="metainfo">
    <xsl:apply-templates select="shortdesc | longdesc"/>
</xsl:template>

<xsl:template match="longdesc">
    <xsl:apply-templates/>
</xsl:template>

<xsl:template match="shortdesc">
    <p><xsl:apply-templates/><xsl:if test="count(//section) !=
1"><xsl:text> </xsl:text>It's divided into <xsl:value-of select="count(//section)"/> categories:</xsl:if></p>

  <xsl:if test="count(//section) != 1">
    <ul>
      <xsl:for-each select="//section">
        <li><a href="#{generate-id(info/heading)}" title="Category: {info/heading}"><xsl:apply-templates select="info/heading"/></a></li>
      </xsl:for-each>
    </ul>
  </xsl:if>

</xsl:template>

<xsl:template match="para">
    <p><xsl:apply-templates/></p>
</xsl:template>

<xsl:template match="list">
    <ul><xsl:apply-templates/></ul>
</xsl:template>

<xsl:template match="list/item">
    <li><xsl:apply-templates/></li>
</xsl:template>

<xsl:template match="section/info">
    <h2><a name="{generate-id(heading)}"><xsl:apply-templates select="heading"/></a></h2>
    <xsl:apply-templates select="description"/>
</xsl:template>

<xsl:template match="section/info/heading">
    <xsl:apply-templates/>
</xsl:template>

<xsl:template match="info/description">
    <p><xsl:apply-templates/></p>
</xsl:template>

<xsl:template match="matrix/info/heading">
    <h3><xsl:apply-templates/></h3>
</xsl:template>


<xsl:template match="matrix/body">

    <table class="table" cellpadding="4" cellspacing="0">

    <colgroup/>
    <colgroup span="{count(//header/row/cell)-1}"/>

      <xsl:apply-templates select="//header"/>
      
      <tbody>
      <xsl:for-each select="row">
        <tr>
        <xsl:for-each select="cell">

          <td>
            <xsl:if test="@value">
	    <xsl:attribute name="class">
	      <xsl:value-of select="@value"/>        
	    </xsl:attribute>
	    </xsl:if>

            <xsl:if test="position()=1">
                <xsl:attribute name="class">first-column</xsl:attribute>
	    </xsl:if>

            <xsl:choose>
              <xsl:when test="text() or bug or pow">
                <xsl:apply-templates/>
              </xsl:when>
              <xsl:otherwise>

              <xsl:choose>
                <xsl:when test="@value='na'">
                  <abbr title="not applicable">n/a</abbr>
                </xsl:when>
                <xsl:otherwise>
                  <xsl:value-of select="@value"/>
                </xsl:otherwise>
              </xsl:choose>

              </xsl:otherwise>
            </xsl:choose>
          </td>

        </xsl:for-each>

        <td class="percent-column"><xsl:number format="1" value="100 *
count(cell[@value='yes' or @value='na']) div (count(cell)-1)"/> %</td>

        </tr>
      </xsl:for-each>

      <tr>

        <td class="first-column">Total</td>

        <xsl:for-each select="row[1]/cell[position() != '1']">
        <xsl:variable name="pos" select="position()+1"/>

          <td class="percent-row">
            <xsl:number format="1" value="100 *
count(../../row/cell[position() = $pos and (@value='yes' or @value='na')]) div
count(../../row/cell[position() = $pos])"/> %
          </td>

        </xsl:for-each>

      </tr>

      </tbody>

    </table>

</xsl:template>

<xsl:template match="bug">
    <a href="http://www.abisource.com/bugzilla/show_bug.cgi?id={@id}" title="BugZilla bug report #{@id}"><xsl:value-of select="@id"/></a>
</xsl:template>

<xsl:template match="pow">
    <a href="{@uri}"><abbr title="Project of the Week">POW</abbr></a>
</xsl:template>

<xsl:template match="abbr">
    <abbr>
      <xsl:if test="@title">
        <xsl:attribute name="title"><xsl:value-of select="@title"/></xsl:attribute>
      </xsl:if>
      <xsl:apply-templates/></abbr>
</xsl:template>

</xsl:stylesheet>
