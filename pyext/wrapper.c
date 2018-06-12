#include <Python.h>
#include "combstruct2json.h"

/*****************************************
 Adapted from:
 https://dfm.io/posts/python-c-extensions/
 *****************************************/

/* Exception */
static PyObject *Combstruct2JsonError;

/* Docstrings */
static char module_docstring[] =
    "This module provides an interface for parsing combstruct grammars.";
static char parse_file_docstring[] =
    "Parse the combstruct grammar file and return JSON string.";

/* Available functions */
static PyObject *combstruct2json_parse_file(PyObject *self, PyObject *args);

/* Module specification */
static PyMethodDef module_methods[] = {
    {"parse_file", combstruct2json_parse_file, METH_VARARGS, parse_file_docstring},
    {NULL, NULL, 0, NULL}
};

/* Initialize the module */
PyMODINIT_FUNC initcombstruct2json(void)
{
    PyObject *m = Py_InitModule3("combstruct2json", module_methods, module_docstring);
    if (m == NULL)
        return;
    
    // Initializing our custom exception
    Combstruct2JsonError = PyErr_NewException("combstruct2json.error", NULL, NULL);
    Py_INCREF(Combstruct2JsonError);
    PyModule_AddObject(m, "error", Combstruct2JsonError);
}

static PyObject *combstruct2json_parse_file(PyObject *self, PyObject *args)
{
    char *arg_filename;

    /* Parse the input tuple */
    if (!PyArg_ParseTuple(args, "s", &arg_filename)) {
        PyErr_SetString(Combstruct2JsonError, "Parsing filename for `parse_file' failed.");
        return NULL;
    }

    /* Call the external C function to parse the grammar. */
    Grammar* root = readGrammar(arg_filename);

    /* Convert to JSON string. */
    char *ret_json = root->toJson(root);

    if (ret_json == NULL) {
        free(root);
        PyErr_SetString(Combstruct2JsonError, "Parsing grammar failed for unknown reasons.");
        return NULL;
    }

    /* Build the Python output string. */
    PyObject *py_ret_json = Py_BuildValue("s", ret_json);

    /* Clean up. */
    free(root);
    free(ret_json);

    /* Return output. */
    return py_ret_json;
}