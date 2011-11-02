#include <windows.h>

const int BOX_LEN = 256;

void swap_byte(TCHAR &a, TCHAR &b);
int GetKey(const TCHAR *pass, TCHAR *out);
int encrypt(const TCHAR *fileadd, const TCHAR *key, int &isTerminal);
int decrypt(const TCHAR *fileadd, const TCHAR *key, int &isTerminal);
