//////////////////////////////////////////////////
// 作成日:2017/02/26
// 更新日:2017/02/26
// 制作者:got
//////////////////////////////////////////////////
#include "Game.h"

// コンストラクタ
Game::Game()
{
    window = std::make_shared<got::Window>("DirectX12Project");
}
// デストラクタ
Game::~Game()
{
}
// 初期化
bool Game::init()
{
    if (window->init()) {
        return false;
    }

    return true;
}
// 更新(メインループ)
void Game::update()
{
    while (true) {
        if (window->update().message == WM_QUIT) {
            break;
        }
    }
}
// 終了処理
void Game::end()
{
}

