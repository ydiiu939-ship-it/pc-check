#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <d2d1.h>
#include <dwrite.h>
#include <string>
#include <vector>
#include <windowsx.h>

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

// Direct2D Interfaces
ID2D1Factory*           pD2DFactory = NULL;
ID2D1HwndRenderTarget*  pRenderTarget = NULL;
IDWriteFactory*         pDWriteFactory = NULL;

// Brushes & Text Formats
ID2D1SolidColorBrush* pBrushBg = NULL;
ID2D1SolidColorBrush* pBrushCard = NULL;
ID2D1SolidColorBrush* pBrushTitleBar = NULL;
ID2D1SolidColorBrush* pBrushTextMain = NULL;
ID2D1SolidColorBrush* pBrushTextSub = NULL;
ID2D1SolidColorBrush* pBrushAccent = NULL;
ID2D1SolidColorBrush* pBrushButtonNormal = NULL;
ID2D1SolidColorBrush* pBrushButtonHover = NULL;
ID2D1SolidColorBrush* pBrushCloseHover = NULL;

IDWriteTextFormat* pFontTitle = NULL;
IDWriteTextFormat* pFontHeading = NULL;
IDWriteTextFormat* pFontBody = NULL;
IDWriteTextFormat* pFontButton = NULL;

// State Variables
bool isHoverBtnCheck = false;
bool isHoverBtnOpt = false;
bool isHoverClose = false;

std::vector<std::wstring> logOutput = { L"Ready. Click a button to start process..." };
bool showCustomAlert = false;
std::wstring alertTitle = L"";
std::wstring alertMsg = L"";

