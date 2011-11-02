
// FileEncryptDlg.cpp : 实现文件
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
// CFileEncryptDlg 对话框
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


// CFileEncryptDlg 消息处理程序
BOOL CFileEncryptDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	defaultTips = _T("请直接拖放要加密的文件到本窗口即可\r\n加密文件前请输入密码,解密文件无需确认密码\r\n密码应该为6-20位的字符");
	SetDlgItemText(IDC_FILE_INFORMATION, defaultTips);
	SetButtonFalse();
	DragAcceptFiles(TRUE);
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CFileEncryptDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CFileEncryptDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CFileEncryptDlg::OnDropFiles(HDROP hDropInfo)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
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
			FileInfo = _T("是不是把硬盘\\U盘\\CD驱动器拖拽进来啦，这样是不行滴!");
			SetButtonFalse();
		}		
		else if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0x10)
		{
			FileInfo = _T("不能对文件夹进行操作!");
			SetButtonFalse();
		}
		else if (FindFileData.nFileSizeLow == 0 && FindFileData.nFileSizeHigh == 0)
		{
			FileInfo = _T("文件大小为0字节，无需操作!");
			SetButtonFalse();
		}
		else
		{
			// 文件名称
			str_FileName = pName;
			FileInfo += str_FileName;
			FileInfo += _T("\r\n");
			CString str_data;
			// 文件大小
			// llFileSize = (FindFileData.nFileSizeHigh << 32) + FindFileData.nFileSizeLow; // 行为未定义，但是运行正常，会首先转换为LONGLONG，然后左移
			llFileSize = FindFileData.nFileSizeHigh;
			llFileSize <<= 32;
			llFileSize += FindFileData.nFileSizeLow;
			str_data.Format(_T("文件大小：%I64d字节\r\n"), llFileSize);
			FileInfo += str_data;
			// 文件创建时间
			// 转化为本地时间，否则会有8小时误差
			FileTimeToLocalFileTime(&FindFileData.ftLastWriteTime, &loctime);
			FileTimeToSystemTime(&loctime, &mysystime);
			str_data.Format(_T("最后修改时间：%d年%d月%d日 %d:%02d:%02d"), mysystime.wYear, mysystime.wMonth, 
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
		FileInfo = _T("本程序只支持单个文件拖拽加密!");
	}	
	SetDlgItemText(IDC_FILE_INFORMATION, FileInfo);
	
	CDialogEx::OnDropFiles(hDropInfo);
}


void CFileEncryptDlg::OnBnClickedEncrypt()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	if (password1.GetLength() < 6)
	{
		MessageBox(_T("密码长度至少为6位，亲!"));
		password1 = password2 = "";
		return;
	}
	else if (password2 != password1)
	{
		MessageBox(_T("两次输入密码不匹配，请重新输入，亲!"));
		password1 = password2 = "";
		return;
	}
	GetDlgItem(IDC_ENCRYPT)->EnableWindow(FALSE);
	GetDlgItem(IDC_DECRYPT)->EnableWindow(FALSE);
	GetDlgItem(IDC_STOP)->EnableWindow();
	SetDlgItemText(IDC_ENCRYPT, _T("正在计算..."));
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
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	if (password1.GetLength() < 6)
	{
		MessageBox(_T("密码长度至少为6位，亲!"));
		password1 = password2 = "";
	}
// 	else if (password2 != password1)
// 	{
// 		MessageBox(_T("两次输入密码不匹配，请重新输入，亲!"));
// 		password1 = password2 = "";
// 	}
	GetDlgItem(IDC_ENCRYPT)->EnableWindow(FALSE);
	GetDlgItem(IDC_DECRYPT)->EnableWindow(FALSE);
	GetDlgItem(IDC_STOP)->EnableWindow();
	SetDlgItemText(IDC_DECRYPT, _T("正在计算..."));
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
	// TODO: 在此添加控件通知处理程序代码
	isTerminal = 1;
}

void CFileEncryptDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	switch (nIDEvent)
	{
	case 100:
		if (thread_ok == 1 || isTerminal)
		{
			KillTimer(100);
			SetDlgItemText(IDC_ENCRYPT, _T("加密"));
			GetDlgItem(IDC_ENCRYPT)->EnableWindow();
			GetDlgItem(IDC_DECRYPT)->EnableWindow();
			GetDlgItem(IDC_STOP)->EnableWindow(FALSE);
			if (funcerror == -1)
			{
				break;
			}
			if (isTerminal)
			{
				MessageBox(_T("终止加密!"));
			}
			else
			{
				MessageBox(_T("加密完成!"));
			}
		}
		break;
	case 101:
		if (thread_ok == 1 || isTerminal)
		{
			
			KillTimer(101);
			SetDlgItemText(IDC_DECRYPT, _T("解密"));
			GetDlgItem(IDC_ENCRYPT)->EnableWindow();
			GetDlgItem(IDC_DECRYPT)->EnableWindow();
			GetDlgItem(IDC_STOP)->EnableWindow(FALSE);
			if (funcerror == -1)
			{
				break;
			}
			if (isTerminal)
			{
				MessageBox(_T("终止解密!"));
			}
			else
			{
				MessageBox(_T("解密完成!"));
			}
		}
		break;
	default:
		break;
	}

	CDialogEx::OnTimer(nIDEvent);
}
