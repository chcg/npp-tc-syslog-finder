//this file is part of notepad++
//Original Copyright (C)2022 Don HO <don.h@free.fr>
//Modifications Copyright (C)2026 bsagarzazu
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "PluginDefinition.h"
#include "menuCmdID.h"
#include <shlobj.h>
#include <tchar.h>
#include <shellapi.h>

//
// The plugin data that Notepad++ needs
//
FuncItem funcItem[nbFunc];

//
// The data of Notepad++ that you can use in your plugin commands
//
NppData nppData;

ShortcutKey syslogShortcut = { false, true, true, 'L' };
ShortcutKey folderShortcut = { false, true, true, 'F' };
ShortcutKey cleanShortcut = { false, true, true, 'C' };

TCHAR szIniFilePath[MAX_PATH] = {0};
TCHAR szSyslogPath[MAX_PATH] = {0};
TCHAR szSearchPattern[MAX_PATH] = {0};
TCHAR szThreshold[MAX_PATH] = {0};

TCHAR szIniSection[] = TEXT("Settings");
TCHAR szKeyPath[] = TEXT("SyslogPath");
TCHAR szKeyPattern[] = TEXT("SearchPattern");
TCHAR szKeyThreshold[] = TEXT("CleanupThreshold");

void loadConfig() 
{
    if (szIniFilePath[0] == 0) {
        ::SendMessage(nppData._nppHandle, NPPM_GETPLUGINSCONFIGDIR, MAX_PATH, (LPARAM)szIniFilePath);
        CreateDirectory(szIniFilePath, NULL);
        _tcscat_s(szIniFilePath, MAX_PATH, TEXT("\\TCSyslogFinder.ini"));
    }
    
    GetPrivateProfileString(szIniSection, szKeyPath, TEXT(""), szSyslogPath, MAX_PATH, szIniFilePath);
    GetPrivateProfileString(szIniSection, szKeyPattern, TEXT("tcserver*.syslog"), szSearchPattern, MAX_PATH, szIniFilePath);
    GetPrivateProfileString(szIniSection, szKeyThreshold, TEXT("7"), szThreshold, MAX_PATH, szIniFilePath);

    if (GetFileAttributes(szIniFilePath) == INVALID_FILE_ATTRIBUTES) {
        saveConfigValue(szKeyPath, szSyslogPath);
        saveConfigValue(szKeyPattern, szSearchPattern);
        saveConfigValue(szKeyThreshold, szThreshold);
    }
}

void saveConfigValue(const TCHAR* szKey, const TCHAR* szValue) 
{
    WritePrivateProfileString(szIniSection, szKey, szValue, szIniFilePath);
}

void CALLBACK timerToScroll(HWND /*hWnd*/, UINT /*uMsg*/, UINT_PTR idEvent, DWORD /*dwTime*/)
{
    ::KillTimer(NULL, idEvent);

    int which = -1;
    ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&which);
    if (which == -1) return;

    HWND hCurScintilla = (which == 0) ? nppData._scintillaMainHandle : nppData._scintillaSecondHandle;

    // Track the new content
    ::SendMessage(nppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_VIEW_MONITORING);

    // Go to the end of the file
    int nLines = (int)::SendMessage(hCurScintilla, SCI_GETLINECOUNT, 0, 0);
    int nLastLine = nLines - 1;

    if (nLastLine >= 0)
    {
        ::SendMessage(hCurScintilla, SCI_ENSUREVISIBLE, nLastLine, 0);
        ::SendMessage(hCurScintilla, SCI_GOTOLINE, nLastLine, 0);
        ::SendMessage(hCurScintilla, SCI_SCROLLCARET, 0, 0);
    }
}

//
// Initialize your plugin data here
// It will be called while plugin loading   
void pluginInit(HANDLE /*hModule*/)
{
}

//
// Here you can do the clean up, save the parameters (if any) for the next session
//
void pluginCleanUp()
{
}

