//////////////////////////////////////////////////
// �쐬��:2017/02/26
// �X�V��:2017/02/26
// �����:got
//////////////////////////////////////////////////
#include "Game.h"

// �R���X�g���N�^
Game::Game()
{
    window = std::make_shared<got::Window>("DirectX12Project");
}
// �f�X�g���N�^
Game::~Game()
{
}
// ������
bool Game::init()
{
    if (window->init()) {
        return false;
    }

    return true;
}
// �X�V(���C�����[�v)
void Game::update()
{
    while (true) {
        if (window->update().message == WM_QUIT) {
            break;
        }
    }
}
// �I������
void Game::end()
{
}

