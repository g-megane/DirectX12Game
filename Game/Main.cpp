//////////////////////////////////////////////////
// çÏê¨ì˙:2017/02/26
// çXêVì˙:2017/02/26
// êßçÏé“:got
//////////////////////////////////////////////////
#include"Game.h"

// ÉÅÉCÉìä÷êî
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