// fp_PageSize.h
#ifndef FP_PAGESIZE_H
#define FP_PAGESIZE_H


class fp_PageSize
{
public:
	enum Unit
	{
		mm,
		cm,
		inch,
		PaperUnit,		//  100 per inch
		LayoutUnit,		// 1440 per inch
		_last_predefined_unit_dont_use_
	};

	enum Predefined
	{
		// If you append a predefined pagesize here, don't forget
		// to update the cpp accordingly.
		A0, A1, A2, A3, A4, A5, A6,
		B0, B1, B2, B3, B4, B5, B6,
		Legal, Folio, Letter,
		// append new pagesizes here
		_last_predefined_pagesize_dont_use_
	};

	fp_PageSize(Predefined preDef);
	fp_PageSize(const char *name);
	fp_PageSize(double w, double h, Unit u);

	void Set(Predefined preDef);
	void Set(const char *name);
	void Set(double w, double h, Unit u);

	double Width(Unit u) const;
	double Height(Unit u) const;

	static Predefined NameToPredefined(const char *name);
	static const char * PredefinedToName(Predefined preDef);

private:
	double m_iWidth;
	double m_iHeight;
};

#endif	// FP_PAGESIZE_H

