
#ifndef XWSAERROR_H
#define XWSAERROR_H


#ifdef __cplusplus
extern "C" {
#endif

int XWSA_GetErrorString(int nErrorCode, TCHAR * lpszBuf, int nBufSize);
int XWSA_GetErrorCode(const TCHAR * lpszErrorString);
int XWSA_GetErrorStringSize();
int XWSA_GetShortDescription(int nErrorCode, TCHAR * lpszBuf, int nBufSize);
int XWSA_GetShortDescriptionSize();
int XWSA_GetLongDescription(int nErrorCode, TCHAR * lpszBuf, int nBufSize);
int XWSA_GetLongDescriptionSize();

#ifdef __cplusplus
}
#endif

#endif //XWSAERROR_H
