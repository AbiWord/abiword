/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 2002 Francis James Franklin <fjf@alinameridon.com>
 * Copyright (C) 2001-2002 AbiSource, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

/* Note: These defs originated in the HTML import plugin and the
 *       comments sometimes reflect this.
 */

#ifndef IE_IMPEXP_HTML_H
#define IE_IMPEXP_HTML_H

#define IE_MIMETYPE_XHTML			"application/xhtml+xml"
#define IE_MIMETYPE_HTML			"text/html"
#define IE_MIMETYPE_CSS				"text/css"

#define TT_OTHER	0	// ?		Tag not recognized (not an error, though)

#define TT_HTML		1	// <html>	Document main/first tag

#define TT_HEAD		2	// <head>	(child of <html>; 1 instance; precedes <body>)
#define TT_BODY		3	// <body>	(child of <html>; 1 instance; follows <head>)

#define TT_TITLE	4	// <title>	(child of <head>)
#define TT_STYLE	5	// <style>	(child of <head>)
#define TT_DIV		6	// <div>	[B] Used with style sheets to produce layers, boxes, etc.
				//		AbiWord uses these in its XHTML output to denote sections
				//		but this seems an uncommon use.
#define TT_SPAN		7	// <span>	Like <div>, this is used in conjuction with style sheets
#define TT_LAYER	8	// <layer>	Layer

#define TT_LINK		9	// <link>	(child of <head>; no children; no end tag)
#define TT_BASE		10	// <base>	(child of <head>; no children; no end tag)
#define TT_META		11	// <meta>	(child of <head>; no children; no end tag)

#define TT_A		21	// <a>		Anchor or Hyperlink (can't nest links)

#define TT_H1		22	// <h1>		Headings
#define TT_H2		23	// <h2>
#define TT_H3		24	// <h3>
#define TT_H4		25	// <h4>		Normal size but bold, others relative (usually) (?)
#define TT_H5		26	// <h5>
#define TT_H6		27	// <h6>

#define TT_ADDRESS	29	// <address>	Author's address { set italics }

/* Structured Text: Phrase elements
 */
#define TT_EM		40	// <em>		Emphasis { toggle italics }
#define TT_STRONG	41	// <strong>	Stronger emphasis { set bold }
#define TT_DFN		42	// <dfn>	Defining instance { ? }
#define TT_CODE		43	// <code>	Computer code { use fixed width font }
#define TT_SAMP		44	// <samp>	Sample output { use fixed width font }
#define TT_KBD		45	// <kbd>	Text to be entered by user { use fixed width font }
#define TT_VAR		46	// <var>	Variable/Argument { use fixed width font }
#define TT_CITE		47	// <cite>	Citation/Reference { no action }
#define TT_ABBR		48	// <abbr>	Abbreviation { no action }
#define TT_ACRONYM	49	// <acronym>	Acronym { no action }

/* Structured Text: Quotations
 */
#define TT_Q		50	// <q>		Quote { quotation mark e.g. `, ', ", <<, - a job for smart quotes?  }
#define TT_BLOCKQUOTE	51	// <blockquote>	[B] Block quote { separate paragraph; indented }

/* Structured Text: Subscripts and superscripts
 */
#define TT_SUB		52	// <sub>	Subscript
#define TT_SUP		53	// <sup>	Superscript

/* Lines And Paragraphs: Paragraphs
 */
#define TT_P		54	// <p>		[B] New paragraph
				//		Can't contain other block elements, inc. self
				//		(Ignore empty paragraphs.)

/* Lines And Paragraphs: Controlling line breaks
 */
#define TT_BR		55	// <br>		Forced line break

/* Lines And Paragraphs: Preformatted text
 */
#define TT_PRE		56	// <pre>	Preformatted text { use fixed width font; white space critical }
				//		Can't contain: <img>, <object>, <big>, <small>, <sub>, <sup>

/* Lines And Paragraphs: Marking document changes
 */
#define TT_INS		57	// <ins>	Insert { set color red }
#define TT_DEL		58	// <del>	Delete { set color red; set strike-through }

/* Lists: Unordered lists, ordered lists, and list items
 */
#define TT_OL		59	// <ol>		Ordered list (children must be <li>)
#define TT_UL		60	// <ul>		Unordered list (children must be <li>)
#define TT_LI		61	// <li>		List item (</li> optional)

/* Lists: Definition lists
 */
#define TT_DL		62	// <dl>		Definition list
#define TT_DT		63	// <dt>		Definition term { set bold }
#define TT_DD		64	// <dd>		Definition description { indent }

