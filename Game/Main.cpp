//////////////////////////////////////////////////
// �쐬��:2017/02/26
// �X�V��:2017/02/26
// �����:got
//////////////////////////////////////////////////
#include"Game.h"

// ���C���֐�
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);

    auto & game = Game::getInstance();

    game.init();

    game.update();

    game.end();

    return 0;
}