//////////////////////////////////////////////////
// 作成日:2017/02/26
// 更新日:2017/02/26
// 制作者:got
//
// クラス詳細:ウィンドウを生成するためのクラス
//////////////////////////////////////////////////
#pragma once
#include <Windows.h>

namespace got
{
    class Window
    {
    public:
        Window(const LPCSTR _windowName);
        ~Window();

        bool init();
        MSG  update();
        HWND getHWND() const;

    private:
        HRESULT setupWindow();
        static LRESULT CALLBACK wndProck(HWND _hWnd, UINT _message, WPARAM _wParam, LPARAM _lParam);

        LPCSTR windowName;
        HWND hWnd;

    };
}
