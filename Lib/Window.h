//////////////////////////////////////////////////
// �쐬��:2017/02/26
// �X�V��:2017/02/26
// �����:got
//
// �N���X�ڍ�:�E�B���h�E�𐶐����邽�߂̃N���X
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
