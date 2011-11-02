#include "stdafx.h"
#include "rc4.h"
#include <fstream>
#include <string>
#include <algorithm>
using namespace std;

const unsigned int FILECACHESIZE = 1024 * 1024;

void GetNewFileName(const TCHAR *oldname, const TCHAR *addstr, TCHAR *newname)
{
	int i = lstrlen(oldname) - 1;
	while (i >= 0 && oldname[i--] != '.');
	i++;
	lstrcpy(newname, oldname);
	newname[i] = 0;
	lstrcat(newname, addstr);
	lstrcat(newname, oldname + i);
}

void swap_byte(TCHAR &a, TCHAR &b)
{
	a = a ^ b;
	b = a ^ b;
	a = a ^ b;
}

int GetKey(const TCHAR *pass, TCHAR *out)
{
	if (pass == NULL || out == NULL)
	{
		return -1;
	}
	int i;
	size_t pass_len = lstrlen(pass);
	for (i = 0; i < BOX_LEN; i++)
	{
		out[i] = i;
	}
	int j = 0;
	for (i = 0; i < BOX_LEN; i++)
	{
		j = (pass[i % pass_len] + out[i] + j) % BOX_LEN;
		swap_byte(out[i], out[j]);
	}
	return 0;
}

int encrypt(const TCHAR *fileadd, const TCHAR *key, int &isTerminal)
{
	size_t key_len = lstrlen(key);
	TCHAR *c = new TCHAR[FILECACHESIZE];
	HANDLE hFile1, hFile2;
	if((hFile1 = CreateFile(fileadd, GENERIC_READ, 
		FILE_SHARE_READ,	0, OPEN_EXISTING, 
		FILE_FLAG_SEQUENTIAL_SCAN, 0)) == INVALID_HANDLE_VALUE)
	{
		MessageBox(0,_T("不能打开文件!\n"),_T("Error!"),0);
		return -1;
	}
	TCHAR *newfileadd = new TCHAR[lstrlen(fileadd) + lstrlen(_T("_en")) + 1];
	GetNewFileName(fileadd, _T("_en"), newfileadd);
//	lstrcpy(newfileadd, fileadd);
//	lstrcat(newfileadd, _T("_en"));
	if((hFile2 = CreateFile(newfileadd, GENERIC_WRITE, 
		FILE_SHARE_WRITE,	0, OPEN_ALWAYS, 
		FILE_FLAG_SEQUENTIAL_SCAN, 0)) == INVALID_HANDLE_VALUE)
	{
		MessageBox(0,_T("不能打开文件!\n"),_T("Error!"),0);
		return -1;
	}
	TCHAR* mBox = new TCHAR[BOX_LEN];

	if(GetKey(key, mBox) == -1) 
		return -1;

	unsigned int i=0;
	int x=0;
	int y=0;
	unsigned long mynum1,mynum2;
	while(ReadFile(hFile1,c,FILECACHESIZE,&mynum1,0) && !isTerminal)
	{
		for (i = 0; i < FILECACHESIZE; i++)
		{
			x = (x + 1) % BOX_LEN;
			y = (mBox[x] + y) % BOX_LEN;
			swap_byte(mBox[x], mBox[y]);
			c[i] = c[i] ^ mBox[(mBox[x] + mBox[y]) % BOX_LEN];

		}
		WriteFile(hFile2,c,mynum1,&mynum2,0);
		if (mynum1 < FILECACHESIZE)
			break;
	}
	
	CloseHandle(hFile2);
	CloseHandle(hFile1);
	if (isTerminal)
	{
		DeleteFile(newfileadd);
	}
	
	delete[] mBox;
	delete[] newfileadd;
	delete[] c;
	
	return 0;
}

int decrypt(const TCHAR *fileadd, const TCHAR *key, int &isTerminal)
{
	size_t key_len = lstrlen(key);
	TCHAR *c = new TCHAR[FILECACHESIZE];
	HANDLE hFile1, hFile2;
	if((hFile1 = CreateFile(fileadd, GENERIC_READ, 
		FILE_SHARE_READ,	0, OPEN_EXISTING, 
		FILE_FLAG_SEQUENTIAL_SCAN, 0)) == INVALID_HANDLE_VALUE)
	{
		MessageBox(0,_T("不能打开文件!\n"),_T("Error!"),0);
		return -1;
	}
	TCHAR *newfileadd = new TCHAR[lstrlen(fileadd) + lstrlen(_T("_de")) + 1];
	GetNewFileName(fileadd, _T("_de"), newfileadd);
	if((hFile2 = CreateFile(newfileadd, GENERIC_WRITE, 
		FILE_SHARE_WRITE,	0, OPEN_ALWAYS, 
		FILE_FLAG_SEQUENTIAL_SCAN, 0)) == INVALID_HANDLE_VALUE)
	{
		MessageBox(0,_T("不能打开文件!\n"),_T("Error!"),0);
		return -1;
	}
	TCHAR* mBox = new TCHAR[BOX_LEN];

	if(GetKey(key, mBox) == -1) 
		return -1;

	unsigned int i=0;
	int x=0;
	int y=0;
	unsigned long mynum1,mynum2;
	while(ReadFile(hFile1,c,FILECACHESIZE,&mynum1,0) && !isTerminal)
	{
		for (i = 0; i < FILECACHESIZE; i++)
		{
			x = (x + 1) % BOX_LEN;
			y = (mBox[x] + y) % BOX_LEN;
			swap_byte(mBox[x], mBox[y]);
			c[i] = c[i] ^ mBox[(mBox[x] + mBox[y]) % BOX_LEN];
		}
		WriteFile(hFile2,c,mynum1,&mynum2,0);
		if (mynum1 < FILECACHESIZE)
			break;
	}

	CloseHandle(hFile2);
	CloseHandle(hFile1);
	if (isTerminal)
	{
		DeleteFile(newfileadd);
	}
	delete[] mBox;
	delete[] newfileadd;
	delete[] c;
	
	return 0;
}