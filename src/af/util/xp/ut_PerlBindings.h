#ifndef UT_PERLBINDINGS_H
#define UT_PERLBINDINGS_H

class UT_String;
class XAP_Frame;

class UT_PerlBindings
{
public:
	static void evalFile(const UT_String& filename, XAP_Frame* frame = 0);
};

#endif // UT_PERLBINDINGS_H
