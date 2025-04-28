// HideArrow.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "HideArrow.h"
#include <shellapi.h>
#include <TlHelp32.h>
#include <shlobj.h>
#include <string>

#define MAX_LOADSTRING 100

// 全局变量:
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名

// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 在此处放置代码。

    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_HIDEARROW, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_HIDEARROW));

    MSG msg;

    // 主消息循环:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  函数: MyRegisterClass()
//
//  目标: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_HIDEARROW));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_HIDEARROW);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 将实例句柄存储在全局变量中

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, 400, 200, nullptr, nullptr, hInstance, nullptr);

   // 创建按钮和状态文本控件
   CreateWindow(L"BUTTON", L"隐藏快捷方式箭头", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
      20, 20, 150, 30, hWnd, (HMENU)IDC_BUTTON_HIDE, hInstance, NULL);

   CreateWindow(L"STATIC", L"点击按钮开始处理...", WS_CHILD | WS_VISIBLE,
      20, 60, 350, 20, hWnd, (HMENU)IDC_STATIC_STATUS, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目标: 处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 分析菜单选择:
            switch (wmId)
            {
            case IDC_BUTTON_HIDE:
            {
                // 检查管理员权限
                BOOL isAdmin = FALSE;
                SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
                PSID AdministratorsGroup;
                if (AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
                    DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &AdministratorsGroup))
                {
                    CheckTokenMembership(NULL, AdministratorsGroup, &isAdmin);
                    FreeSid(AdministratorsGroup);
                }

                if (!isAdmin)
                {
                    SetWindowText(GetDlgItem(hWnd, IDC_STATIC_STATUS), L"请以管理员权限运行此程序！");
                    break;
                }

                // 添加注册表项
                HKEY hKey;
                if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Icons",
                    0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS)
                {
                    TCHAR value[] = L"%systemroot%\\system32\\imageres.dll,197";
                    RegSetValueEx(hKey, L"29", 0, REG_SZ, (BYTE*)value, (lstrlen(value) + 1) * sizeof(TCHAR));
                    RegCloseKey(hKey);
                }

                // 结束explorer进程
                HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0);
                PROCESSENTRY32W pEntry;
                pEntry.dwSize = sizeof(pEntry);
                BOOL hRes = Process32FirstW(hSnapShot, &pEntry);
                while (hRes)
                {
                    if (wcscmp(pEntry.szExeFile, L"explorer.exe") == 0)
                    {
                        HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, 0, pEntry.th32ProcessID);
                        if (hProcess != NULL)
                        {
                            TerminateProcess(hProcess, 9);
                            CloseHandle(hProcess);
                        }
                    }
                    hRes = Process32NextW(hSnapShot, &pEntry);
                }
                CloseHandle(hSnapShot);

                // 删除图标缓存
                TCHAR szPath[MAX_PATH];
                if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, szPath)))
                {
                    std::wstring cachePath = szPath;
                    cachePath += L"\\iconcache.db";
                    SetFileAttributes(cachePath.c_str(), FILE_ATTRIBUTE_NORMAL);
                    DeleteFile(cachePath.c_str());
                }

                // 等待一段时间确保explorer进程完全终止
                Sleep(1000);

                // 重启explorer
                STARTUPINFO si = { sizeof(si) };
                PROCESS_INFORMATION pi;
                TCHAR explorerPath[MAX_PATH];
                ExpandEnvironmentStrings(L"%windir%\\explorer.exe", explorerPath, MAX_PATH);
                
                if (CreateProcess(explorerPath, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
                {
                    CloseHandle(pi.hProcess);
                    CloseHandle(pi.hThread);
                }

                SetWindowText(GetDlgItem(hWnd, IDC_STATIC_STATUS), L"处理完成！");
                break;
            }
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 在此处添加使用 hdc 的任何绘图代码...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}