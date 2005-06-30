/* -- THIS FILE IS GENERATED - DO NOT EDIT *//* -*- Mode: C; c-basic-offset: 4 -*- */

#include <Python.h>



#line 3 "abiword.override"
#include <Python.h>
#include <pygobject.h>
#include <abi-widget.h>

#line 13 "abiword.c"


/* ---------- types from other modules ---------- */
static PyTypeObject *_PyGtkFrame_Type;
#define PyGtkFrame_Type (*_PyGtkFrame_Type)
static PyTypeObject *_PyGObject_Type;
#define PyGObject_Type (*_PyGObject_Type)


/* ---------- forward type declarations ---------- */



/* ----------- AbiWidget ----------- */

#line 20 "abiword.override"
static int
_wrap_abi_widget_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    GType obj_type = pyg_type_from_object((PyObject *) self);
    static char* kwlist[] = { NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, ":abiword.Widget.__init__", kwlist))
        return -1;

    self->obj = (GObject*)g_object_newv(obj_type, 0, NULL);
    if (!self->obj) {
        PyErr_SetString(PyExc_RuntimeError, "could not create %(typename)s object");
        return -1;
    }

    pygobject_register_wrapper((PyObject *)self);
    return 0;
}
#line 48 "abiword.c"


static PyObject *
_wrap_abi_widget_load_file(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "filename", NULL };
    char *filename;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s:AbiWidget.load_file", kwlist, &filename))
        return NULL;
    ret = abi_widget_load_file(ABI_WIDGET(self->obj), filename);
    return PyBool_FromLong(ret);

}

static PyMethodDef _PyAbiWidget_methods[] = {
    { "load_file", (PyCFunction)_wrap_abi_widget_load_file, METH_VARARGS|METH_KEYWORDS },
    { NULL, NULL, 0 }
};

PyTypeObject PyAbiWidget_Type = {
    PyObject_HEAD_INIT(NULL)
    0,					/* ob_size */
    "abiword.Widget",			/* tp_name */
    sizeof(PyGObject),	        /* tp_basicsize */
    0,					/* tp_itemsize */
    /* methods */
    (destructor)0,	/* tp_dealloc */
    (printfunc)0,			/* tp_print */
    (getattrfunc)0,	/* tp_getattr */
    (setattrfunc)0,	/* tp_setattr */
    (cmpfunc)0,		/* tp_compare */
    (reprfunc)0,		/* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,		/* tp_hash */
    (ternaryfunc)0,		/* tp_call */
    (reprfunc)0,		/* tp_str */
    (getattrofunc)0,	/* tp_getattro */
    (setattrofunc)0,	/* tp_setattro */
    (PyBufferProcs*)0,	/* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL, 				/* Documentation string */
    (traverseproc)0,	/* tp_traverse */
    (inquiry)0,		/* tp_clear */
    (richcmpfunc)0,	/* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,		/* tp_iter */
    (iternextfunc)0,	/* tp_iternext */
    _PyAbiWidget_methods,			/* tp_methods */
    0,					/* tp_members */
    0,		       	/* tp_getset */
    NULL,				/* tp_base */
    NULL,				/* tp_dict */
    (descrgetfunc)0,	/* tp_descr_get */
    (descrsetfunc)0,	/* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_abi_widget_new,		/* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- functions ----------- */

PyMethodDef abiword_functions[] = {
    { NULL, NULL, 0 }
};

/* initialise stuff extension classes */
void
abiword_register_classes(PyObject *d)
{
    PyObject *module;

    if ((module = PyImport_ImportModule("gobject")) != NULL) {
        PyObject *moddict = PyModule_GetDict(module);

        _PyGObject_Type = (PyTypeObject *)PyDict_GetItemString(moddict, "GObject");
        if (_PyGObject_Type == NULL) {
            PyErr_SetString(PyExc_ImportError,
                "cannot import name GObject from gobject");
            return;
        }
    } else {
        PyErr_SetString(PyExc_ImportError,
            "could not import gobject");
        return;
    }
    if ((module = PyImport_ImportModule("gtk")) != NULL) {
        PyObject *moddict = PyModule_GetDict(module);

        _PyGtkFrame_Type = (PyTypeObject *)PyDict_GetItemString(moddict, "Frame");
        if (_PyGtkFrame_Type == NULL) {
            PyErr_SetString(PyExc_ImportError,
                "cannot import name Frame from gtk");
            return;
        }
    } else {
        PyErr_SetString(PyExc_ImportError,
            "could not import gtk");
        return;
    }


#line 159 "abiword.c"
    pygobject_register_class(d, "AbiWidget", ABI_TYPE_WIDGET, &PyAbiWidget_Type, Py_BuildValue("(O)", &PyGtkFrame_Type));
}
