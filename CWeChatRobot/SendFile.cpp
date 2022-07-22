#include "pch.h"

struct FileParamStruct {
    DWORD wxid;
    DWORD filepath;
};

int SendFile(DWORD pid,wchar_t* wxid, wchar_t* filepath) {
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!hProcess)
        return 1;
    DWORD WeChatRobotBase = GetWeChatRobotBase(pid);
    if (!WeChatRobotBase) {
        CloseHandle(hProcess);
        return 1;
    }
    DWORD dwId = 0;
    DWORD dwWriteSize = 0;
    FileParamStruct params;
    ZeroMemory(&params, sizeof(params));
    LPVOID wxidaddr = VirtualAllocEx(hProcess, NULL, 1, MEM_COMMIT, PAGE_READWRITE);
    LPVOID filepathaddr = VirtualAllocEx(hProcess, NULL, 1, MEM_COMMIT, PAGE_READWRITE);
    FileParamStruct* paramAndFunc = (FileParamStruct*)::VirtualAllocEx(hProcess, 0, sizeof(FileParamStruct), MEM_COMMIT, PAGE_READWRITE);
    if (!wxidaddr || !filepathaddr || !paramAndFunc || !WeChatRobotBase) {
        CloseHandle(hProcess);
        return 1;
    }
    DWORD dwTId = 0;

    if (wxidaddr)
        WriteProcessMemory(hProcess, wxidaddr, wxid, wcslen(wxid) * 2 + 2, &dwWriteSize);

    if (filepathaddr)
        WriteProcessMemory(hProcess, filepathaddr, filepath, wcslen(filepath) * 2 + 2, &dwWriteSize);

    params.wxid = (DWORD)wxidaddr;
    params.filepath = (DWORD)filepathaddr;

    if (paramAndFunc) {
        WriteProcessMemory(hProcess, paramAndFunc, &params, sizeof(params), &dwTId);
    }
    else {
        CloseHandle(hProcess);
        return 1;
    }

    DWORD SendFileRemoteAddr = WeChatRobotBase + SendFileOffset;
    HANDLE hThread = ::CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)SendFileRemoteAddr, (LPVOID)paramAndFunc, 0, &dwId);
    if (hThread) {
        WaitForSingleObject(hThread, INFINITE);
    }
    else {
        CloseHandle(hProcess);
        return 1;
    }
    CloseHandle(hThread);
    VirtualFreeEx(hProcess, wxidaddr, 0, MEM_RELEASE);
    VirtualFreeEx(hProcess, filepathaddr, 0, MEM_RELEASE);
    VirtualFreeEx(hProcess, paramAndFunc, 0, MEM_RELEASE);
    CloseHandle(hProcess);
    return 0;
}
