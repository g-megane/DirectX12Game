//////////////////////////////////////////////////
// 作成日:2017/02/26
// 更新日:2017/02/26
// 制作者:got
//
// クラス詳細:ゲームの根幹になるクラス
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
