#ifndef AP_UNIXDIALOG_GRAMMAR_H
#define AP_UNIXDIALOG_GRAMMAR_H

#include <gtk/gtk.h>
#include "DialogGrammar.h"

class XAP_Frame;
	
class AP_UnixDialog_Grammar : public DialogGrammar
{
public:

	AP_UnixDialog_Grammar (XAP_DialogFactory * pDlgFactory, 
						 XAP_Dialog_Id id);
	virtual ~AP_UnixDialog_Grammar (void);

	static XAP_Dialog *	static_constructor (XAP_DialogFactory *, XAP_Dialog_Id id);

	virtual void runModal (XAP_Frame * pFrame);

	// callbacks can fire these events
	virtual void onChangeClicked	  (void);
	virtual void onIgnoreClicked	  (void);
	virtual void onSuggestionSelected (void);
	virtual void onSuggestionChanged  (void);
	
	const GtkWidget * getWindow (void) const { return m_wDialog; }

protected:

	virtual GtkWidget * _constructWindow	   (void);
	void 			   	_updateWindow 	   		();

private:

	char * 		_convertToMB   (const UT_UCSChar *wword);
	char * 		_convertToMB (const UT_UCSChar *wword, 
								   UT_sint32 iLength);
	UT_UCSChar * _convertFromMB (const char *word);

	void 	_updateOriginal();
	void	_updateSuggestions();
	void	_updateExplanation();

	// pointers to widgets we need to query/set
	GtkWidget * m_wDialog;
	GtkWidget * m_txOriginal;
	GtkWidget * m_eChange;
	GtkWidget * m_lvSuggestions;
	GtkWidget * m_txExplanation;
   
	GdkColor m_highlight;

	guint m_listHandlerID;
	guint m_replaceHandlerID;
};

#endif /* AP_UNIXDIALOG_GRAMMAR_H */
