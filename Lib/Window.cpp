//////////////////////////////////////////////////
// 作成日:2017/02/26
// 更新日:2017/02/26
// 制作者:got
//////////////////////////////////////////////////
#include <tchar.h>
#include "Window.h"

// コンストラクタ
got::Window::Window(const LPCSTR _windowName)
    : windowName(_windowName)
{
}
// デストラクタ
got::Window::~Window()
{
}
// 初期化
bool got::Window::init()
{
    if (FAILED(setupWindow())) {
        return false;
    }

    return true;
}
// 更新
MSG got::Window::update()
{
    MSG msg = {};

    if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg;
}
// ハンドルの取得
HWND got::Window::getHWND() const
{
    return HWND();
}
// ウィンドウに関する各種パラメータの設定とウィンドウの作成
HRESULT got::Window::setupWindow()
{
    auto hInstance = GetModuleHandle(0);

    WNDCLASSEX wcex;
    wcex.cbSize        = sizeof(WNDCLASSEX);						 // 構造体のサイズ
    wcex.style         = CS_HREDRAW | CS_VREDRAW;					 // ウィンドウスタイル
    wcex.lpfnWndProc   = wndProck;									 // ウィンドウプロシージャ
    wcex.cbClsExtra    = 0;											 // ウィンドウクラスに付加したいメモリ
    wcex.cbWndExtra    = 0;											 // ウィンドウに付加したいメモリ
    wcex.hInstance     = hInstance;									 // インスタンスハンドル
    wcex.hIcon         = nullptr;									 // アイコン
    wcex.hCursor       = nullptr;									 // カーソル
    wcex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1); // 背景色
    wcex.lpszMenuName  = nullptr;									 // メニュー
    wcex.lpszClassName = _T("WindowClass");								 // ウィンドウクラスの名前
    wcex.hIconSm       = nullptr;									 // アイコン小
    if (!RegisterClassEx(&wcex)) {
        return E_FAIL;
    }

    // ウィンドウの作成
    RECT rect = { 0, 0, 1048, 768 };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
    const int windowWidth = (rect.right - rect.left);
    const int windowHeight = (rect.bottom - rect.top);

    HWND hWnd = CreateWindow(
        _T("WindowClass"),		// ウィンドウクラス名
        _T("gotLib"),			// ウィンドウタイトル
        WS_OVERLAPPEDWINDOW,	// ウィンドウスタイル
        CW_USEDEFAULT,			// Y座標の初期値
        CW_USEDEFAULT,			// X座標の初期値
        windowWidth,            // 幅の初期値
        windowHeight,           // 高さの初期値
        nullptr,				// 親ウィンドウのハンドル
        nullptr,				// ウィンドウメニュー
        hInstance,				// インスタンスハンドル
        nullptr);				// 作成パラメータ
    if (!hWnd) {
        return E_FAIL;
    }

    ShowWindow(hWnd, SW_SHOWNORMAL);

    return S_OK;
}
// ウィンドウプロシージャ
LRESULT got::Window::wndProck(HWND _hWnd, UINT _message, WPARAM _wParam, LPARAM _lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;

    switch (_message) {
    case WM_KEYDOWN:
        if (_wParam == VK_ESCAPE) {
            PostMessage(_hWnd, WM_DESTROY, 0, 0);
            return 0;
        }
        break;
    case WM_PAINT:
        hdc = BeginPaint(_hWnd, &ps);
        EndPaint(_hWnd, &ps);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(_hWnd, _message, _wParam, _lParam);
    }
    return 0;
}
