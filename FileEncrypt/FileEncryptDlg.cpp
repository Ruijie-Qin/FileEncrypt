
// FileEncryptDlg.cpp : ʵ���ļ�
//
#include "stdafx.h"
#include "FileEncrypt.h"
#include "FileEncryptDlg.h"
#include "afxdialogex.h"
#include "rc4.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

typedef struct  
{
	CString fileaddress;
	CString password;
}PARAM, *PPARAM;

int thread_ok  = 0;
int isTerminal = 0;
int funcerror = 0;
DWORD WINAPI Thread_Encrypt(LPVOID lpParameter)
{
	PPARAM pparam = (PPARAM)lpParameter;
	funcerror = encrypt(pparam->fileaddress, pparam->password, isTerminal);
	thread_ok = 1;
	return 0;
}
DWORD WINAPI Thread_Decrypt(LPVOID lpParameter)
{
	PPARAM pparam = (PPARAM)lpParameter;
	funcerror = decrypt(pparam->fileaddress, pparam->password, isTerminal);
	thread_ok = 1;
	return 0;
}
// CFileEncryptDlg �Ի���
CFileEncryptDlg::CFileEncryptDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CFileEncryptDlg::IDD, pParent)
	, password1(_T(""))
	, password2(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON1);
}

void CFileEncryptDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_PASSWORD1, password1);
	DDV_MaxChars(pDX, password1, 20);
	DDX_Text(pDX, IDC_PASSWORD2, password2);
	DDV_MaxChars(pDX, password2, 20);
}

BEGIN_MESSAGE_MAP(CFileEncryptDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DROPFILES()
	ON_BN_CLICKED(IDC_ENCRYPT, &CFileEncryptDlg::OnBnClickedEncrypt)
	ON_BN_CLICKED(IDC_DECRYPT, &CFileEncryptDlg::OnBnClickedDecrypt)
	ON_BN_CLICKED(IDC_STOP, &CFileEncryptDlg::OnBnClickedStop)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CFileEncryptDlg ��Ϣ�������
BOOL CFileEncryptDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	defaultTips = _T("��ֱ���Ϸ�Ҫ���ܵ��ļ��������ڼ���\r\n�����ļ�ǰ����������,�����ļ�����ȷ������\r\n����Ӧ��Ϊ6-20λ���ַ�");
	SetDlgItemText(IDC_FILE_INFORMATION, defaultTips);
	SetButtonFalse();
	DragAcceptFiles(TRUE);
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CFileEncryptDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CFileEncryptDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CFileEncryptDlg::OnDropFiles(HDROP hDropInfo)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	int DropCount = DragQueryFile(hDropInfo, -1, NULL, 0);
	CString FileInfo;
	TCHAR *pName;
	WIN32_FIND_DATA FindFileData;
	SYSTEMTIME mysystime;
	FILETIME loctime;
	if (DropCount <= 1)
	{
		HANDLE hFile;
		int NameSize = DragQueryFile(hDropInfo, 0, NULL, 0);
		pName = new TCHAR[NameSize + 1];
		DragQueryFile(hDropInfo, 0, pName, NameSize+1);
		hFile = FindFirstFile(pName, &FindFileData);
		FindClose(hFile);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			FileInfo = _T("�ǲ��ǰ�Ӳ��\\U��\\CD��������ק�������������ǲ��е�!");
			SetButtonFalse();
		}		
		else if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0x10)
		{
			FileInfo = _T("���ܶ��ļ��н��в���!");
			SetButtonFalse();
		}
		else if (FindFileData.nFileSizeLow == 0 && FindFileData.nFileSizeHigh == 0)
		{
			FileInfo = _T("�ļ���СΪ0�ֽڣ��������!");
			SetButtonFalse();
		}
		else
		{
			// �ļ�����
			str_FileName = pName;
			FileInfo += str_FileName;
			FileInfo += _T("\r\n");
			CString str_data;
			// �ļ���С
			// llFileSize = (FindFileData.nFileSizeHigh << 32) + FindFileData.nFileSizeLow; // ��Ϊδ���壬��������������������ת��ΪLONGLONG��Ȼ������
			llFileSize = FindFileData.nFileSizeHigh;
			llFileSize <<= 32;
			llFileSize += FindFileData.nFileSizeLow;
			str_data.Format(_T("�ļ���С��%I64d�ֽ�\r\n"), llFileSize);
			FileInfo += str_data;
			// �ļ�����ʱ��
			// ת��Ϊ����ʱ�䣬�������8Сʱ���
			FileTimeToLocalFileTime(&FindFileData.ftLastWriteTime, &loctime);
			FileTimeToSystemTime(&loctime, &mysystime);
			str_data.Format(_T("����޸�ʱ�䣺%d��%d��%d�� %d:%02d:%02d"), mysystime.wYear, mysystime.wMonth, 
				mysystime.wDay, mysystime.wHour, mysystime.wMinute, mysystime.wSecond);
			FileInfo += str_data;
			GetDlgItem(IDC_ENCRYPT)->EnableWindow(TRUE);
			GetDlgItem(IDC_DECRYPT)->EnableWindow(TRUE);
		}
 		delete[] pName;
		pName = NULL;
		DragFinish(hDropInfo);
	}
	else
	{
		FileInfo = _T("������ֻ֧�ֵ����ļ���ק����!");
	}	
	SetDlgItemText(IDC_FILE_INFORMATION, FileInfo);
	
	CDialogEx::OnDropFiles(hDropInfo);
}


