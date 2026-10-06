#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <commctrl.h>
#include <string>
#include <vector>

#pragma comment(lib, "comctl32.lib")

#define ID_BTN_CHECK_APPS    1001
#define ID_BTN_OPTIMIZE      1002
#define ID_TXT_OUTPUT        1003

// ฟังก์ชันช่วยรันคำสั่ง CMD โดยบังคับให้ Output ออกมาเป็น UTF-8 (chcp 65001)
std::wstring RunCommand(const std::wstring& cmd) {
    std::wstring result = L"";
    HANDLE hRead, hWrite;
    SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };

    if (!CreatePipe(&hRead, &hWrite, &sa, 0)) return L"Error creating pipe";

    STARTUPINFO si = { sizeof(STARTUPINFO) };
    si.cb = sizeof(STARTUPINFO);
    si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    si.wShowWindow = SW_HIDE;
    si.hStdOutput = hWrite;
    si.hStdError = hWrite;

    PROCESS_INFORMATION pi;
    // ใช้ chcp 65001 เพื่อบังคับการแสดงผลเป็น UTF-8 ภาษาไทยไม่ต่างด้าวแน่นอน
    std::wstring fullCmd = L"cmd.exe /c chcp 65001 >nul && " + cmd;

    if (CreateProcess(NULL, &fullCmd[0], NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        CloseHandle(hWrite);
        char buffer[1024];
        DWORD bytesRead;
        while (ReadFile(hRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
            buffer[bytesRead] = '\0';
            int reqLen = MultiByteToWideChar(CP_UTF8, 0, buffer, -1, NULL, 0);
            if (reqLen > 0) {
                std::vector<wchar_t> wbuf(reqLen);
                MultiByteToWideChar(CP_UTF8, 0, buffer, -1, &wbuf[0], reqLen);
                result += &wbuf[0];
            }
        }
        CloseHandle(hRead);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    } else {
        CloseHandle(hWrite);
        CloseHandle(hRead);
    }
    return result;
}

// ตรวจสอบแอปที่จำเป็น
void CheckRequiredApps(HWND hOutput) {
    SetWindowText(hOutput, L"=== กำลังตรวจสอบแอปพลิเคชันที่จำเป็นสำหรับ FiveM... ===\r\n\r\n");

    struct AppCheck {
        std::wstring name;
        std::wstring checkCmd;
    };

    std::vector<AppCheck> apps = {
        { L"Visual C++ Redistributable (2015-2022)", L"reg query \"HKLM\\SOFTWARE\\Microsoft\\VisualStudio\\14.0\\VC\\Runtimes\\x64\" /v Installed 2>nul" },
        { L"DirectX End-User Runtimes", L"dir %windir%\\System32\\d3d9.dll 2>nul" },
        { L"Discord", L"dir \"%localappdata%\\Discord\\app-*\" /b 2>nul" },
        { L"MSI Afterburner", L"dir \"C:\\Program Files (x86)\\MSI Afterburner\\MSIAfterburner.exe\" 2>nul" },
        { L"7-Zip / WinRAR", L"where 7z 2>nul || where winrar 2>nul" }
    };

    std::wstring output = L"";
    for (const auto& app : apps) {
        std::wstring res = RunCommand(app.checkCmd);
        if (!res.empty() && res.find(L"ERROR") == std::wstring::npos) {
            output += L"[✓ Found] " + app.name + L"\r\n";
        } else {
            output += L"[X Missing] " + app.name + L" (แนะนำให้ติดตั้งเพิ่มเติม)\r\n";
        }
    }

    output += L"\r\n--------------------------------------------------\r\n";
    output += L"* หมายเหตุ: หากขาด Visual C++ หรือ DirectX อาจทำให้ FiveM เข้าไม่ได้ หรือ Crash บ่อย\r\n";

    SetWindowText(hOutput, output.c_str());
}

// ปรับแต่ง Windows เพื่อ FPS สูงสุด
void OptimizeWindowsForFiveM(HWND hOutput) {
    SetWindowText(hOutput, L"=== กำลัง Optimize Windows สำหรับ FiveM (High FPS)... ===\r\n\r\n");

    std::vector<std::pair<std::wstring, std::wstring>> optimizations = {
        { L"ปรับ Power Plan เป็น High Performance", L"powercfg /s 8c5e7fda-e8bf-4a96-9a85-a6e23a8c635c" },
        { L"ปิด Xbox Game Bar & Game DVR (ลดอาการกระตุก)", L"reg add \"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\GameDVR\" /v AppCaptureEnabled /t REG_DWORD /d 0 /f" },
        { L"เปิด Hardware-Accelerated GPU Scheduling (HAGS)", L"reg add \"HKLM\\SYSTEM\\CurrentControlSet\\Control\\GraphicsDrivers\" /v HwSchMode /t REG_DWORD /d 2 /f" },
        { L"ปิด Windows Telemetry & Background Tracking", L"sc config DiagTrack start= disabled & sc stop DiagTrack" },
        { L"ลบไฟล์ Temp ขยะในระบบ", L"del /q /f /s \"%temp%\\*\" 2>nul" }
    };

    std::wstring log = L"";
    for (const auto& opt : optimizations) {
        log += L"[Optimizing] " + opt.first + L"...\r\n";
        RunCommand(opt.second);
    }

    log += L"\r\n[SUCCESS] ปรับแต่งระบบเรียบร้อยแล้ว!\r\n";
    log += L"แนะนำให้ Restart คอมพิวเตอร์ 1 รอบเพื่อให้ผลลัพธ์มีประสิทธิภาพสูงสุด\r\n";

    SetWindowText(hOutput, log.c_str());
}

// Window Procedure
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HWND hBtnCheck, hBtnOptimize, hTextOutput;

    switch (uMsg) {
    case WM_CREATE: {
        hBtnCheck = CreateWindow(L"BUTTON", L"1. เช็กแอปที่จำเป็น", 
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            20, 20, 200, 40, hwnd, (HMENU)ID_BTN_CHECK_APPS, NULL, NULL);

        hBtnOptimize = CreateWindow(L"BUTTON", L"2. Optimize FPS FiveM", 
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            240, 20, 200, 40, hwnd, (HMENU)ID_BTN_OPTIMIZE, NULL, NULL);

        hTextOutput = CreateWindow(L"EDIT", L"กดปุ่มด้านบนเพื่อเริ่มการทำงาน...", 
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
            20, 80, 540, 340, hwnd, (HMENU)ID_TXT_OUTPUT, NULL, NULL);

        HFONT hFont = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
        SendMessage(hBtnCheck, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hBtnOptimize, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hTextOutput, WM_SETFONT, (WPARAM)hFont, TRUE);
        break;
    }
    case WM_COMMAND: {
        int wmId = LOWORD(wParam);
        if (wmId == ID_BTN_CHECK_APPS) {
            CheckRequiredApps(hTextOutput);
        } else if (wmId == ID_BTN_OPTIMIZE) {
            OptimizeWindowsForFiveM(hTextOutput);
        }
        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"FiveM_Optimizer_Class";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0, CLASS_NAME, L"FiveM App Checker & FPS Optimizer", 
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 600, 480,
        NULL, NULL, hInstance, NULL
    );

    if (hwnd == NULL) return 0;

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
