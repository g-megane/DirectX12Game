//////////////////////////////////////////////////
// �쐬��:2017/02/26
// �X�V��:2017/02/26
// �����:got
//////////////////////////////////////////////////
#include <tchar.h>
#include "Window.h"

// �R���X�g���N�^
got::Window::Window(const LPCSTR _windowName)
    : windowName(_windowName)
{
}
// �f�X�g���N�^
got::Window::~Window()
{
}
// ������
bool got::Window::init()
{
    if (FAILED(setupWindow())) {
        return false;
    }

    return true;
}
// �X�V
MSG got::Window::update()
{
    MSG msg = {};

    if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg;
}
// �n���h���̎擾
HWND got::Window::getHWND() const
{
    return HWND();
}
// �E�B���h�E�Ɋւ���e��p�����[�^�̐ݒ�ƃE�B���h�E�̍쐬
HRESULT got::Window::setupWindow()
{
    auto hInstance = GetModuleHandle(0);

    WNDCLASSEX wcex;
    wcex.cbSize        = sizeof(WNDCLASSEX);						 // �\���̂̃T�C�Y
    wcex.style         = CS_HREDRAW | CS_VREDRAW;					 // �E�B���h�E�X�^�C��
    wcex.lpfnWndProc   = wndProck;									 // �E�B���h�E�v���V�[�W��
    wcex.cbClsExtra    = 0;											 // �E�B���h�E�N���X�ɕt��������������
    wcex.cbWndExtra    = 0;											 // �E�B���h�E�ɕt��������������
    wcex.hInstance     = hInstance;									 // �C���X�^���X�n���h��
    wcex.hIcon         = nullptr;									 // �A�C�R��
    wcex.hCursor       = nullptr;									 // �J�[�\��
    wcex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1); // �w�i�F
    wcex.lpszMenuName  = nullptr;									 // ���j���[
    wcex.lpszClassName = _T("WindowClass");								 // �E�B���h�E�N���X�̖��O
    wcex.hIconSm       = nullptr;									 // �A�C�R����
    if (!RegisterClassEx(&wcex)) {
        return E_FAIL;
    }

    // �E�B���h�E�̍쐬
    RECT rect = { 0, 0, 1048, 768 };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
    const int windowWidth = (rect.right - rect.left);
    const int windowHeight = (rect.bottom - rect.top);

    HWND hWnd = CreateWindow(
        _T("WindowClass"),		// �E�B���h�E�N���X��
        _T("gotLib"),			// �E�B���h�E�^�C�g��
        WS_OVERLAPPEDWINDOW,	// �E�B���h�E�X�^�C��
        CW_USEDEFAULT,			// Y���W�̏����l
        CW_USEDEFAULT,			// X���W�̏����l
        windowWidth,            // ���̏����l
        windowHeight,           // �����̏����l
        nullptr,				// �e�E�B���h�E�̃n���h��
        nullptr,				// �E�B���h�E���j���[
        hInstance,				// �C���X�^���X�n���h��
        nullptr);				// �쐬�p�����[�^
    if (!hWnd) {
        return E_FAIL;
    }

    ShowWindow(hWnd, SW_SHOWNORMAL);

    return S_OK;
}
// �E�B���h�E�v���V�[�W��
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
