#pragma once

#include "ProcessUnit.h"
#include "Queue.h"
#include "Utils.h"

#include <mutex>

// ProcessDlg dialog

#define WM_SAFE_DESTORY (WM_USER+1)

class ProcessDlg : public CDialogEx
{
DECLARE_DYNAMIC(ProcessDlg)
	SafeQueue<ProcessUnit> m_Queue;
	std::shared_ptr<std::thread> m_Thread;
	bool m_CancellationFlag{false};
public:
	ProcessDlg(CWnd* pParent = nullptr); // standard constructor
	virtual ~ProcessDlg();

	template <typename IterType>
	auto AppendProcessQueue(IterType first, IterType last)
	{
		for (auto it = first; it != last; ++it)
		{
			m_Queue.Push(*it);
			CString source{it->Source().c_str()},
			        destination{it->Destination().c_str()},
			        name{GetFilenameFromPath(it->Source()).c_str()};
			auto index = m_ProcessList.InsertItem(0, name);
			m_ProcessList.SetItemText(index, 1, source);
			m_ProcessList.SetItemText(index, 2, destination);
			m_ProcessList.SetItemText(index, 3, _T("-"));
			m_ProcessList.SetItemText(index, 4, _T("Waiting"));
		}
	}

	static auto ProcessProc(ProcessDlg& dialog,
	                        SafeQueue<ProcessUnit>& queue,
	                        const bool& cancellationFlag);

	auto StartProcess() -> void;

	static auto FindListItem(const CListCtrl& list, const int colum, const CString& text) -> int
	{
		for (int i = 0; i < list.GetItemCount(); ++i)
		{
			CString szText = list.GetItemText(i, colum);
			if (szText == text)
			{
				return i;
			}
		}
		return -1;
	}

	static auto GetFileLength(const std::string& filename) -> std::streampos;


	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PROCESS_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support

	virtual BOOL OnInitDialog();
DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_ProcessList;
	afx_msg void OnOK() override;
	afx_msg void OnCancel() override;
	afx_msg void OnBnClickedCancel();
	afx_msg LRESULT OnSafeDestroy(WPARAM wParam, LPARAM lParam);
	afx_msg void PostNcDestroy() override;
	afx_msg void OnClose();
	CButton m_BtnCancel;
	CProgressCtrl m_ProgressBar;
	CStatic m_TipText;
	afx_msg void OnBnClickedDetail();
};
