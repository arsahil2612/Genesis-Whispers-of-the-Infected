#ifndef LEVEL3_MANAGER_H
#define LEVEL3_MANAGER_H

#include "player.h"
#include "Level3Boss.h"

enum Level3State
{
    LEVEL3_PLAYING,
    LEVEL3_BOSS_LOADING,
    LEVEL3_BOSS_INTRO,
    LEVEL3_BOSS_FIGHT
};

class Level3Manager
{
public:
    typedef Level3State GameState;

    Level3Manager();
    ~Level3Manager();

    void Initialize();
    void StartBossLoading(double arenaX = 7200.0);
    void Update(float dt, Player& player, Level3Boss& boss);
    void Draw();
    void StartBossFight();

    Level3State GetState() const { return state; }
    bool IsLoading() const { return state == LEVEL3_BOSS_LOADING; }
    bool IsInIntro() const { return state == LEVEL3_BOSS_INTRO; }
    bool IsInFight() const { return state == LEVEL3_BOSS_FIGHT; }

private:
    Level3State state;
    float loadingTimer;
    float minLoadingTime;
    float pulseTimer;
    unsigned int texLoadingBg;
    bool assetsLoaded;
    double savedArenaX;
};

#endif // LEVEL3_MANAGER_H
