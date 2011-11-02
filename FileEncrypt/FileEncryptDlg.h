
// FileEncryptDlg.h : 头文件
//

#pragma once


// CFileEncryptDlg 对话框
class CFileEncryptDlg : public CDialogEx
{
// 构造
public:
	CFileEncryptDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_FILEENCRYPT_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
private:
	// 文件信息文本框中默认的提示内容
	CString defaultTips;
	// 文件大小
	LONGLONG llFileSize;
	CString  str_FileName;
public:
	afx_msg void OnDropFiles(HDROP hDropInfo);
private:
	void SetButtonFalse(void)
	{
		GetDlgItem(IDC_STOP)->EnableWindow(FALSE);
		GetDlgItem(IDC_ENCRYPT)->EnableWindow(FALSE);
		GetDlgItem(IDC_DECRYPT)->EnableWindow(FALSE);
	}
public:
	afx_msg void OnBnClickedEncrypt();
	afx_msg void OnBnClickedDecrypt();
	afx_msg void OnBnClickedStop();
	CString password1;
	CString password2;
	// 多线程句柄
	HANDLE hThread;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
