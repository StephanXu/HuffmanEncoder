// ProcessDlg.cpp : implementation file
//

#include "pch.h"
#include "Huffman.h"
#include "ProcessDlg.h"
#include "afxdialogex.h"
#include "HuffmanEncoder.h"
#include "Utils.h"
#include "FileDetailDlg.h"
#include "Sha256.h"

#include <vector>
#include <tuple>

// ProcessDlg dialog

IMPLEMENT_DYNAMIC(ProcessDlg, CDialogEx)

const int kFilename               = 0;
const int kSourceColumn           = 1;
const int kDestColumn             = 2;
const int kCompressionRatioColumn = 3;
const int kStatusColumn           = 4;

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
		dialog.m_ProcessList.SetItemText(index, kStatusColumn, _T("Processing..."));
		dialog.m_TipText.SetWindowText(CString("Processing: ") + CString(GetFilenameFromPath(unit.Source()).c_str()));

		if (unit.m_IsEncode)
		{
			HuffmanEncoder::Encode(unit.Source(), unit.Destination());
			auto sourceLength = GetFileLength(unit.Source());
			auto destLength   = GetFileLength(unit.Destination());
			auto compRatio    = (destLength * 100) / sourceLength;

			CString ratio;
			ratio.Format(_T("%d%%"), compRatio);
			dialog.m_ProcessList.SetItemText(index, kCompressionRatioColumn, ratio);
			dialog.m_ProcessList.SetItemText(index, kStatusColumn, _T("Encoded"));
		}
		else
		{
			auto digest = HuffmanEncoder::Decode(unit.Source(), unit.Destination());
			auto verify = HuffmanEncoder::Verify(unit.Destination(), digest);
			dialog.m_ProcessList.SetItemText(index, kCompressionRatioColumn, _T("-"));
			dialog.m_ProcessList.SetItemText(index, kStatusColumn, verify ? _T("Decoded") : _T("Verify failed"));
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
		[this]()
		{
			ProcessProc(*this, m_Queue, m_CancellationFlag);
		}));
}

auto ProcessDlg::GetFileLength(const std::string& filename) -> std::streampos
{
	std::ifstream f{filename, std::ios::in};
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
	ON_BN_CLICKED(IDC_Detail, &ProcessDlg::OnBnClickedDetail)
END_MESSAGE_MAP()


BOOL ProcessDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_ProcessList.InsertColumn(kFilename, _T("Name"), LVCFMT_LEFT, 90);
	m_ProcessList.InsertColumn(kSourceColumn, _T("File Address"), LVCFMT_LEFT, 90);
	m_ProcessList.InsertColumn(kDestColumn, _T("Destination"), LVCFMT_LEFT, 90);
	m_ProcessList.InsertColumn(kCompressionRatioColumn, _T("Compression Ratio"), LVCFMT_LEFT, 90);
	m_ProcessList.InsertColumn(kStatusColumn, _T("Status"), LVCFMT_LEFT, 90);

	return TRUE; // return TRUE  unless you set the focus to a control
}

// ProcessDlg message handlers


void ProcessDlg::OnOK()
{
	return;
}

void ProcessDlg::OnCancel()
{
	return;
}

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
		[this]()
		{
			m_CancellationFlag = true;
			m_Thread->join();
			m_Thread = nullptr;

			PostMessage(WM_SAFE_DESTORY, 0, 0);
		}).detach();
}

void ProcessDlg::OnBnClickedDetail()
{
	auto pos = m_ProcessList.GetFirstSelectedItemPosition();
	if (!pos)
	{
		MessageBox(_T("Please select a file"));
		return;
	}
	while (pos)
	{
		int index = m_ProcessList.GetNextSelectedItem(pos);

		auto csSource = m_ProcessList.GetItemText(index, kSourceColumn);
		auto csDest   = m_ProcessList.GetItemText(index, kDestColumn);
		auto csStatus = m_ProcessList.GetItemText(index, kStatusColumn);
		auto isEncode = 0 == csStatus.Compare(_T("Encoded"));

		if (0 != csStatus.Compare(_T("Encoded")) && 0 != csStatus.Compare(_T("Decoded")))
		{
			MessageBox(_T("The process have not finished yet."));
			return;
		}

		std::vector<std::tuple<std::string, std::string>> details;
		details.push_back({"Filename", csSource.GetString()});
		auto [metaData, huffmanTable] = HuffmanEncoder::GetMetaData(isEncode
			                                                            ? csDest.GetString()
			                                                            : csSource.GetString());
		details.push_back({
			"SHA256",
			picosha2::bytes_to_hex_string(metaData.m_FileHash, metaData.m_FileHash + sizeof(metaData.m_FileHash))
		});
		for (size_t i{}; i < 3; ++i)details.push_back({"", ""});
		details.push_back({"Character", "Encode"});
		for (const auto& item : huffmanTable)
		{
			std::stringstream ss;
			std::string strChar;
			std::string strEncode;
			if ((static_cast<int>(item.first) & 0x80) == 0x80)
			{
				ss << "0x" << std::hex << (static_cast<int>(item.first) & 0xff);
			}
			else
			{
				ss << item.first;
			}
			strChar = ss.str();
			ss.str("");
			auto [bitLength, encode] = item.second;
			for (int i{}; i < static_cast<int>(bitLength); ++i)
			{
				ss << ((encode & (0 | (1 << i))) >> i);
			}
			strEncode = ss.str();

			details.push_back({strChar, strEncode});
		}

		auto dialog = new FileDetailDlg();
		dialog->Create(IDD_FILE_DETAIL);
		dialog->SetFileDetail(details);
		dialog->SetWindowText(("File Detail: " + GetFilenameFromPath(csSource.GetString())).c_str());
		dialog->ShowWindow(SW_SHOW);
	}
}
