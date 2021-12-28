// FileDetailDlg.cpp : implementation file
//

#include "pch.h"
#include "Huffman.h"
#include "FileDetailDlg.h"
#include "afxdialogex.h"


// FileDetailDlg dialog

IMPLEMENT_DYNAMIC(FileDetailDlg, CDialogEx)

FileDetailDlg::FileDetailDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_FILE_DETAIL, pParent)
{
}

FileDetailDlg::~FileDetailDlg()
{
}

auto FileDetailDlg::SetFileDetail(const FileDetailList& details) -> void
{
	m_FileDetail.SetSize(2, static_cast<int>(details.size()));
	m_FileDetail.SetColHeading(0, _T("Item"));
	m_FileDetail.SetColHeading(1, _T("Content"));
	m_FileDetail.SetColWidth(1, 550);
	int count{};
	for (const auto& item : details)
	{
		const auto& [caption, content] = item;
		m_FileDetail.SetCellText(0, count, CString(caption.c_str()));
		m_FileDetail.SetCellText(1, count, CString(content.c_str()));
		++count;
	}
}

void FileDetailDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DETAIL_TABLE, m_FileDetail);
}


BEGIN_MESSAGE_MAP(FileDetailDlg, CDialogEx)
	ON_WM_NCDESTROY()
END_MESSAGE_MAP()


// FileDetailDlg message handlers

void FileDetailDlg::PostNcDestroy()
{
	CDialog::PostNcDestroy();
	delete this;
}