void CFileEncryptDlg::OnBnClickedEncrypt()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(TRUE);
	if (password1.GetLength() < 6)
	{
		MessageBox(_T("���볤������Ϊ6λ����!"));
		password1 = password2 = "";
		return;
	}
	else if (password2 != password1)
	{
		MessageBox(_T("�����������벻ƥ�䣬���������룬��!"));
		password1 = password2 = "";
		return;
	}
	GetDlgItem(IDC_ENCRYPT)->EnableWindow(FALSE);
	GetDlgItem(IDC_DECRYPT)->EnableWindow(FALSE);
	GetDlgItem(IDC_STOP)->EnableWindow();
	SetDlgItemText(IDC_ENCRYPT, _T("���ڼ���..."));
	static PARAM param;
	param.fileaddress = str_FileName;
	param.password = password1;
	thread_ok = 0;
	isTerminal = 0;
	funcerror = 0;
	hThread = CreateThread(NULL, 0, Thread_Encrypt, &param, 0, NULL);
	SetTimer(100, 100, NULL);
	UpdateData(FALSE);
}


void CFileEncryptDlg::OnBnClickedDecrypt()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(TRUE);
	if (password1.GetLength() < 6)
	{
		MessageBox(_T("���볤������Ϊ6λ����!"));
		password1 = password2 = "";
	}
// 	else if (password2 != password1)
// 	{
// 		MessageBox(_T("�����������벻ƥ�䣬���������룬��!"));
// 		password1 = password2 = "";
// 	}
	GetDlgItem(IDC_ENCRYPT)->EnableWindow(FALSE);
	GetDlgItem(IDC_DECRYPT)->EnableWindow(FALSE);
	GetDlgItem(IDC_STOP)->EnableWindow();
	SetDlgItemText(IDC_DECRYPT, _T("���ڼ���..."));
	static PARAM param;
	param.fileaddress = str_FileName;
	param.password = password1;
	thread_ok = 0;
	isTerminal = 0;
	funcerror = 0;
	hThread = CreateThread(NULL, 0, Thread_Decrypt, &param, 0, NULL);
	SetTimer(101, 100, NULL);
	UpdateData(FALSE);
}


void CFileEncryptDlg::OnBnClickedStop()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	isTerminal = 1;
}

void CFileEncryptDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	switch (nIDEvent)
	{
	case 100:
		if (thread_ok == 1 || isTerminal)
		{
			KillTimer(100);
			SetDlgItemText(IDC_ENCRYPT, _T("����"));
			GetDlgItem(IDC_ENCRYPT)->EnableWindow();
			GetDlgItem(IDC_DECRYPT)->EnableWindow();
			GetDlgItem(IDC_STOP)->EnableWindow(FALSE);
			if (funcerror == -1)
			{
				break;
			}
			if (isTerminal)
			{
				MessageBox(_T("��ֹ����!"));
			}
			else
			{
				MessageBox(_T("�������!"));
			}
		}
		break;
	case 101:
		if (thread_ok == 1 || isTerminal)
		{
			
			KillTimer(101);
			SetDlgItemText(IDC_DECRYPT, _T("����"));
			GetDlgItem(IDC_ENCRYPT)->EnableWindow();
			GetDlgItem(IDC_DECRYPT)->EnableWindow();
			GetDlgItem(IDC_STOP)->EnableWindow(FALSE);
			if (funcerror == -1)
			{
				break;
			}
			if (isTerminal)
			{
				MessageBox(_T("��ֹ����!"));
			}
			else
			{
				MessageBox(_T("�������!"));
			}
		}
		break;
	default:
		break;
	}

	CDialogEx::OnTimer(nIDEvent);
}
