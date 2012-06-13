/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource
 * 
 * Copyright (C) 2007 Philippe Milot <PhilMilot@gmail.com>
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#ifndef _OXMLI_TYPES_H_
#define _OXMLI_TYPES_H_

// Internal includes
#include <OXML_Element.h>
#include <OXML_Section.h>

// External includes
#include <stack>
#include <vector>

typedef std::stack<OXML_SharedElement> OXMLi_ElementStack;
typedef std::stack<OXML_SharedSection> OXMLi_SectionStack;
typedef std::vector<std::string> OXMLi_ContextVector;

struct OXMLi_StartElementRequest
{
	std::string pName;
	std::map<std::string, std::string>* ppAtts;
	OXMLi_ElementStack * stck;
	OXMLi_SectionStack * sect_stck;	
	OXMLi_ContextVector * context;
	bool handled;
	bool valid;
};

struct OXMLi_EndElementRequest
{
	std::string pName;
	OXMLi_ElementStack * stck;
	OXMLi_SectionStack * sect_stck;	
	OXMLi_ContextVector * context;
	bool handled;
	bool valid;
};

struct OXMLi_CharDataRequest
{
	const gchar * buffer;
	int length;
	OXMLi_ElementStack * stck;
	OXMLi_ContextVector * context;
	bool handled;
	bool valid;
};

