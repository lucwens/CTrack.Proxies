// XWSAError.cpp  Version 1.0
//
// Author:  Hans Dietrich
//          hdietrich@gmail.com
//
// Description:
//     XWSAError.cpp implements functions to retrieve short and long descriptions
//     of Winsock error codes.
//
// History:
//     Version 1.0 - 2005 January 1
//     - Initial public release
//
// Public APIs:
//                 NAME                              DESCRIPTION
//     ------------------------------  ------------------------------------------
//     XWSA_GetErrorCode()             Retrieve error code from error code string
//     XWSA_GetErrorString()           Retrieve error code string from error code
//     XWSA_GetErrorStringSize()       Retrieve max size of an error code string
//     XWSA_GetLongDescription()       Retrieve long description
//     XWSA_GetLongDescriptionSize()   Retrieve max size of a long description
//     XWSA_GetShortDescription()      Retrieve short description
//     XWSA_GetShortDescriptionSize()  Retrieve max size of a short description
//
// Known limitations:
//     1.  List of Winsock error codes may not be complete.
//     2.  XWSAError has only been tested with Winsock 2.
//
// License:
//     This software is released into the public domain.  You are free to use 
//     it in any way you like, except that you may not sell this source code.
//
//     This software is provided "as is" with no expressed or implied warranty.
//     I accept no liability for any damage or loss of business that this 
//     software may cause.
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NetworkErrorTable.h"
#include "NetworkError.h"

#pragma warning(disable : 4127)	// for _ASSERTE: conditional expression is constant

// following sizes include terminating nul char
static int XWSA_MaxErrorStringSize      = 501;
static int XWSA_MaxShortDescriptionSize = 501;
static int XWSA_MaxLongDescriptionSize  = 501;


///////////////////////////////////////////////////////////////////////////////
//
// XWSA_GetErrorString()
//
// Purpose:     Retrieve the string associated with the error code. For
//              example, calling XWSA_GetErrorString() with an error code
//              of 10004 will return the string "WSAEINTR".
//
// Parameters:  nErrorCode - [in] Winsock error code
//              lpszBuf    - [out] pointer to buffer that receives the string
//              nBufSize   - [in] size of buffer in TCHARs
//
// Returns:     int - 1 if error code string found; 0 otherwise
//
int  XWSA_GetErrorString(int nErrorCode, TCHAR * lpszBuf, int nBufSize)
{
	_ASSERTE(lpszBuf);

	if (lpszBuf)
	{
		lpszBuf[0] = _T('\0');

		for (int i = 0; i < NUMERRORTABLEENTRIES; i++)
		{
			if (error_table[i].nErrorCode == nErrorCode)
			{
				_tcsncpy(lpszBuf, error_table[i].pszErrorString, nBufSize-1);
				lpszBuf[nBufSize-1] = _T('\0');
				return 1;
			}
		}
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
//
// XWSA_GetErrorCode()
//
// Purpose:     Retrieve numeric error code associated with error code string.
//              For example, calling XWSA_GetErrorCode() with an error code
//              string of "WSAEINTR" will return the value 10004.
//
// Parameters:  lpszErrorString - [in] pointer to error code string
//
// Returns:     int - numeric value of error code
//
int  XWSA_GetErrorCode(const TCHAR * lpszErrorString)
{
	_ASSERTE(lpszErrorString);

	if (lpszErrorString && lpszErrorString[0] != _T('\0'))
	{
		for (int i = 0; i < NUMERRORTABLEENTRIES; i++)
		{
			if (_tcsicmp(error_table[i].pszErrorString, lpszErrorString) == 0)
			{
				return error_table[i].nErrorCode;
			}
		}
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
//
// XWSA_GetErrorStringSize()
//
// Purpose:     Returns the maximum size in TCHARs of an error code string.
//
// Parameters:  none
//
// Returns:     int - maximum size in TCHARs of an error code string
//
int  XWSA_GetErrorStringSize()
{
	return XWSA_MaxErrorStringSize;
}

///////////////////////////////////////////////////////////////////////////////
//
// XWSA_GetShortDescription()
//
// Purpose:     Retrieve the short description string associated with the 
//              error code.
//
// Parameters:  nErrorCode - [in] Winsock error code
//              lpszBuf    - [out] pointer to buffer that receives the string
//              nBufSize   - [in] size of buffer in TCHARs
//
// Returns:     int - 1 if error code found; 0 otherwise
//
int  XWSA_GetShortDescription(int nErrorCode, TCHAR * lpszBuf, int nBufSize)
{
	_ASSERTE(lpszBuf);

	if (lpszBuf)
	{
		lpszBuf[0] = _T('\0');

		for (int i = 0; i < NUMERRORTABLEENTRIES; i++)
		{
			if (error_table[i].nErrorCode == nErrorCode)
			{
				_tcsncpy(lpszBuf, error_table[i].pszShortDescription, nBufSize-1);
				lpszBuf[nBufSize-1] = _T('\0');
				return 1;
			}
		}
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
//
// XWSA_GetShortDescriptionSize()
//
// Purpose:     Returns the maximum size in TCHARs of a short description 
//              string.
//
// Parameters:  none
//
// Returns:     int - maximum size in TCHARs of a short description string
//
int  XWSA_GetShortDescriptionSize()
{
	return XWSA_MaxShortDescriptionSize;
}

///////////////////////////////////////////////////////////////////////////////
//
// XWSA_GetLongDescription()
//
// Purpose:     Retrieve the long description string associated with the 
//              error code.
//
// Parameters:  nErrorCode - [in] Winsock error code
//              lpszBuf    - [out] pointer to buffer that receives the string
//              nBufSize   - [in] size of buffer in TCHARs
//
// Returns:     int - 1 if error code found; 0 otherwise
//
int  XWSA_GetLongDescription(int nErrorCode, TCHAR * lpszBuf, int nBufSize)
{
	_ASSERTE(lpszBuf);

	if (lpszBuf)
	{
		lpszBuf[0] = _T('\0');

		for (int i = 0; i < NUMERRORTABLEENTRIES; i++)
		{
			if (error_table[i].nErrorCode == nErrorCode)
			{
				_tcsncpy(lpszBuf, error_table[i].pszLongDescription, nBufSize-1);
				lpszBuf[nBufSize-1] = _T('\0');
				return 1;
			}
		}
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
//
// XWSA_GetLongDescriptionSize()
//
// Purpose:     Returns the maximum size in TCHARs of a long description 
//              string.
//
// Parameters:  none
//
// Returns:     int - maximum size in TCHARs of a long description string
//
int  XWSA_GetLongDescriptionSize()
{
	return XWSA_MaxLongDescriptionSize;
}
