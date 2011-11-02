
// FileEncryptDlg.h : ͷ�ļ�
//

#pragma once


// CFileEncryptDlg �Ի���
class CFileEncryptDlg : public CDialogEx
{
// ����
public:
	CFileEncryptDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_FILEENCRYPT_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
private:
	// �ļ���Ϣ�ı�����Ĭ�ϵ���ʾ����
	CString defaultTips;
	// �ļ���С
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
	// ���߳̾��
	HANDLE hThread;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