enum OXMLi_Keyword
{
	KEYWORD_abstractNum,
	KEYWORD_abstractNumId,
	KEYWORD_active,
	KEYWORD_activeRecord,
	KEYWORD_activeWritingStyle,
	KEYWORD_addressFieldName,
	KEYWORD_adjustLineHeightInTable,
	KEYWORD_adjustRightInd,
	KEYWORD_alias,
	KEYWORD_aliases,
	KEYWORD_alignBordersAndEdges,
	KEYWORD_alignTablesRowByRow,
	KEYWORD_allowPNG,
	KEYWORD_allowSpaceOfSameStyleInTable,
	KEYWORD_altChunk,
	KEYWORD_altChunkPr,
	KEYWORD_altName,
	KEYWORD_alwaysMergeEmptyNamespace,
	KEYWORD_alwaysShowPlaceholderText,
	KEYWORD_annotationRef,
	KEYWORD_applyBreakingRules,
	KEYWORD_attachedSchema,
	KEYWORD_attachedTemplate,
	KEYWORD_attr,
	KEYWORD_autoCaption,
	KEYWORD_autoCaptions,
	KEYWORD_autofitToFirstFixedWidthCell,
	KEYWORD_autoFormatOverride,
	KEYWORD_autoHyphenation,
	KEYWORD_autoRedefine,
	KEYWORD_autoSpaceDE,
	KEYWORD_autoSpaceDN,
	KEYWORD_autoSpaceLikeWord95,
	KEYWORD_b,
	KEYWORD_background, 
	KEYWORD_balanceSingleByteDoubleByteWidth,
	KEYWORD_bar,
	KEYWORD_basedOn,
	KEYWORD_bCs,
	KEYWORD_bdr,
	KEYWORD_behavior,
	KEYWORD_behaviors,
	KEYWORD_between,
	KEYWORD_bibliography,
	KEYWORD_bidi,
	KEYWORD_bidiVisual,
	KEYWORD_blipFill,
	KEYWORD_blockQuote,
	KEYWORD_body,
	KEYWORD_bodyDiv,
	KEYWORD_bookFoldPrinting,
	KEYWORD_bookFoldPrintingSheets,
	KEYWORD_bookFoldRevPrinting,
	KEYWORD_bookmarkEnd,
	KEYWORD_bookmarkStart,
	KEYWORD_bordersDoNotSurroundFooter,
	KEYWORD_bordersDoNotSurroundHeader,
	KEYWORD_bottom,
	KEYWORD_break,
	KEYWORD_cachedColBalance,
	KEYWORD_calcOnExit,
	KEYWORD_calendar,
	KEYWORD_cantSplit,
	KEYWORD_caps,
	KEYWORD_caption,
	KEYWORD_captions,
	KEYWORD_category,
	KEYWORD_cellDel,
	KEYWORD_cellIns,
	KEYWORD_cellMerge,
	KEYWORD_characterSpacingControl,
	KEYWORD_charset,
	KEYWORD_checkBox,
	KEYWORD_checked,
	KEYWORD_checkErrors,
	KEYWORD_citation,
	KEYWORD_clickAndTypeStyle,
	KEYWORD_clrSchemeMapping,
	KEYWORD_cnfStyle,
	KEYWORD_col,
	KEYWORD_colDelim,
	KEYWORD_cols,
	KEYWORD_color,
	KEYWORD_column,
	KEYWORD_comboBox,
	KEYWORD_comment,
	KEYWORD_commentRangeEnd,
	KEYWORD_commentRangeStart,
	KEYWORD_commentReference,
	KEYWORD_comments,
	KEYWORD_compat,
	KEYWORD_connectString,
	KEYWORD_consecutiveHyphenLimit,
	KEYWORD_contextualSpacing,
	KEYWORD_continuationSeparator,
	KEYWORD_control,
	KEYWORD_convMailMergeEsc,
	KEYWORD_cNvPicPr,
	KEYWORD_cNvPr,
	KEYWORD_cr,
	KEYWORD_cs,
	KEYWORD_customXml,
	KEYWORD_customXmlDelRangeEnd,
	KEYWORD_customXmlDelRangeStart,
	KEYWORD_customXmlInsRangeEnd,
	KEYWORD_customXmlInsRangeStart,
	KEYWORD_customXmlMoveFromRangeEnd,
	KEYWORD_customXmlMoveFromRangeStart,
	KEYWORD_customXmlMoveToRangeEnd,
	KEYWORD_customXmlMoveToRangeStart,
	KEYWORD_customXmlPr,
	KEYWORD_dataBinding,
	KEYWORD_dataSource,
	KEYWORD_dataType,
	KEYWORD_date,
	KEYWORD_dateFormat,
	KEYWORD_dayLong,
	KEYWORD_dayShort,
	KEYWORD_ddList,
	KEYWORD_decimalSymbol,
	KEYWORD_default,
	KEYWORD_defaultTableStyle,
	KEYWORD_defaultTabStop,
	KEYWORD_del,
	KEYWORD_delInstrText,
	KEYWORD_delText,
	KEYWORD_description,
	KEYWORD_destination,
	KEYWORD_dirty,
	KEYWORD_displayHangulFixedWidth,
	KEYWORD_displayBackgroundShape,
	KEYWORD_displayHorizontalDrawingGridEvery,
	KEYWORD_displayVerticalDrawingGridEvery,
	KEYWORD_div,
	KEYWORD_divBdr,
	KEYWORD_divId,
	KEYWORD_divs,
	KEYWORD_divsChild,
	KEYWORD_docDefaults,
	KEYWORD_docGrid,
	KEYWORD_docPart,
	KEYWORD_docPartBody,
	KEYWORD_docPartCategory,
	KEYWORD_docPartGallery,
	KEYWORD_docPartList,
	KEYWORD_docPartObj,
	KEYWORD_docPartPr,
	KEYWORD_docParts,
	KEYWORD_docPartUnique,
	KEYWORD_document, 
	KEYWORD_documentProtection,
	KEYWORD_documentType,
	KEYWORD_docVar,  
	KEYWORD_docVars, 
	KEYWORD_doNotAutoCompressPictures,
	KEYWORD_doNotAutofitConstrainedTables,
	KEYWORD_doNotBreakConstrainedForcedTable,
	KEYWORD_doNotBreakWrappedTables,
	KEYWORD_doNotDemarcateInvalidXml,
	KEYWORD_doNotDisplayPageBoundries,
	KEYWORD_doNotEmbedSmartTags,
	KEYWORD_doNotExpandShiftReturn,
	KEYWORD_doNotHyphenateCaps,
	KEYWORD_doNotIncludeSubdocsInStats,
	KEYWORD_doNotLeaveBackslashAlone,
	KEYWORD_doNotOrganizeInFolder,
	KEYWORD_doNotRelyOnCSS,
	KEYWORD_doNotSaveAsSingleFile,
	KEYWORD_doNotShadeFormData,
	KEYWORD_doNotSnapToGridInCell,
	KEYWORD_doNotSuppressBlankLines,
	KEYWORD_doNotSuppressIndentation,
	KEYWORD_doNotSuppressParagraphBorders,
	KEYWORD_doNotTrackFormatting,
	KEYWORD_doNotTrackMoves,
	KEYWORD_doNotUseEastAsianBreakRules,
	KEYWORD_doNotUseHTMLParagraphAutoSpacing,
	KEYWORD_doNotUseIndentAsNumberingTabStop,
	KEYWORD_doNotUseLongFileNames,
	KEYWORD_doNotUseMarginsForDrawingGridOrigin,
	KEYWORD_doNotValidateAgainstSchema,
	KEYWORD_doNotVertAlignCellWithSp,
	KEYWORD_doNotVertAlignInTxbx,
	KEYWORD_doNotWrapTextWithPunct,
	KEYWORD_drawing,
	KEYWORD_drawingGridHorizontalOrigin,
	KEYWORD_drawingGridHorizontalSpacing,
	KEYWORD_drawingGridVerticalOrigin,
	KEYWORD_drawingGridVerticalSpacing,
	KEYWORD_dropDownList,
	KEYWORD_dstrike,
	KEYWORD_dynamicAddress,
	KEYWORD_eastAsianLayout,
	KEYWORD_effect,
	KEYWORD_equation,
	KEYWORD_em,
	KEYWORD_embedBold,
	KEYWORD_embedBoldItalic,
	KEYWORD_embedItalic,
	KEYWORD_embedRegular,
	KEYWORD_embedSystemFonts,
	KEYWORD_embedTrueTypeFonts,
	KEYWORD_emboss,
	KEYWORD_enabled,
	KEYWORD_encoding,
	KEYWORD_endnote,
	KEYWORD_endnotePr,
	KEYWORD_endnoteRef,
	KEYWORD_endnoteReference,
	KEYWORD_endnotes,
	KEYWORD_entryMacro,
	KEYWORD_evenAndOddHeaders,
	KEYWORD_exitMacro,
	KEYWORD_family,
	KEYWORD_ffData,
	KEYWORD_fHdr,
	KEYWORD_fieldMapData,
	KEYWORD_fitText,
	KEYWORD_flatBorders,
	KEYWORD_fldChar,
	KEYWORD_fldData,
	KEYWORD_fldSimple,
	KEYWORD_font,
	KEYWORD_fonts,
	KEYWORD_footerReference,
	KEYWORD_footnote,
	KEYWORD_footnoteLayoutLikeWW8,
	KEYWORD_footnotePr,
	KEYWORD_footnoteRef,
	KEYWORD_footnoteReference,
	KEYWORD_footnotes,
	KEYWORD_forceUpgrade,
	KEYWORD_forgetLastTabAlignment,
	KEYWORD_format,
	KEYWORD_formProt,
	KEYWORD_formsDesign,
	KEYWORD_frame,
	KEYWORD_frameLayout,
	KEYWORD_framePr,
	KEYWORD_frameset,
	KEYWORD_framesetSplitbar,
	KEYWORD_ftr,
	KEYWORD_gallery,
	KEYWORD_glossaryDocument,
	KEYWORD_gridAfter,
	KEYWORD_gridBefore,
	KEYWORD_gridCol,
	KEYWORD_gridSpan,
	KEYWORD_group,
	KEYWORD_growAutofit,
	KEYWORD_guid,
	KEYWORD_gutterAtTop,
	KEYWORD_hdr,
	KEYWORD_hdrShapeDefaults,
	KEYWORD_headerReference,
	KEYWORD_headerSource,
	KEYWORD_helpText,
	KEYWORD_hidden,
	KEYWORD_hideGrammaticalErrors,
	KEYWORD_hideMark,
	KEYWORD_hideSpellingErrors,
	KEYWORD_highlight,
	KEYWORD_hMerge,
	KEYWORD_hps,
	KEYWORD_hpsBaseText,
	KEYWORD_hpsRaise,
	KEYWORD_hyperlink,
	KEYWORD_hyphenationZone,
	KEYWORD_i,
	KEYWORD_iCs,
	KEYWORD_id,
	KEYWORD_ignoreMixedContent,
	KEYWORD_ilvl,
	KEYWORD_imprint,
	KEYWORD_ind,
	KEYWORD_ins,
	KEYWORD_insideH,
	KEYWORD_insideV,
	KEYWORD_instrText,
	KEYWORD_isLgl,
	KEYWORD_jc,
	KEYWORD_keepLines,
	KEYWORD_keepNext,
	KEYWORD_kern,
	KEYWORD_kinsoku,
	KEYWORD_lang,
	KEYWORD_lastRenderedPageBreak,
	KEYWORD_latentStyles,
	KEYWORD_layoutRawTableWidth,
	KEYWORD_layoutTableRowsApart,
	KEYWORD_left,
	KEYWORD_legacy,
	KEYWORD_lid,
	KEYWORD_lineWrapLikeWord6,
	KEYWORD_link,
	KEYWORD_linkedToFile,
	KEYWORD_linkStyles,
	KEYWORD_linkToQuery,
	KEYWORD_listEntry,
	KEYWORD_listItem,
	KEYWORD_listSeparator,
	KEYWORD_lock,
	KEYWORD_locked,
	KEYWORD_lnNumType,
	KEYWORD_lsdException,
	KEYWORD_lvl,
	KEYWORD_lvlJc,
	KEYWORD_lvlOverride,
	KEYWORD_lvlPicBulletId,
	KEYWORD_lvlRestart,
	KEYWORD_lvlText,
	KEYWORD_mailAsAttachment,
	KEYWORD_mailMerge,
	KEYWORD_mailSubject,
	KEYWORD_mainDocumentType,
	KEYWORD_mappedName,
	KEYWORD_marBottom,
	KEYWORD_marH,
	KEYWORD_marLeft,
	KEYWORD_marRight,
	KEYWORD_marTop,
	KEYWORD_marW,
	KEYWORD_matchSrc,
	KEYWORD_maxLength,
	KEYWORD_mirrorIndents,
	KEYWORD_mirrorMargins,
	KEYWORD_monthLong,
	KEYWORD_monthShort,
	KEYWORD_moveFrom,
	KEYWORD_moveFromRangeEnd,
	KEYWORD_moveFromRangeStart,
	KEYWORD_moveTo,
	KEYWORD_moveToRangeEnd,
	KEYWORD_moveToRangeStart,
	KEYWORD_movie,
	KEYWORD_multiLevelType,
	KEYWORD_mwSmallCaps,
	KEYWORD_name,
	KEYWORD_next,
	KEYWORD_noBorder,
	KEYWORD_noBreakHyphen,
	KEYWORD_noColumnBalance,
	KEYWORD_noEndnote,
	KEYWORD_noExtraLineSpacing,
	KEYWORD_noLeading,
	KEYWORD_noLineBreaksAfter,
	KEYWORD_noLineBreaksBefore,
	KEYWORD_noProof,
	KEYWORD_noPunctuationKerning,
	KEYWORD_noResizeAllowed,
	KEYWORD_noSpaceRaiseLower,
	KEYWORD_noTabHangInd,
	KEYWORD_notTrueType,
	KEYWORD_noWrap,
	KEYWORD_nsid,
	KEYWORD_num,
	KEYWORD_numbering,
	KEYWORD_numberingChange,
	KEYWORD_numFmt,
	KEYWORD_numId,
	KEYWORD_numIdMacAtCleanup,
	KEYWORD_numPicBullet,
	KEYWORD_numPr,
	KEYWORD_numRestart,
	KEYWORD_numStart,
	KEYWORD_numStyleLink,
	KEYWORD_nvPicPr,
	KEYWORD_object,
	KEYWORD_odso,
	KEYWORD_oMath,
	KEYWORD_optimizeForBrowser,
	KEYWORD_outline,
	KEYWORD_outlineLvl,
	KEYWORD_overflowPunct,
	KEYWORD_p,
	KEYWORD_pageBreakBefore,
	KEYWORD_panose1,
	KEYWORD_paperSrc,
	KEYWORD_pBdr,
	KEYWORD_permEnd,
	KEYWORD_permStart,
	KEYWORD_personal,
	KEYWORD_personalCompose,
	KEYWORD_personalReply,
	KEYWORD_pgBorders,
	KEYWORD_pgMar,
	KEYWORD_pgNum,
	KEYWORD_pgNumType,
	KEYWORD_pgSz,
	KEYWORD_pic,
	KEYWORD_pict,
	KEYWORD_picture,
	KEYWORD_pitch,
	KEYWORD_pixelsPerInch,
	KEYWORD_placeholder,
	KEYWORD_pPr,
	KEYWORD_pPrChange,
	KEYWORD_pPrDefault,
	KEYWORD_pos,
	KEYWORD_position,
	KEYWORD_printBodyTextBeforeHeader,
	KEYWORD_printColBlack,
	KEYWORD_printerSettings,
	KEYWORD_printFormsData,
	KEYWORD_printFractionalCharacterWidth,
	KEYWORD_printPostScriptOverText,
	KEYWORD_printTwoOnOne,
	KEYWORD_proofErr,
	KEYWORD_proofState,
	KEYWORD_pStyle,
	KEYWORD_ptab,
	KEYWORD_qFormat,
	KEYWORD_query,
	KEYWORD_r,
	KEYWORD_readModeInkLockDown,
	KEYWORD_recipientData,
	KEYWORD_recipients,
	KEYWORD_relyOnVML,
	KEYWORD_removeDateAndTime,
	KEYWORD_removePersonalInformation,
	KEYWORD_result,
	KEYWORD_revisionView,
	KEYWORD_rFonts,
	KEYWORD_richText,
	KEYWORD_right,
	KEYWORD_rPr,
	KEYWORD_rPrChange,
	KEYWORD_rPrDefault,
	KEYWORD_rsid,
	KEYWORD_rsidRoot,
	KEYWORD_rsids,
	KEYWORD_rStyle,
	KEYWORD_rt,
	KEYWORD_rtl,
	KEYWORD_rtlGutter,
	KEYWORD_ruby,
	KEYWORD_rubyAlign,
	KEYWORD_rubyBase,
	KEYWORD_rubyPr,
	KEYWORD_saveFormsData,
	KEYWORD_saveInvalidXml,
	KEYWORD_savePreviewPicture,
	KEYWORD_saveThroughXslt,
	KEYWORD_saveSmartTagsAsXml,
	KEYWORD_saveSubsetFonts,
	KEYWORD_saveXmlDataOnly,
	KEYWORD_scrollbar,
	KEYWORD_sdt,
	KEYWORD_sdtContent,
	KEYWORD_sdtEndPr,
	KEYWORD_sdtPr,
	KEYWORD_sectPr,
	KEYWORD_sectPrChange,
	KEYWORD_selectFldWithFirstOrLastChar,
	KEYWORD_semiHidden,
	KEYWORD_separator,
	KEYWORD_settings,
	KEYWORD_shadow,
	KEYWORD_shapeDefaults,
	KEYWORD_shapeLayoutLikeWW8,
	KEYWORD_shd,
	KEYWORD_showBreaksInFrames,
	KEYWORD_showEnvelope,
	KEYWORD_showingPlcHdr,
	KEYWORD_showXMLTags,
	KEYWORD_sig,
	KEYWORD_size,
	KEYWORD_sizeAuto,
	KEYWORD_smallCaps,
	KEYWORD_smartTag,
	KEYWORD_smartTagPr,
	KEYWORD_smartTagType,
	KEYWORD_snapToGrid,
	KEYWORD_softHyphen,
	KEYWORD_sourceFileName,
	KEYWORD_spaceForUL,
	KEYWORD_spacing,
	KEYWORD_spacingInWholePoints,
	KEYWORD_specVanish,
	KEYWORD_splitPgBreakAndParaMark,
	KEYWORD_spPr,
	KEYWORD_src,
	KEYWORD_start,
	KEYWORD_startOverride,
	KEYWORD_statusText,
	KEYWORD_storeMappedDataAs,
	KEYWORD_strictFirstAndLastChars,
	KEYWORD_strike,
	KEYWORD_style,
	KEYWORD_styleLink,
	KEYWORD_styleLockQFset,
	KEYWORD_styleLockTheme,
	KEYWORD_stylePaneFormatFilter,
	KEYWORD_stylePaneSortMethod,
	KEYWORD_styles,
	KEYWORD_subDoc,
	KEYWORD_subFontBySize,
	KEYWORD_suff,
	KEYWORD_summaryLength,
	KEYWORD_suppressAutoHypens,
	KEYWORD_suppressBottomSpacing,
	KEYWORD_suppressLineNumbers,
	KEYWORD_suppressOverlap,
	KEYWORD_suppressSpacingAtTopOfPage,
	KEYWORD_suppressSpBfAfterPgBrk,
	KEYWORD_suppressTopSpacing,
	KEYWORD_suppressTopSpacingWP,
	KEYWORD_swapBordersFacingPages,
	KEYWORD_sym,
	KEYWORD_sz,
	KEYWORD_szCs,
	KEYWORD_t,
	KEYWORD_tab,
	KEYWORD_table,
	KEYWORD_tabs,
	KEYWORD_tag,
	KEYWORD_targetScreenSz,
	KEYWORD_tbl,
	KEYWORD_tblBorders,
	KEYWORD_tblCellMar,
	KEYWORD_tblCellSpacing,
	KEYWORD_tblGrid,
	KEYWORD_tblGridChange,
	KEYWORD_tblHeader,
	KEYWORD_tblInd,
	KEYWORD_tblLayout,
	KEYWORD_tblLook,
	KEYWORD_tblOverlap,
	KEYWORD_tblpPr,
	KEYWORD_tblPr,
	KEYWORD_tblPrChange,
	KEYWORD_tblPrEx,
	KEYWORD_tblPrExChange,
	KEYWORD_tblStyle,
	KEYWORD_tblStylePr,
	KEYWORD_tblStyleColBandSize,
	KEYWORD_tblStyleRowBandSize,
	KEYWORD_tblW,
	KEYWORD_tc,
	KEYWORD_tcBorders,
	KEYWORD_tcFitText,
	KEYWORD_tcMar,
	KEYWORD_tcPr,
	KEYWORD_tcPrChange,
	KEYWORD_tcW,
	KEYWORD_temporary,
	KEYWORD_text,
	KEYWORD_textAlignment,
	KEYWORD_textboxTightWrap,
	KEYWORD_textDirection,
	KEYWORD_textInput,
	KEYWORD_themeFontLang,
	KEYWORD_titlePg,
	KEYWORD_tl2br,
	KEYWORD_tmpl,
	KEYWORD_top,
	KEYWORD_topLinePunct,
	KEYWORD_tr,
	KEYWORD_tr2bl,
	KEYWORD_trackRevisions,
	KEYWORD_trHeight,
	KEYWORD_trPr,
	KEYWORD_trPrChange,
	KEYWORD_truncateFontHeightsLikeWP6,
	KEYWORD_txbxContent,
	KEYWORD_type,
	KEYWORD_types,
	KEYWORD_u,
	KEYWORD_udl,
	KEYWORD_uiCompat97To2003,
	KEYWORD_uiPriority,
	KEYWORD_ulTrailSpace,
	KEYWORD_underlineTabInNumList,
	KEYWORD_unhideWhenUsed,
	KEYWORD_uniqueTag,
	KEYWORD_updateFields,
	KEYWORD_useAltKinsokuLineBreakRules,
	KEYWORD_useAnsiKerningPairs,
	KEYWORD_useFELayout,
	KEYWORD_useNormalStyleForList,
	KEYWORD_usePrinterMetrics,
	KEYWORD_useSingleBorderforContiguousCells,
	KEYWORD_useWord2002TableStyleRules,
	KEYWORD_useWord97LineBreakRules,
	KEYWORD_useXSLTWhenSaving,
	KEYWORD_vAlign,
	KEYWORD_vanish,
	KEYWORD_vertAlign,
	KEYWORD_view,
	KEYWORD_viewMergedData,
	KEYWORD_vMerge,
	KEYWORD_yearLong,
	KEYWORD_yearShort,
	KEYWORD_w,
	KEYWORD_wAfter,
	KEYWORD_wBefore,
	KEYWORD_webHidden,
	KEYWORD_webSettings,
	KEYWORD_widowControl,
	KEYWORD_wordWrap,
	KEYWORD_wpJustification,
	KEYWORD_wpSpaceWidth,
	KEYWORD_wrapTrailSpaces,
	KEYWORD_writeProtection,
	KEYWORD_zoom
};

#endif //_OXMLI_TYPES_H_