//
// Initialization of your plugin commands
// You should fill your plugins commands here
void commandMenuInit()
{

    //--------------------------------------------//
    //-- STEP 3. CUSTOMIZE YOUR PLUGIN COMMANDS --//
    //--------------------------------------------//
    // with function :
    // setCommand(int index,                      // zero based number to indicate the order of command
    //            TCHAR *commandName,             // the command name that you want to see in plugin menu
    //            PFUNCPLUGINCMD functionPointer, // the symbol of function (function pointer) associated with this command. The body should be defined below. See Step 4.
    //            ShortcutKey *shortcut,          // optional. Define a shortcut to trigger this command
    //            bool check0nInit                // optional. Make this menu item be checked visually
    //            );
    setCommand(0, TEXT("Open Latest TC Syslog"), openLatestSyslog, &syslogShortcut, false);
    setCommand(1, TEXT("Open Syslog Folder"), openSyslogFolder, &folderShortcut, false);
    setCommand(2, TEXT("Clean Old Syslogs"), cleanOldSyslogs, &cleanShortcut, false);
    setCommand(3, TEXT("---"), NULL, NULL, false);
    setCommand(4, TEXT("Set TC Syslog Path"), setSyslogPath, NULL, false);
    setCommand(5, TEXT("Edit Settings"), openSettings, NULL, false);
    setCommand(6, TEXT("---"), NULL, NULL, false);
    setCommand(7, TEXT("GitHub Repository"), openGitHubRepo, NULL, false);
    setCommand(8, TEXT("About TC Syslog Finder"), showAbout, NULL, false);
}

//
// Here you can do the clean up (especially for the shortcut)
//
void commandMenuCleanUp()
{
	// Don't forget to deallocate your shortcut here
}


//
// This function help you to initialize your plugin commands
//
bool setCommand(size_t index, TCHAR *cmdName, PFUNCPLUGINCMD pFunc, ShortcutKey *sk, bool check0nInit) 
{
    if (index >= nbFunc)
        return false;

    if (!pFunc)
        return false;

    lstrcpy(funcItem[index]._itemName, cmdName);
    funcItem[index]._pFunc = pFunc;
    funcItem[index]._init2Check = check0nInit;
    funcItem[index]._pShKey = sk;

    return true;
}

//----------------------------------------------//
//-- STEP 4. DEFINE YOUR ASSOCIATED FUNCTIONS --//
//----------------------------------------------//
void openLatestSyslog()
{
    // Load the latest configuration
    loadConfig();

    // Read TC Syslog Path from settings
    if (szSyslogPath[0] == 0)
    {
        ::MessageBox(nppData._nppHandle, TEXT("Path not set. Use 'Set TC Syslog Path'."), TEXT("TC Syslog Finder"), MB_OK | MB_ICONWARNING);
        return;
    }

    // Search for the latest syslog file in TC Syslog Path
    TCHAR szSearch[MAX_PATH];
    _stprintf_s(szSearch, MAX_PATH, TEXT("%s\\%s"), szSyslogPath, szSearchPattern);
    
    WIN32_FIND_DATA fd;
    HANDLE hFind = FindFirstFile(szSearch, &fd);
    
    if (hFind == INVALID_HANDLE_VALUE) {
        MessageBox(nppData._nppHandle, TEXT("No syslog files found."), TEXT("TC Syslog Finder"), MB_OK | MB_ICONWARNING);
        return;
    }
    
    FILETIME ftLatest = fd.ftLastWriteTime;
    TCHAR szLatestFile[MAX_PATH];
    _stprintf_s(szLatestFile, MAX_PATH, TEXT("%s\\%s"), szSyslogPath, fd.cFileName);

    while (FindNextFile(hFind, &fd)) {
        if (CompareFileTime(&fd.ftLastWriteTime, &ftLatest) > 0) {
            ftLatest = fd.ftLastWriteTime;
            _stprintf_s(szLatestFile, MAX_PATH, TEXT("%s\\%s"), szSyslogPath, fd.cFileName);
        }
    }
    FindClose(hFind);  
    
    // Open the latest syslog file in Notepad++
    if (::SendMessage(nppData._nppHandle, NPPM_DOOPEN, 0, (LPARAM)szLatestFile)) 
    {
        ::SetTimer(NULL, 0, 0, (TIMERPROC)timerToScroll);
    }
}

void openSyslogFolder()
{
    // Load the latest configuration
    loadConfig();

    if (szSyslogPath[0] == 0)
    {
        ::MessageBox(nppData._nppHandle, TEXT("Path not set. Use 'Set TC Syslog Path'."), TEXT("TC Syslog Finder"), MB_OK | MB_ICONWARNING);
        return;
    }
    else
    {
        ShellExecute(NULL, TEXT("explore"), szSyslogPath, NULL, NULL, SW_SHOWNORMAL);
    }
}

