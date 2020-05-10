// HuffmanDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "Huffman.h"
#include "HuffmanDlg.h"
#include "afxdialogex.h"
#include "HuffmanEncoder.h"
#include "ProcessDlg.h"

#include <string>
#include <vector>
#include <sstream>
#include <filesystem>
#include <thread>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CHuffmanDlg dialog


CHuffmanDlg::CHuffmanDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_HUFFMAN_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CHuffmanDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CHuffmanDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_DROPFILES()
	ON_WM_QUERYDRAGICON()
END_MESSAGE_MAP()


// CHuffmanDlg message handlers

BOOL CHuffmanDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);  // Set big icon
	SetIcon(m_hIcon, FALSE); // Set small icon

	// TODO: Add extra initialization here

	return TRUE; // return TRUE  unless you set the focus to a control
}

void CHuffmanDlg::OnOK()
{
	
	return;
}

void CHuffmanDlg::OnCancel()
{
	CDialog::OnCancel();
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CHuffmanDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CHuffmanDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CHuffmanDlg::OnDropFiles(HDROP hDropInfo)
{
	std::vector<std::string> vwcFileName;
	auto DropCount = DragQueryFile(hDropInfo, -1, NULL, 0);

	for (unsigned int i = 0; i < DropCount; i++)
	{
		TCHAR wcStr[MAX_PATH];
		DragQueryFile(hDropInfo, i, wcStr, MAX_PATH);
		std::string fileName{wcStr};
		if (std::filesystem::is_directory(wcStr))
		{
			for (const auto& item : std::filesystem::recursive_directory_iterator(fileName))
			{
				if (!item.is_directory())
				{
					vwcFileName.push_back(item.path().string());
				}
			}
		}
		else
		{
			vwcFileName.push_back(fileName);
		}
	}
	std::vector<ProcessUnit> tasks;

	const std::string defaultPrefix{".huff"};
	for (auto&& item : vwcFileName)
	{
		const auto isEncode = !(0 == item.substr(item.rfind('.'), 256).compare(defaultPrefix));
		auto targetFile     = isEncode ? item + ".huff" : item.substr(0, item.length() - defaultPrefix.length());

		if (std::filesystem::exists(targetFile))
		{
			const auto result = MessageBox(
				("The file \"" + targetFile + "\" is exists. Would you like to override it?").c_str(),
				"Collision warning",
				MB_YESNO);
			if (result == IDNO)
			{
				continue;
			}
		}
		tasks.push_back({isEncode, item, targetFile});
	}

	if (tasks.size() > 0)
	{
		theApp.AddProcess(tasks);
	}

	DragFinish(hDropInfo);
	CDialog::OnDropFiles(hDropInfo);
}