// Utility: Run Command Silent
std::wstring RunCommand(const std::wstring& cmd) {
    std::wstring result = L"";
    HANDLE hRead, hWrite;
    SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };

    if (!CreatePipe(&hRead, &hWrite, &sa, 0)) return L"";

    STARTUPINFO si = { sizeof(STARTUPINFO) };
    si.cb = sizeof(STARTUPINFO);
    si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    si.wShowWindow = SW_HIDE;
    si.hStdOutput = hWrite;
    si.hStdError = hWrite;

    PROCESS_INFORMATION pi;
    std::wstring fullCmd = L"cmd.exe /c " + cmd;

    if (CreateProcess(NULL, &fullCmd[0], NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        CloseHandle(hWrite);
        char buffer[512];
        DWORD bytesRead;
        while (ReadFile(hRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
            buffer[bytesRead] = '\0';
            int reqLen = MultiByteToWideChar(CP_OEMCP, 0, buffer, -1, NULL, 0);
            if (reqLen > 0) {
                std::vector<wchar_t> wbuf(reqLen);
                MultiByteToWideChar(CP_OEMCP, 0, buffer, -1, &wbuf[0], reqLen);
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

// Check Required Apps
void CheckRequiredApps() {
    logOutput.clear();
    logOutput.push_back(L"=== CHECKING REQUIRED APPLICATIONS ===");
    logOutput.push_back(L"");

    struct AppCheck {
        std::wstring name;
        std::wstring cmd;
    };

    std::vector<AppCheck> apps = {
        { L"Visual C++ Redistributable (2015-2022)", L"reg query \"HKLM\\SOFTWARE\\Microsoft\\VisualStudio\\14.0\\VC\\Runtimes\\x64\" /v Installed 2>nul" },
        { L"DirectX End-User Runtimes", L"dir %windir%\\System32\\d3d9.dll 2>nul" },
        { L"Discord Desktop App", L"dir \"%localappdata%\\Discord\\app-*\" /b 2>nul" },
        { L"MSI Afterburner", L"dir \"C:\\Program Files (x86)\\MSI Afterburner\\MSIAfterburner.exe\" 2>nul" },
        { L"7-Zip / WinRAR Archiver", L"where 7z 2>nul || where winrar 2>nul" }
    };

    int missingCount = 0;
    for (const auto& app : apps) {
        std::wstring res = RunCommand(app.cmd);
        if (!res.empty() && res.find(L"ERROR") == std::wstring::npos) {
            logOutput.push_back(L"[ INSTALLED ]  " + app.name);
        } else {
            logOutput.push_back(L"[ MISSING ]    " + app.name);
            missingCount++;
        }
    }

    logOutput.push_back(L"");
    logOutput.push_back(L"Scan completed.");

    // Trigger Custom Alert
    alertTitle = L"App Verification Complete";
    if (missingCount == 0) {
        alertMsg = L"All essential applications are installed and ready!";
    } else {
        alertMsg = L"Found " + std::to_wstring(missingCount) + L" missing app(s). Please check details in the panel.";
    }
    showCustomAlert = true;
}

// Windows FPS Optimization
void OptimizeWindows() {
    logOutput.clear();
    logOutput.push_back(L"=== OPTIMIZING WINDOWS FOR FIVEM ===");
    logOutput.push_back(L"");

    std::vector<std::pair<std::wstring, std::wstring>> opts = {
        { L"Power Plan: Ultimate / High Performance", L"powercfg /s 8c5e7fda-e8bf-4a96-9a85-a6e23a8c635c" },
        { L"Disable Xbox Game Bar & Game DVR", L"reg add \"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\GameDVR\" /v AppCaptureEnabled /t REG_DWORD /d 0 /f" },
        { L"Enable Hardware GPU Scheduling (HAGS)", L"reg add \"HKLM\\SYSTEM\\ControlSet001\\Control\\GraphicsDrivers\" /v HwSchMode /t REG_DWORD /d 2 /f" },
        { L"Disable Telemetry & Tracking Services", L"sc config DiagTrack start= disabled & sc stop DiagTrack" },
        { L"Clear System Temp Files", L"del /q /f /s \"%temp%\\*\" 2>nul" }
    };

    for (const auto& opt : opts) {
        logOutput.push_back(L"[Applying] " + opt.first + L"...");
        RunCommand(opt.second);
    }

    logOutput.push_back(L"");
    logOutput.push_back(L"[SUCCESS] Optimization tweaks applied!");
    logOutput.push_back(L"Restart your PC to get the highest FPS performance.");

    alertTitle = L"Optimization Finished";
    alertMsg = L"Windows FPS tweaks successfully applied!\nPlease restart your computer.";
    showCustomAlert = true;
}

// Create Direct2D Resources
void CreateD2DResources(HWND hwnd) {
    if (!pRenderTarget) {
        RECT rc;
        GetClientRect(hwnd, &rc);

        D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);
        pD2DFactory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(hwnd, size),
            &pRenderTarget
        );

        pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x0F172A), &pBrushBg);          // Dark slate background
        pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x1E293B), &pBrushCard);        // Darker slate card
        pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x0B0F19), &pBrushTitleBar);    // Top bar
        pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0xF8FAFC), &pBrushTextMain);    // Bright white text
        pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x94A3B8), &pBrushTextSub);     // Muted gray text
        pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x38BDF8), &pBrushAccent);      // Cyan accent
        pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x2563EB), &pBrushButtonNormal);  // Blue button
        pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x3B82F6), &pBrushButtonHover);   // Light blue hover
        pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0xEF4444), &pBrushCloseHover);   // Red hover for close button
    }
}

void DiscardD2DResources() {
    if (pRenderTarget) {
        pRenderTarget->Release(); pRenderTarget = NULL;
        pBrushBg->Release(); pBrushCard->Release();
        pBrushTitleBar->Release(); pBrushTextMain->Release();
        pBrushTextSub->Release(); pBrushAccent->Release();
        pBrushButtonNormal->Release(); pBrushButtonHover->Release();
        pBrushCloseHover->Release();
    }
}

