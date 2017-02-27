//////////////////////////////////////////////////
// 作成日:2017/02/26
// 更新日:2017/02/26
// 制作者:got
//////////////////////////////////////////////////
#include <stdexcept>
#include "Game.h"
#include "../Lib/DirectX12.h"

// コンストラクタ
Game::Game()
{
    m_spWindow = std::make_shared<got::Window>("DirectX12Project");
}
// デストラクタ
Game::~Game()
{
}
// 初期化
bool Game::init()
{
    if (!m_spWindow->init()) {
        return false;
    }

    auto hr = got::DirectX12::getInstance().init(1048, 768, m_spWindow);
    if (FAILED(hr)) {
        throw std::runtime_error("DirectX12 init() is failed value");
    }

    return true;
}
// 更新(メインループ)
void Game::update()
{
    while (true) {
        if (m_spWindow->update().message == WM_QUIT) {
            break;
        }

        got::DirectX12::getInstance().draw();
    }
}
// 終了処理
void Game::end()
{
}

