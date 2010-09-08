<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"  version="1.0"
  xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0" 
  xmlns:style="urn:oasis:names:tc:opendocument:xmlns:style:1.0" 
  xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0" 
  xmlns:table="urn:oasis:names:tc:opendocument:xmlns:table:1.0" 
  xmlns:draw="urn:oasis:names:tc:opendocument:xmlns:drawing:1.0" 
  xmlns:fo="urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0" 
  xmlns:xlink="http://www.w3.org/1999/xlink" 
  xmlns:dc="http://purl.org/dc/elements/1.1/" 
  xmlns:meta="urn:oasis:names:tc:opendocument:xmlns:meta:1.0" 
  xmlns:number="urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0" 
  xmlns:svg="urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0" 
  xmlns:chart="urn:oasis:names:tc:opendocument:xmlns:chart:1.0" 
  xmlns:dr3d="urn:oasis:names:tc:opendocument:xmlns:dr3d:1.0" 
  xmlns:math="http://www.w3.org/1998/Math/MathML" 
  xmlns:form="urn:oasis:names:tc:opendocument:xmlns:form:1.0" 
  xmlns:script="urn:oasis:names:tc:opendocument:xmlns:script:1.0" 
  xmlns:ooo="http://openoffice.org/2004/office" 
  xmlns:ooow="http://openoffice.org/2004/writer" 
  xmlns:oooc="http://openoffice.org/2004/calc" 
  xmlns:dom="http://www.w3.org/2001/xml-events" 
  xmlns:xforms="http://www.w3.org/2002/xforms" 
  xmlns:xsd="http://www.w3.org/2001/XMLSchema" 
  xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
  xmlns:delta="http://www.deltaxml.com/ns/track-changes/delta-namespace"
  xmlns:ac="http://www.deltaxml.com/ns/track-changes/attribute-change-namespace"
  xmlns:split="http://www.deltaxml.com/ns/track-changes/split-namespace"
  >
  <xsl:output method="xml"/>

  <xsl:variable name="newline">
  <xsl:text>
</xsl:text>
  </xsl:variable>

<!--
   <xsl:variable name="normal"> \033[0m </xsl:variable>
   <xsl:variable name="bold"> \033[1m </xsl:variable>
-->

  <xsl:variable name="normal"></xsl:variable>
  <xsl:variable name="bold"></xsl:variable>

  <xsl:template match="/">
      <xsl:apply-templates select="//office:text"/>
  </xsl:template>

  <xsl:template match="office:text">
      <office:text>
      <xsl:value-of select="$newline"/>
      <xsl:apply-templates select="./text:section/*"/>
      </office:text>
  </xsl:template>

    <xsl:template match="delta:removed-content">
      <xsl:value-of select="$bold"/>
      <removed-content delta:removal-change-idref="{@delta:removal-change-idref}">
      <xsl:value-of select="$normal"/>
      <xsl:value-of select="$newline"/>
      <xsl:apply-templates select="./*"/>
      </removed-content>
      <xsl:value-of select="$bold"/>
      <xsl:value-of select="$newline"/>
      <xsl:value-of select="$normal"/>
    </xsl:template>

    <xsl:template match="text:p">
      <p delta:insertion-change-idref="{@delta:insertion-change-idref}">
      <xsl:value-of select="$newline"/>
      <xsl:value-of select="."/>
      <xsl:apply-templates select="*"/>
      </p>
      <xsl:value-of select="$newline"/>

    </xsl:template>
  
  <xsl:template match="Folder">
      <context name="{./name}">
        <xsl:apply-templates select="./Folder"/>
        <xsl:apply-templates select="./Placemark"/>
      </context>

  </xsl:template>

  <xsl:template match="Placemark">
    <context name="{./name}" longitude="{./LookAt/longitude}" latitude="{./LookAt/latitude}" zoom="{./LookAt/range}" />
  </xsl:template>

  <xsl:template match="*"></xsl:template>

</xsl:stylesheet>