// Render Window
void OnRender(HWND hwnd) {
    CreateD2DResources(hwnd);

    pRenderTarget->BeginDraw();
    pRenderTarget->Clear(D2D1::ColorF(0x0F172A));

    RECT rc;
    GetClientRect(hwnd, &rc);
    float width = static_cast<float>(rc.right - rc.left);
    float height = static_cast<float>(rc.bottom - rc.top);

    // 1. Draw Title Bar (Custom Drag Area)
    D2D1_RECT_F titleBarRect = D2D1::RectF(0, 0, width, 40);
    pRenderTarget->FillRectangle(titleBarRect, pBrushTitleBar);

    std::wstring appTitle = L"FIVEM PERFORMANCE SUITE v2.0";
    pRenderTarget->DrawText(appTitle.c_str(), appTitle.length(), pFontTitle, D2D1::RectF(15, 8, 300, 32), pBrushAccent);

    // Close Button (X)
    D2D1_RECT_F closeBtnRect = D2D1::RectF(width - 40, 0, width, 40);
    if (isHoverClose) {
        pRenderTarget->FillRectangle(closeBtnRect, pBrushCloseHover);
    }
    pRenderTarget->DrawText(L"X", 1, pFontHeading, D2D1::RectF(width - 40, 8, width, 32), pBrushTextMain);

    // 2. Action Buttons Section
    // Button 1: Check Apps
    D2D1_ROUNDED_RECT btnCheckRect = D2D1::RoundedRect(D2D1::RectF(20, 60, 260, 110), 8.0f, 8.0f);
    pRenderTarget->FillRoundedRectangle(btnCheckRect, isHoverBtnCheck ? pBrushButtonHover : pBrushButtonNormal);
    std::wstring txtCheck = L"1. Check Essential Apps";
    pRenderTarget->DrawText(txtCheck.c_str(), txtCheck.length(), pFontButton, D2D1::RectF(20, 72, 260, 110), pBrushTextMain);

    // Button 2: Optimize FPS
    D2D1_ROUNDED_RECT btnOptRect = D2D1::RoundedRect(D2D1::RectF(280, 60, 520, 110), 8.0f, 8.0f);
    pRenderTarget->FillRoundedRectangle(btnOptRect, isHoverBtnOpt ? pBrushButtonHover : pBrushButtonNormal);
    std::wstring txtOpt = L"2. Optimize Windows FPS";
    pRenderTarget->DrawText(txtOpt.c_str(), txtOpt.length(), pFontButton, D2D1::RectF(280, 72, 520, 110), pBrushTextMain);

    // 3. Log Output Card
    D2D1_ROUNDED_RECT cardRect = D2D1::RoundedRect(D2D1::RectF(20, 130, width - 20, height - 20), 10.0f, 10.0f);
    pRenderTarget->FillRoundedRectangle(cardRect, pBrushCard);

    float textY = 145.0f;
    for (const auto& line : logOutput) {
        if (textY > height - 40) break;
        pRenderTarget->DrawText(
            line.c_str(), 
            line.length(), 
            pFontBody, 
            D2D1::RectF(35, textY, width - 35, textY + 24), 
            line.find(L"[ MISSING ]") != std::wstring::npos ? pBrushCloseHover : (line.find(L"[ INSTALLED ]") != std::wstring::npos || line.find(L"[SUCCESS]") != std::wstring::npos ? pBrushAccent : pBrushTextMain)
        );
        textY += 22.0f;
    }

    // 4. Custom Alert Dialog Overlay
    if (showCustomAlert) {
        // Semi-transparent backdrop
        pRenderTarget->FillRectangle(D2D1::RectF(0, 0, width, height), pBrushTitleBar);

        // Alert Box
        D2D1_ROUNDED_RECT alertBox = D2D1::RoundedRect(D2D1::RectF(width / 2 - 180, height / 2 - 80, width / 2 + 180, height / 2 + 80), 12.0f, 12.0f);
        pRenderTarget->FillRoundedRectangle(alertBox, pBrushCard);
        pRenderTarget->DrawRoundedRectangle(alertBox, pBrushAccent, 1.5f);

        pRenderTarget->DrawText(alertTitle.c_str(), alertTitle.length(), pFontHeading, D2D1::RectF(width / 2 - 160, height / 2 - 65, width / 2 + 160, height / 2 - 35), pBrushAccent);
        pRenderTarget->DrawText(alertMsg.c_str(), alertMsg.length(), pFontBody, D2D1::RectF(width / 2 - 160, height / 2 - 25, width / 2 + 160, height / 2 + 25), pBrushTextMain);

        // OK Button
        D2D1_ROUNDED_RECT alertBtn = D2D1::RoundedRect(D2D1::RectF(width / 2 - 50, height / 2 + 30, width / 2 + 50, height / 2 + 65), 6.0f, 6.0f);
        pRenderTarget->FillRoundedRectangle(alertBtn, pBrushButtonNormal);
        std::wstring okText = L"OK";
        pRenderTarget->DrawText(okText.c_str(), okText.length(), pFontButton, D2D1::RectF(width / 2 - 50, height / 2 + 36, width / 2 + 50, height / 2 + 65), pBrushTextMain);
    }

    pRenderTarget->EndDraw();
}