void cleanOldSyslogs()
{
    // Load the latest configuration
    loadConfig();

    if (szSyslogPath[0] == 0)
    {
        ::MessageBox(nppData._nppHandle, TEXT("Path not set. Use 'Set TC Syslog Path'."), TEXT("TC Syslog Finder"), MB_OK | MB_ICONWARNING);
        return;
    }

    int iThreshold = _ttoi(szThreshold);
    if (iThreshold <= 0 && _tcscmp(szThreshold, TEXT("0")) != 0)
    {
        iThreshold = 7;
    }

    TCHAR szMessage[MAX_PATH];
    _stprintf_s(szMessage, MAX_PATH, TEXT("Delete all syslogs older than %d days?"), iThreshold);
    
    if (::MessageBox(nppData._nppHandle, szMessage, TEXT("Clean Logs"), MB_YESNO | MB_ICONQUESTION) == IDYES)
    {
        TCHAR szSearch[MAX_PATH];
        _stprintf_s(szSearch, MAX_PATH, TEXT("%s\\%s"), szSyslogPath, szSearchPattern);

        WIN32_FIND_DATA fd;
        HANDLE hFind = FindFirstFile(szSearch, &fd);

        if (hFind != INVALID_HANDLE_VALUE) {
            FILETIME ftNow;
            GetSystemTimeAsFileTime(&ftNow);

            ULONGLONG thresholdDays = (ULONGLONG)iThreshold * 24ULL * 60ULL * 60ULL * 1000ULL * 1000ULL * 10ULL;

            do {
                ULARGE_INTEGER uiLastWrite;
                uiLastWrite.LowPart = fd.ftLastWriteTime.dwLowDateTime;
                uiLastWrite.HighPart = fd.ftLastWriteTime.dwHighDateTime;

                ULARGE_INTEGER uiNow;
                uiNow.LowPart = ftNow.dwLowDateTime;
                uiNow.HighPart = ftNow.dwHighDateTime;

                if (uiNow.QuadPart - uiLastWrite.QuadPart > thresholdDays) {
                    TCHAR szFileToDelete[MAX_PATH];
                    _stprintf_s(szFileToDelete, MAX_PATH, TEXT("%s\\%s"), szSyslogPath, fd.cFileName);
                    DeleteFile(szFileToDelete);
                }
            } while (FindNextFile(hFind, &fd));
            FindClose(hFind);

            MessageBox(nppData._nppHandle, TEXT("Old logs cleaned."), TEXT("Success"), MB_OK);
        }
    }
}

void setSyslogPath()
{
    // Load the latest configuration
    loadConfig();

    // Open a folder selection dialog to set TC Syslog Path
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr))
    {
        IFileDialog *pFileDialog = NULL;
        hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileDialog, (void**)&pFileDialog);
        if (SUCCEEDED(hr))
        {
            DWORD dwOptions;
            pFileDialog->GetOptions(&dwOptions);
            pFileDialog->SetOptions(dwOptions | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM);
            pFileDialog->SetTitle(L"Select Teamcenter syslog directory");

            if (SUCCEEDED(pFileDialog->Show(nppData._nppHandle)))
            {
                IShellItem *pItem;
                if (SUCCEEDED(pFileDialog->GetResult(&pItem)))
                {
                    PWSTR pszFilePath = NULL;
                    if (SUCCEEDED(pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath)))
                    {
                        _tcscpy_s(szSyslogPath, MAX_PATH, pszFilePath);
                        saveConfigValue(TEXT("SyslogPath"), szSyslogPath);
                        CoTaskMemFree(pszFilePath);
                    }
                    pItem->Release();
                }
            }
            pFileDialog->Release();
        }
        CoUninitialize();
    }
}

void openSettings()
{
    // Load the latest configuration
    loadConfig();
    
    // Open the configuration file in Notepad++ to allow user to edit settings
    ::SendMessage(nppData._nppHandle, NPPM_DOOPEN, 0, (LPARAM)szIniFilePath);
    ::MessageBox(nppData._nppHandle, 
        TEXT("Configuration file opened.\n\n"
             "Update the settings and save the file (Ctrl+S).\n"
             "Changes will apply immediately on your next search."), 
        TEXT("TC Syslog Finder"), MB_OK | MB_ICONINFORMATION);
}

void openGitHubRepo()
{
    // Open the GitHub repository in the default web browser
    ShellExecute(NULL, TEXT("open"), TEXT("https://github.com/bsagarzazu/npp-tc-syslog-finder"), NULL, NULL, SW_SHOWNORMAL);
}

void showAbout()
{
    TCHAR szAbout[1024];
    _stprintf_s(szAbout, 1024,
        TEXT("TC Syslog Finder v1.0.0\n\n")
        TEXT("Instantly find and open the latest Teamcenter syslog in Notepad++.\n\n")
        TEXT("Author: bsagarzazu\n")
        TEXT("License: GPL-2.0")
    );

    ::MessageBox(nppData._nppHandle, szAbout, TEXT("About TC Syslog Finder"), MB_OK | MB_ICONINFORMATION);
}