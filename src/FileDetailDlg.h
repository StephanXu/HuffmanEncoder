#pragma once

#include "MultilineList.h"

#include <map>

// FileDetailDlg dialog

using FileDetailList = std::vector<std::tuple<std::string, std::string>>;

class FileDetailDlg : public CDialogEx
{
DECLARE_DYNAMIC(FileDetailDlg)

public:
	FileDetailDlg(CWnd* pParent = nullptr); // standard constructor
	virtual ~FileDetailDlg();

	auto SetFileDetail(const FileDetailList& details) -> void;

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FILE_DETAIL };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support

	afx_msg void PostNcDestroy();

DECLARE_MESSAGE_MAP()
public:
	CMultilineList m_FileDetail;
};
