
// FileEncrypt.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CFileEncryptApp:
// �йش����ʵ�֣������ FileEncrypt.cpp
//

class CFileEncryptApp : public CWinApp
{
public:
	CFileEncryptApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CFileEncryptApp theApp;