// Window Event Handler
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE: {
        D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pD2DFactory);
        DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&pDWriteFactory));

        pDWriteFactory->CreateTextFormat(L"Segoe UI", NULL, DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 14.0f, L"en-us", &pFontTitle);
        pDWriteFactory->CreateTextFormat(L"Segoe UI", NULL, DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 16.0f, L"en-us", &pFontHeading);
        pDWriteFactory->CreateTextFormat(L"Consolas", NULL, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 13.0f, L"en-us", &pFontBody);
        pDWriteFactory->CreateTextFormat(L"Segoe UI", NULL, DWRITE_FONT_WEIGHT_SEMI_BOLD, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 14.0f, L"en-us", &pFontButton);

        pFontTitle->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
        pFontHeading->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
        pFontButton->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
        break;
    }
    case WM_MOUSEMOVE: {
        float x = (float)GET_X_LPARAM(lParam);
        float y = (float)GET_Y_LPARAM(lParam);

        RECT rc;
        GetClientRect(hwnd, &rc);
        float width = static_cast<float>(rc.right - rc.left);

        isHoverBtnCheck = (x >= 20 && x <= 260 && y >= 60 && y <= 110);
        isHoverBtnOpt = (x >= 280 && x <= 520 && y >= 60 && y <= 110);
        isHoverClose = (x >= width - 40 && x <= width && y >= 0 && y <= 40);

        InvalidateRect(hwnd, NULL, FALSE);
        break;
    }
    case WM_LBUTTONDOWN: {
        float x = (float)GET_X_LPARAM(lParam);
        float y = (float)GET_Y_LPARAM(lParam);

        RECT rc;
        GetClientRect(hwnd, &rc);
        float width = static_cast<float>(rc.right - rc.left);
        float height = static_cast<float>(rc.bottom - rc.top);

        // Handle Alert Dialog Click
        if (showCustomAlert) {
            if (x >= width / 2 - 50 && x <= width / 2 + 50 && y >= height / 2 + 30 && y <= height / 2 + 65) {
                showCustomAlert = false;
                InvalidateRect(hwnd, NULL, FALSE);
            }
            return 0;
        }

        // Close Button
        if (x >= width - 40 && x <= width && y >= 0 && y <= 40) {
            PostQuitMessage(0);
        }
        // Smooth Dragging Window Anywhere on Title Bar
        else if (y <= 40) {
            ReleaseCapture();
            SendMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
        }
        // Button Check
        else if (isHoverBtnCheck) {
            CheckRequiredApps();
            InvalidateRect(hwnd, NULL, FALSE);
        }
        // Button Optimize
        else if (isHoverBtnOpt) {
            OptimizeWindows();
            InvalidateRect(hwnd, NULL, FALSE);
        }
        break;
    }
    case WM_PAINT: {
        OnRender(hwnd);
        ValidateRect(hwnd, NULL);
        break;
    }
    case WM_DESTROY: {
        DiscardD2DResources();
        if (pD2DFactory) pD2DFactory->Release();
        if (pDWriteFactory) pDWriteFactory->Release();
        PostQuitMessage(0);
        return 0;
    }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// Entry Point
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"FiveM_D2D_GUI_Class";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClass(&wc);

    // Create Borderless Window for Custom UI
    HWND hwnd = CreateWindowEx(
        0, CLASS_NAME, L"FiveM Performance Suite",
        WS_POPUP | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 540, 420,
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
