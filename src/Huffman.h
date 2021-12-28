
// Huffman.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
#error "include 'pch.h' before including this file for PCH"
#endif

#include <string>
#include <memory>
#include <vector>
#include <thread>

#include "resource.h"		// main symbols
#include "ProcessUnit.h"
#include "ProcessDlg.h"

// CHuffmanApp:
// See Huffman.cpp for the implementation of this class
//

class CHuffmanApp : public CWinApp
{

public:
	CHuffmanApp();

	auto AddProcess(const std::vector<ProcessUnit>& processUnit) -> void
	{
		auto dialog = new ProcessDlg;
		dialog->Create(IDD_PROCESS_DIALOG);
		dialog->AppendProcessQueue(processUnit.begin(), processUnit.end());
		dialog->ShowWindow(SW_SHOW);
		dialog->StartProcess();
	}

	// Overrides
public:
	virtual BOOL InitInstance();

	// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CHuffmanApp theApp;
