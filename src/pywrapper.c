#include <Python.h>
#include "../combstruct2json.h"

/*****************************************
 Adapted from:
 https://dfm.io/posts/python-c-extensions/
 *****************************************/

/* Exception */
static PyObject *Combstruct2JsonError;

/* Docstrings */
static char module_docstring[] =
    "This module provides an interface for parsing combstruct grammars.";
static char read_file_docstring[] =
    "Parse the combstruct grammar file and return JSON string.";

/* Available functions */
static PyObject *combstruct2json_read_file(PyObject *self, PyObject *args);

/* Module specification */
static PyMethodDef module_methods[] = {
    {"read_file", combstruct2json_read_file, METH_VARARGS, read_file_docstring},
    {NULL, NULL, 0, NULL}
};

/* Initialize the module */
void initcombstruct2json(void)
{
    PyObject *m = Py_InitModule3("combstruct2json", module_methods, module_docstring);
    if (m == NULL)
        return;
    
    // Initializing our custom exception
    Combstruct2JsonError = PyErr_NewException("combstruct2json.error", NULL, NULL);
    Py_INCREF(Combstruct2JsonError);
    PyModule_AddObject(m, "error", Combstruct2JsonError);
}

static PyObject *combstruct2json_read_file(PyObject *self, PyObject *args)
{
    char *arg_filename;

    /* Parse the input tuple */
    if (!PyArg_ParseTuple(args, "s", &arg_filename)) {
        PyErr_SetString(Combstruct2JsonError, "Parsing filename for `read_file' failed.");
        return NULL;
    }

    /* Call the external C function to parse the grammar. */
    Grammar* root = readGrammar(arg_filename);

    /* Convert to JSON string. */
    char *ret_jsonstr = root->toJson(root);

    if (ret_jsonstr == NULL) {
        free(root);
        PyErr_SetString(Combstruct2JsonError, "Parsing grammar failed for unknown reasons.");
        return NULL;
    }

    /* Build the Python output string. */
    PyObject *py_ret_jsonstr = Py_BuildValue("s", ret_jsonstr);

    /* Run "import json; json.loads(s)" to return dictionary. */
    PyObject* myModuleString = PyString_FromString((char*)"json");
    PyObject* myModule = PyImport_Import(myModuleString);
    PyObject* myFunction = PyObject_GetAttrString(myModule, (char*)"loads");
    PyObject* myArgs = PyTuple_Pack(1, py_ret_jsonstr);
    PyObject* py_ret_json = PyObject_CallObject(myFunction, myArgs);

    /* Clean up. */
    free(root);
    free(ret_jsonstr);

    Py_DECREF(myModuleString);
    Py_DECREF(myModule);
    Py_DECREF(myFunction);
    Py_DECREF(myArgs);

    /* Return output. */
    return py_ret_json;
}