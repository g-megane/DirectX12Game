//////////////////////////////////////////////////
// �쐬��:2017/02/26
// �X�V��:2017/02/26
// �����:got
//
// �N���X�ڍ�:�Q�[���̍����ɂȂ�N���X
//////////////////////////////////////////////////
#pragma once
#ifndef GAME_H
#define GAME_H
#include "..//Lib//Singleton.h"
#include "..//Lib//Window.h"

class Game : public got::Singleton<Game>
{
public:
    ~Game();
    bool init();
    void update();
    void end();
    
private:
    friend class got::Singleton<Game>;
    Game();
    std::shared_ptr<got::Window> m_spWindow;

};

#endif
