/*! \page fmt_overview Formatter

    FL_DocLayout is a formatted representation of a specific PD_Document, 
    formatted for a specific GR_Graphics context.  

    A %FL_DocLayout encapsulates two related hierarchies of objects.  

    The \e logical (or \e content) hierarchy corresponds to the logical
    structure of the document.

        - each FL_DocLayout contains a list of fl_SectionLayout objects
        - each fl_SectionLayout are composed of fl_BlockLayout objects

    Where each fl_BlockLayout corresponds to a logical element in
    the PD_Document (i.e., usually a paragraph of text).

    The \e physical (or \e layout) hierarchy, by contrast, encapsulates
    the subdivision of physical space into objects of successively
    finer granularity.

        - each FL_DocLayout contains a list of fp_Page objects, each
          of which was constructed during the process of formatting
          the document
        - each fp_Page is a list of \link fp_Column fp_Columns \endlink
        - each fp_Column is a list of \link fp_Line fp_Lines \endlink
        - each fp_Line is a list of \link fp_Run fp_Runs \endlink

    Where each fp_Run contains some fragment of content from the original 
    document, usually text.  

*/
