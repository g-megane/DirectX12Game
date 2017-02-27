//////////////////////////////////////////////////
// �쐬��:2017/02/26
// �X�V��:2017/02/26
// �����:got
//////////////////////////////////////////////////
#include <stdexcept>
#include "Game.h"
#include "../Lib/DirectX12.h"

// �R���X�g���N�^
Game::Game()
{
    m_spWindow = std::make_shared<got::Window>("DirectX12Project");
}
// �f�X�g���N�^
Game::~Game()
{
}
// ������
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
// �X�V(���C�����[�v)
void Game::update()
{
    while (true) {
        if (m_spWindow->update().message == WM_QUIT) {
            break;
        }

        got::DirectX12::getInstance().draw();
    }
}
// �I������
void Game::end()
{
}

