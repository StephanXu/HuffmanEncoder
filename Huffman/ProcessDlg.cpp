// ProcessDlg.cpp : implementation file
//

#include "pch.h"
#include "Huffman.h"
#include "ProcessDlg.h"
#include "afxdialogex.h"
#include "HuffmanEncoder.h"
#include "Utils.h"

// ProcessDlg dialog

IMPLEMENT_DYNAMIC(ProcessDlg, CDialogEx)

ProcessDlg::ProcessDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_PROCESS_DIALOG, pParent)
{

}

ProcessDlg::~ProcessDlg()
{
}

auto ProcessDlg::ProcessProc(ProcessDlg& dialog,
							 SafeQueue<ProcessUnit>& queue,
							 const bool& cancellationFlag)
{
	ProcessUnit unit;
	short count = 0;
	dialog.m_ProgressBar.SetRange(0, static_cast<short>(queue.Size()));
	while (!queue.Empty() && !cancellationFlag)
	{
		queue.WaitAndPop(unit, cancellationFlag);
		auto index = ProcessDlg::FindListItem(dialog.m_ProcessList, 1, CString(unit.Source().c_str()));
		dialog.m_ProcessList.SetItemText(index, 4, _T("Processing..."));
		dialog.m_TipText.SetWindowText(CString("Processing: ") + CString(GetFilenameFromPath(unit.Source()).c_str()));

		if (unit.m_IsEncode)
		{
			HuffmanEncoder::Encode(unit.Source(), unit.Destination());
			auto sourceLength = GetFileLength(unit.Source());
			auto destLength = GetFileLength(unit.Destination());
			auto compRatio = (destLength * 100) / sourceLength;

			CString ratio;
			ratio.Format(_T("%d%%"), compRatio);
			dialog.m_ProcessList.SetItemText(index, 3, ratio);
			dialog.m_ProcessList.SetItemText(index, 4, _T("Encoded"));
		}
		else
		{
			auto digest = HuffmanEncoder::Decode(unit.Source(), unit.Destination());
			auto verify = HuffmanEncoder::Verify(unit.Destination(), digest);
			dialog.m_ProcessList.SetItemText(index, 3, _T("-"));
			dialog.m_ProcessList.SetItemText(index, 4, verify ? _T("Decoded") : _T("Verify failed"));
		}
		dialog.m_ProgressBar.SetPos(++count);
	}
	dialog.m_TipText.SetWindowText(_T("Done"));
	dialog.m_BtnCancel.SetWindowText(_T("Done"));
}

auto ProcessDlg::StartProcess() -> void
{
	if (m_Thread)
	{
		return;
	}
	m_Thread.reset(new std::thread(
		[this]() {
			ProcessProc(*this, m_Queue, m_CancellationFlag);
		}));
}

auto ProcessDlg::GetFileLength(const std::string& filename) -> std::streampos
{
	std::ifstream f{ filename, std::ios::in };
	f.seekg(0, std::ios::end);
	return f.tellg();
}

void ProcessDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROCESS_LIST, m_ProcessList);
	DDX_Control(pDX, IDC_CANCEL, m_BtnCancel);
	DDX_Control(pDX, IDC_PROGRESS1, m_ProgressBar);
	DDX_Control(pDX, IDC_TIPS, m_TipText);
}


BEGIN_MESSAGE_MAP(ProcessDlg, CDialogEx)
	ON_BN_CLICKED(IDC_CANCEL, &ProcessDlg::OnBnClickedCancel)
	ON_WM_NCDESTROY()
	ON_WM_CLOSE()
	ON_MESSAGE(WM_SAFE_DESTORY, OnSafeDestroy)
END_MESSAGE_MAP()


BOOL ProcessDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_ProcessList.InsertColumn(0, _T("Name"), LVCFMT_LEFT, 90);
	m_ProcessList.InsertColumn(1, _T("File Address"), LVCFMT_LEFT, 90);
	m_ProcessList.InsertColumn(2, _T("Destination"), LVCFMT_LEFT, 90);
	m_ProcessList.InsertColumn(3, _T("Compression Ratio"), LVCFMT_LEFT, 90);
	m_ProcessList.InsertColumn(4, _T("Status"), LVCFMT_LEFT, 90);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// ProcessDlg message handlers



void ProcessDlg::OnBnClickedCancel()
{
	SendMessage(WM_CLOSE, 0, 0);
}

LRESULT ProcessDlg::OnSafeDestroy(WPARAM wParam, LPARAM lParam)
{
	DestroyWindow();
	return 0;
}

void ProcessDlg::PostNcDestroy()
{
	CDialog::PostNcDestroy();
	delete this;
}

void ProcessDlg::OnClose()
{
	m_BtnCancel.EnableWindow(FALSE);
	m_TipText.SetWindowText(_T("Canceling"));

	std::thread(
		[this]() {
			m_CancellationFlag = true;
			m_Thread->join();
			m_Thread = nullptr;

			PostMessage(WM_SAFE_DESTORY, 0, 0);
		}).detach();
}