/* Lists: <dir> & <menu>
 */
#define TT_DIR		TT_UL	// <dir>	Directory list { cf. <ul> }
#define TT_MENU		TT_UL	// <menu>	Menu list { cf. <ul> }

/* Tables:
 */
#define TT_TABLE	65	// <table>	Table
#define TT_CAPTION	66	// <caption>	Caption (child of <table>)
#define TT_THEAD	67	// <thead>	Table Head (child of <table> acting as new <table>)
#define TT_TFOOT	68	// <tfoot>	Table Foot (child of <table> acting as new <table>)
#define TT_TBODY	69	// <tbody>	Table Body (child of <table> acting as new <table>)
#define TT_COLGROUP	70	// <colgroup>	Un-partition existing columns into one
#define TT_COL		71	// <col>	Subdivide grouped column (child of <colgroup>)
#define TT_TR		72	// <tr>		Row (child of <table>)
#define TT_TH		73	// <th>		Header { set bold } (child of <tr>)
#define TT_TD		74	// <td>		Data (child of <tr>)

/* Objects, Images & Applets: Including an image
 */
#define TT_IMG		75	// <img>	Image tag (empty; no children; no end tag)

/* Objects, Images & Applets: Generic inclusion
 */
#define TT_OBJECT	76	// <object>	Object

/* Objects, Images & Applets: Object initialization
 */
#define TT_PARAM	77	// <param>	Option for object (empty; no children; no end tag)

/* Objects, Images & Applets: Including an applet
 */
#define TT_APPLET	78	// <applet>	Applet

/* Objects, Images & Applets: Client-side image maps
 */
#define TT_MAP		79	// <map>	Map
#define TT_AREA		80	// <area>	Area with map (child of <map>; empty; no children; no end tag)

/* Alignment, Font Styles etc.: Alignment
 */
#define TT_CENTER	81	// <center>	= <div align="center">

/* Alignment, Font Styles etc.: Font style elements
 */
#define TT_TT		82	// <tt>		Fixed width { set fixed width }
#define TT_I		83	// <i>		Italic { set italics }
#define TT_B		84	// <b>		Bold { set bold }
#define TT_BIG		85	// <big>	Large { inc. font size }
#define TT_SMALL	86	// <small>	Small { dec. font size }
#define TT_S		87	// <s>		Strike-through { set strike-through }
#define TT_STRIKE	TT_S	// <strike>	Strike-through { set strike-through }
#define TT_U		88	// <u>		Underline { set underline }

/* Alignment, Font Styles etc.: Font modifier elements
 */
#define TT_FONT		89	// <font>	Set new font (mother of all...)
#define TT_BASEFONT	90	// <basefont>	Default font (font sizes calculated relative to base-font)

/* Alignment, Font Styles etc.: Rules
 */
#define TT_HR		91	// <hr>		Horizontal rule (empty; no children; no end tag)

/* Frames:
 */
#define TT_FRAMESET	92	// <frameset>	Set of frames { no action }
#define TT_FRAME	93	// <frame>	A frame (child of frameset) { no action }
#define TT_NOFRAMES	94	// <noframes>	Frameless version { no action }
#define TT_IFRAME	95	// <iframe>	Inline frame { no action }

/* Forms:
 */
#define TT_FORM		96	// <form>	Form { no action }
#define TT_INPUT	97	// <input>	Input { no action }
#define TT_BUTTON	98	// <button>	Button { no action }
#define TT_SELECT	99	// <select>	Select { no action }
#define TT_OPTGROUP	100	// <optgroup>	Opt. Group { no action }
#define TT_OPTION	101	// <option>	Option { no action }
#define TT_TEXTAREA	102	// <textarea>	Text area { no action }
#define TT_ISINDEX	103	// <isindex>	Index { no action }
#define TT_LABEL	104	// <label>	Label { no action }
#define TT_FIELDSET	105	// <fieldset>	Field set { no action }
#define TT_LEGEND	106	// <legend>	Legend { no action }

/* Scripts:
 */
#define TT_SCRIPT	107	// <script>	Script { no action }
#define TT_NOSCRIPT	108	// <noscript>	Alt. to script { no action }

/* Misc:
 */
#define TT_BDO		109	// <bdo>	BiDi override

/* Ruby:
 */
#define TT_RUBY		110	// <ruby>	Ruby block
#define TT_RP		111	// <rp>		Ruby parenthesis
#define TT_RT		112	// <rt>		Ruby text

/* Math:
 */
#define TT_MATH		113 // <math>	MathML equation


#endif /* IE_IMPEXP_HTML_H */
