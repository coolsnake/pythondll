// PythonDll.cpp : Defines the exported functions for the DLL application.
//
#include "stdafx.h"

#include <Python.h>
#include "pyhelper.h"
#include "arrayobject.h"
#include <wchar.h>

CPyInstance hInstance;
CPyObject pName, pModule, pFunc, pValue, pInputs, pargs, pArray;

long RunPythonCode(double *mql5_arr[][20], int array_rows, int array_cols, char FileName[], char FuncName[], int *args[]) {
	
	if (PyArray_API == NULL)
	{
		import_array();
	}
	pName = PyUnicode_FromString(FileName);
	pModule = PyImport_Import(pName);

	if (pModule)
	{
		pFunc = PyObject_GetAttrString(pModule, FuncName);

		if (pFunc && PyCallable_Check(pFunc))
		{
			// Build the 2D array in C++
			int SIZE = array_rows;
			npy_intp dims[2]{ array_rows, array_cols };
			const int ND = 2;

			// Convert it to a NumPy array.
			pArray = PyArray_SimpleNewFromData(ND, dims, NPY_DOUBLE, reinterpret_cast<void*>(mql5_arr));

			if (!pArray) {
				printf("Error converting to python array.\n");
				return -2;
			}

			//Add python array to tuple
			pInputs = PyTuple_New(2); //Number of inputs to function
			PyTuple_SetItem(pInputs, 0, pArray); //Add value to inputs

			//Add args to Inputs
			int foo = sizeof(args) / sizeof(int);
			npy_intp argsdims[1]{ foo };
			pargs = PyArray_SimpleNewFromData(1, argsdims, NPY_INT, reinterpret_cast<void*>(args));
			PyTuple_SetItem(pInputs, 1, pargs); //Add value to inputs

			// Execute function pFunc with variables pInputs
			pValue = PyObject_CallObject(pFunc, pInputs); // <--- This is the problem causing access violation!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

			long pyResult = PyLong_AsLong(pValue);
			return pyResult;
		}
		else
		{
			printf("ERROR: Python function in module\n");
		}

	}
	else
	{
		printf_s("ERROR: Module not imported\n");
		return -3;
	}

	return 1;
}

_DLLAPI long __stdcall CallPython(double *mql5_arr[][20], int array_rows, int array_cols, wchar_t* FileName, wchar_t* FuncName, int *args[]) //First column should be the targets
{
	int n_file = wcslen(FileName) + 1;
	int n_func = wcslen(FuncName)+1;
	size_t i_file,i_func;
	char *CharFileName = new char[n_file];
	char *CharFuncName = new char[n_func];
	wcstombs_s(&i_file, CharFileName, n_file, FileName, n_file);
	wcstombs_s(&i_func, CharFuncName, n_func, FuncName, n_func);

	long Result = RunPythonCode(mql5_arr, array_rows, array_cols, CharFileName, CharFuncName, args);
	delete(CharFileName);
	delete(CharFuncName);
	return Result;
}