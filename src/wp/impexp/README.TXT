/*! \page impexp_overview ImpExp

\todo Finish it.

<h1>1. Generalities</h1>

This part contains all the importer and exporter code used by AbiWord.
IE_Imp_* classes are the document importers. IE_Exp_* classes are exporters. 
IE_ImpGraphic_* classes are graphics importers.
<p>
Importers and exporters are also used for clipboard operations.

<h1>2. Importers</h1>

<ul>

<li>IE_Imp -- This is the base class for all WP importers.

<li>IE_Imp_AbiWord_1 -- Imports version 1 (ie current) of AbiWord documents.

<li>IE_Imp_Applix -- This is the importer for Applix Words documents.

<li>IE_Imp_DocBook -- Importer for DocBook SGML documents.

<li>IE_Imp_GraphicAsDocument -- Import a graphic as an empty document 
containing that graphics. Use available IE_ImpGraphic_*

<li>IE_Imp_GZipAbiWord -- Imports gzip compressed AbiWord documents (.zabw)

<li>IE_Imp_MsWord_97 -- Imports MS Word 97 documents using libwv.

<li>IE_Imp_RTF -- This is the RTF importer.

<li>IE_Imp_Text -- Plain text importer. Also handle non-ASCII text.

<li>IE_Imp_WordPerfect -- Imports WordPerfect documents.

<li>IE_Imp_XHTML -- Import valid XHTML documents.

<li>IE_Imp_XML -- Generic XML importer. Used as a base class for all other 
XML work.

</ul>

<h1>3. Graphic Importers</h1>

<ul>

<li>IE_ImpGraphic -- This is the base class for all graphics
importers.

<li>IE_ImpGraphic_JPEG -- This is the JPEG importer using
jpeglib. Convert JPEG image to a PNG image.

<li>IE_ImpGraphic_PNG -- This is the PNG importer. Simply reads the PNG file.

<li>IE_ImpGraphic_BMP -- This is the BMP importer. Convert a BMP file to PNG.

<li>IE_ImpGraphic_WMF -- WMF Importer.

<li>IE_ImpGraphic_SVG -- SVG Importer. Currently worthless.

</ul>

<h1>4. Exporters</h1>

<ul>

<li>IE_Exp_AbiWord_1 -- Write AbiWord XML files version 1, the native file 
format as of today.

<li>IE_Exp_AWT -- Write AbiWord template documents. Most of the functionnality 
is inherited from IE_Exp_AbiWord_1

<li>IE_Exp_HTML -- Output HTML 4.0 or XHTML.

<li>IE_Exp_RTF -- Exports RTF.

<li>IE_Exp_Text -- Exports plain text.

</ul>

*/
