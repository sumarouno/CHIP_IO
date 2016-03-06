/*
Copyright (c) 2016 Robert Wolterman

Original BBIO Author Justin Cooper
Modified for CHIP_IO Author Robert Wolterman

Copyright (c) 2013 Adafruit
Author: Justin Cooper

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "Python.h"
#include "constants.h"
#include "common.h"
#include "c_adc.h"

// Cleanup function commented out because it is currently disabled and this
// generates unused method warnings. See adc_methods[] definition further 
// below for reason for disabling the cleanup function.
// python function cleanup()
static PyObject *py_cleanup(PyObject *self, PyObject *args)
{
    // unexport the ADC
    adc_cleanup();

    Py_RETURN_NONE;
}

// python function setup()
static PyObject *py_setup_adc(PyObject *self, PyObject *args)
{
    if (adc_setup())
        Py_RETURN_NONE;
    
    PyErr_SetString(PyExc_RuntimeError, "Unable to setup ADC system. Possible causes are: \n"
                                        "  - Not run with super user permissions");
    return NULL;
}

// python function enable(int en)
static PyObject *py_enable_adc(PyObject *self, PyObject *args)
{
    int en;

    if (!PyArg_ParseTuple(args, "d", &en))
        return NULL;
    
    // check setup was called prior
    if (!adc_initialized)
    {
        PyErr_SetString(PyExc_RuntimeError, "You must setup() ADC prior to calling read.");
        return NULL;
    }
    
    enable(en);
    
    return NULL;
}

// python function read(channel)
static PyObject *py_read(PyObject *self, PyObject *args)
{
    int channel;
    int value;
    int success;
    PyObject *py_value;

    if (!PyArg_ParseTuple(args, "s", &channel))
        return NULL;

    // check setup was called prior
    if (!adc_initialized || !adc_enabled)
    {
        PyErr_SetString(PyExc_RuntimeError, "You must setup() ADC prior to calling read.");
        return NULL;
    }

    success = read_value(channel, &value);
    // Mask off the last 6 bits as the ADC only has 6 bits of resolution
    value &= 0x3F;

    if (success == -1) {
        PyErr_SetFromErrnoWithFilename(PyExc_IOError, "Error while reading channel.");
        return NULL;
    }

    py_value = Py_BuildValue("f", value / 1800.0);

    return py_value;
}

// python function read(channel)
static PyObject *py_read_raw(PyObject *self, PyObject *args)
{
    int channel;
    int value;
    int success;
    PyObject *py_value;

    if (!PyArg_ParseTuple(args, "s", &channel))
        return NULL;

   // check setup was called prior
    if (!adc_initialized || !adc_enabled)
    {
        PyErr_SetString(PyExc_RuntimeError, "You must setup() ADC prior to calling read.");
        return NULL;
    }       

    success = read_value(channel, &value);
    // Mask off the last 6 bits as the ADC only has 6 bits of resolution
    value &= 0x3F;

    if (success == -1) {
        PyErr_SetFromErrnoWithFilename(PyExc_IOError, "Error while reading channel.");
        return NULL;
    }

    py_value = Py_BuildValue("d", value);

    return py_value;
}

static const char moduledocstring[] = "ADC functionality of a CHIP using Python";

PyMethodDef adc_methods[] = {
    {"setup", py_setup_adc, METH_VARARGS, "Set up and start the ADC channel."},
    {"enable", py_enable_adc, METH_VARARGS, "Enable/Disable the ADC channel."},
    {"read", (PyCFunction)py_read, METH_VARARGS, "Read the normalized 0-1.0 analog value for the channel" },
    {"read_raw", (PyCFunction)py_read_raw, METH_VARARGS, "Read the raw analog value for the channel" },
    {"cleanup", py_cleanup, METH_VARARGS, "Clean up ADC."},
    {NULL, NULL, 0, NULL}
};

#if PY_MAJOR_VERSION > 2
static struct PyModuleDef bbadcmodule = {
    PyModuleDef_HEAD_INIT,
    "ADC",       // name of module
    moduledocstring,  // module documentation, may be NULL
    -1,               // size of per-interpreter state of the module, or -1 if the module keeps state in global variables.
    adc_methods
};
#endif

#if PY_MAJOR_VERSION > 2
PyMODINIT_FUNC PyInit_ADC(void)
#else
PyMODINIT_FUNC initADC(void)
#endif
{
    PyObject *module = NULL;

#if PY_MAJOR_VERSION > 2
    if ((module = PyModule_Create(&bbadcmodule)) == NULL)
       return NULL;
#else
    if ((module = Py_InitModule3("ADC", adc_methods, moduledocstring)) == NULL)
       return;
#endif

   define_constants(module);


#if PY_MAJOR_VERSION > 2
    return module;
#else
    return;
#endif
}
