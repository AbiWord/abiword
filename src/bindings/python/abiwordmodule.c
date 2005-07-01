
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <Python.h>
#include <pygobject.h>

void abiword_register_classes (PyObject *d);
extern libabiword_init (void);
extern PyMethodDef abiword_functions[];
 
DL_EXPORT(void)
initabiword(void)
{
    PyObject *m, *d;
 
    init_pygobject (); 
	/* init_pygtk () */
	libabiword_init ();

    m = Py_InitModule ("abiword", abiword_functions);
    d = PyModule_GetDict (m);
 
    abiword_register_classes (d);
 
    if (PyErr_Occurred ()) {
        Py_FatalError ("can't initialise module abiword");
    }
}

DL_EXPORT(void)
initpyabiword(void)
{
	initabiword ();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
