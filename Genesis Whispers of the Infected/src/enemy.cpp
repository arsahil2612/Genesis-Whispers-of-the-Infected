#include "enemy.h"
#include "igraphics_declarations.h"
#include <vector>
#include <cmath>
#include <windows.h>
#include <GL/gl.h>

// ============================================================================
// Static Member Texture Variables
// ============================================================================
unsigned int Enemy::texWalkerIdle = 0;
unsigned int Enemy::texWalkerWalk = 0;
unsigned int Enemy::texWalkerAttack = 0;
unsigned int Enemy::texWalkerHurt = 0;
unsigned int Enemy::texWalkerDeath = 0;

unsigned int Enemy::texRunnerIdle = 0;
unsigned int Enemy::texRunnerWalk = 0;
unsigned int Enemy::texRunnerRun = 0;
unsigned int Enemy::texRunnerAttack = 0;
unsigned int Enemy::texRunnerHurt = 0;
unsigned int Enemy::texRunnerDeath = 0;

unsigned int Enemy::texRaiderIdle = 0;
unsigned int Enemy::texRaiderWalk = 0;
unsigned int Enemy::texRaiderRun = 0;
unsigned int Enemy::texRaiderAttackMelee = 0;
unsigned int Enemy::texRaiderAttackRanged = 0;
unsigned int Enemy::texRaiderHurt = 0;
unsigned int Enemy::texRaiderDeath = 0;

unsigned int Enemy::texHeavyIdle = 0;
unsigned int Enemy::texHeavyWalk = 0;
unsigned int Enemy::texHeavyAttack = 0;
unsigned int Enemy::texHeavyHurt = 0;
unsigned int Enemy::texHeavyDeath = 0;

// Static Frame Sequence Vectors (Walker, Runner, Raider & Heavy)
std::vector<unsigned int> Enemy::seqWalkerIdle;
std::vector<unsigned int> Enemy::seqWalkerWalk;
std::vector<unsigned int> Enemy::seqWalkerAttack;
std::vector<unsigned int> Enemy::seqWalkerHurt;
std::vector<unsigned int> Enemy::seqWalkerDeath;

std::vector<unsigned int> Enemy::seqRunnerIdle;
std::vector<unsigned int> Enemy::seqRunnerWalk;
std::vector<unsigned int> Enemy::seqRunnerRun;
std::vector<unsigned int> Enemy::seqRunnerAttack;
std::vector<unsigned int> Enemy::seqRunnerHurt;
std::vector<unsigned int> Enemy::seqRunnerDeath;

std::vector<unsigned int> Enemy::seqRaiderIdle;
std::vector<unsigned int> Enemy::seqRaiderWalk;
std::vector<unsigned int> Enemy::seqRaiderAttack;
std::vector<unsigned int> Enemy::seqRaiderHurt;
std::vector<unsigned int> Enemy::seqRaiderDeath;

std::vector<unsigned int> Enemy::seqHeavyIdle;
std::vector<unsigned int> Enemy::seqHeavyWalk;
std::vector<unsigned int> Enemy::seqHeavyAttack;
std::vector<unsigned int> Enemy::seqHeavyHurt;
std::vector<unsigned int> Enemy::seqHeavyDeath;

std::vector<unsigned int> Enemy::seqAbominationIdle;
std::vector<unsigned int> Enemy::seqAbominationWalk;
std::vector<unsigned int> Enemy::seqAbominationAttack;
std::vector<unsigned int> Enemy::seqAbominationHurt;
std::vector<unsigned int> Enemy::seqAbominationDeath;

// ============================================================================
// Enemy Constructor & Texture Initialization
// ============================================================================
Enemy::Enemy(double sX, double eX, double startY, EnemyType t) {
    startX = sX;
    endX = eX;
    x = sX;
    y = startY;
    type = t;
    width = 64;
    height = 96;
    isFacingRight = true;
    animFrame = 0;
    frameCounter = 0;
    state = ENEMY_PATROL;
    attackCooldown = 0;
    stateTimer = 0;
    rangedShotFired = false;
    lastHitAttackID = 0;
    bruteAttack = BRUTE_PUNCH;
    bruteAttackTimer = 0;
    raiderBackstepTimer = 0;
    raiderPauseTimer = 0;
    inAttackRange = false;
    hasDealtDamage = false;

    if (type == TYPE_SPITTER) {
        hp = maxHp = 50;
        damage = 10;
        vx = 1.0;
    }
    else if (type == TYPE_RUNNER) {
        hp = maxHp = 60;
        damage = 15;
        vx = 3.5;
    }
    else if (type == TYPE_RAIDER) {
        hp = maxHp = 60;
        damage = 12;
        vx = 1.8;
    }
    else if (type == TYPE_HEAVY) {
        hp = maxHp = 120;
        damage = 18;
        vx = 1.2;
        width = 80;
        height = 100;
    }
    else if (type == TYPE_ABOMINATION) {
        hp = maxHp = 300;
        damage = 25;
        vx = 1.5;
        width = 128;
        height = 160;
    }

    // Load static textures once
    if (texWalkerIdle == 0) {
        texWalkerIdle = iLoadImage((char*)GetAssetPath("Assets/Characters/InfectedWalker/Idle/infected_walker_sprite_01.png").c_str());
        texWalkerWalk = iLoadImage((char*)GetAssetPath("Assets/Characters/InfectedWalker/Walk/infected_walker_sprite_01.png").c_str());
        texWalkerAttack = iLoadImage((char*)GetAssetPath("Assets/Characters/InfectedWalker/AttackClawsSwipe/infected_walker_sprite_01.png").c_str());
        texWalkerHurt = iLoadImage((char*)GetAssetPath("Assets/Characters/InfectedWalker/Hurt/infected_walker_01.png").c_str());
        texWalkerDeath = iLoadImage((char*)GetAssetPath("Assets/Characters/InfectedWalker/Death/infected_walker_01.png").c_str());

        texRunnerIdle = iLoadImage((char*)GetAssetPath("Assets/Characters/runnerIdle.png").c_str());
        texRunnerWalk = iLoadImage((char*)GetAssetPath("Assets/Characters/runnerWalk.png").c_str());
        texRunnerRun = iLoadImage((char*)GetAssetPath("Assets/Characters/runnerRun.png").c_str());
        texRunnerAttack = iLoadImage((char*)GetAssetPath("Assets/Characters/runnerLeapAttack.png").c_str());
        texRunnerHurt = iLoadImage((char*)GetAssetPath("Assets/Characters/runnerHurt.png").c_str());
        texRunnerDeath = iLoadImage((char*)GetAssetPath("Assets/Characters/runnerDeath.png").c_str());

        texRaiderIdle = iLoadImage((char*)GetAssetPath("Assets/Characters/raiderIdle.png").c_str());
        texRaiderWalk = iLoadImage((char*)GetAssetPath("Assets/Characters/raiderWalk.png").c_str());
        texRaiderRun = iLoadImage((char*)GetAssetPath("Assets/Characters/raiderRun.png").c_str());
        texRaiderAttackMelee = iLoadImage((char*)GetAssetPath("Assets/Characters/raiderAttackMelee.png").c_str());
        texRaiderAttackRanged = iLoadImage((char*)GetAssetPath("Assets/Characters/raiderAttackRanged.png").c_str());
        texRaiderHurt = iLoadImage((char*)GetAssetPath("Assets/Characters/raiderHurt.png").c_str());
        texRaiderDeath = iLoadImage((char*)GetAssetPath("Assets/Characters/raiderDeath.png").c_str());

        texHeavyIdle = iLoadImage((char*)GetAssetPath("Assets/Characters/Heavy Infected/idle/enemy_heavy_infected_idle_01.png").c_str());
        texHeavyWalk = iLoadImage((char*)GetAssetPath("Assets/Characters/Heavy Infected/walk/enemy_heavy_infected_walk_01_master.png").c_str());
        texHeavyAttack = iLoadImage((char*)GetAssetPath("Assets/Characters/Heavy Infected/attack/enemy_heavy_infected_attack_01_master.png").c_str());
        texHeavyHurt = iLoadImage((char*)GetAssetPath("Assets/Characters/Heavy Infected/hurt/enemy_heavy_infected_hurt_01_master_1.png").c_str());
        texHeavyDeath = iLoadImage((char*)GetAssetPath("Assets/Characters/Heavy Infected/death/enemy_heavy_infected_death_01_master.png").c_str());
    }

    // Initialize reusable Animation instances for enemy states (cached statically once)
    if (type == TYPE_SPITTER) {
        if (seqWalkerIdle.empty() || seqWalkerIdle[0] == 0) {
            seqWalkerIdle.clear();
            seqWalkerWalk.clear();
            seqWalkerAttack.clear();
            seqWalkerHurt.clear();
            seqWalkerDeath.clear();

            for (int i = 1; i <= 6; ++i) {
                char path[256];
                sprintf_s(path, sizeof(path), "Assets/Characters/InfectedWalker/Idle/infected_walker_sprite_%02d.png", i);
                seqWalkerIdle.push_back(iLoadImage((char*)GetAssetPath(path).c_str()));
            }
            for (int i = 1; i <= 8; ++i) {
                char path[256];
                sprintf_s(path, sizeof(path), "Assets/Characters/InfectedWalker/Walk/infected_walker_sprite_%02d.png", i);
                seqWalkerWalk.push_back(iLoadImage((char*)GetAssetPath(path).c_str()));
            }
            for (int i = 1; i <= 8; ++i) {
                char path[256];
                sprintf_s(path, sizeof(path), "Assets/Characters/InfectedWalker/AttackClawsSwipe/infected_walker_sprite_%02d.png", i);
                seqWalkerAttack.push_back(iLoadImage((char*)GetAssetPath(path).c_str()));
            }
            for (int i = 1; i <= 4; ++i) {
                char path[256];
                sprintf_s(path, sizeof(path), "Assets/Characters/InfectedWalker/Hurt/infected_walker_%02d.png", i);
                seqWalkerHurt.push_back(iLoadImage((char*)GetAssetPath(path).c_str()));
            }
            for (int i = 1; i <= 8; ++i) {
                char path[256];
                sprintf_s(path, sizeof(path), "Assets/Characters/InfectedWalker/Death/infected_walker_%02d.png", i);
                seqWalkerDeath.push_back(iLoadImage((char*)GetAssetPath(path).c_str()));
            }
        }

        animIdle.Init(seqWalkerIdle, 10, true);     // 6 FPS (10 ticks)
        animWalk.Init(seqWalkerWalk, 6, true);      // 10 FPS (6 ticks)
        animAttack.Init(seqWalkerAttack, 5, false); // 12 FPS (5 ticks, 8 frames = 40 ticks = 0.67s)
        animHurt.Init(seqWalkerHurt, 6, false);     // 10 FPS (6 ticks, 4 frames = 24 ticks = 0.4s)
        animDeath.Init(seqWalkerDeath, 7, false);   // 8.5 FPS (7 ticks, 8 frames = 56 ticks = 0.93s)
    }
    else if (type == TYPE_RUNNER) {
        if (seqRunnerIdle.empty() || seqRunnerIdle[0] == 0) {
            seqRunnerIdle.clear();
            seqRunnerWalk.clear();
            seqRunnerRun.clear();
            seqRunnerAttack.clear();
            seqRunnerHurt.clear();
            seqRunnerDeath.clear();

            for (int i = 1; i <= 6; ++i) {
                char path[256];
                sprintf_s(path, sizeof(path), "Assets/Characters/Runner/Idle/frame_%02d.png", i);
                unsigned int handle = iLoadImage((char*)GetAssetPath(path).c_str());
                if (handle != 0) seqRunnerIdle.push_back(handle);
            }
            for (int i = 1; i <= 8; ++i) {
                char path[256];
                sprintf_s(path, sizeof(path), "Assets/Characters/Runner/Walk/runnerWalk_frame_%02d.png", i);
                unsigned int handle = iLoadImage((char*)GetAssetPath(path).c_str());
                if (handle != 0) seqRunnerWalk.push_back(handle);
            }
            for (int i = 1; i <= 10; ++i) {
                char path[256];
                sprintf_s(path, sizeof(path), "Assets/Characters/Runner/Run/frame_%02d.png", i);
                unsigned int handle = iLoadImage((char*)GetAssetPath(path).c_str());
                if (handle != 0) seqRunnerRun.push_back(handle);
            }
            for (int i = 1; i <= 8; ++i) {
                char path[256];
                sprintf_s(path, sizeof(path), "Assets/Characters/Runner/Leap Attack/frame_%02d.png", i);
                unsigned int handle = iLoadImage((char*)GetAssetPath(path).c_str());
                if (handle != 0) seqRunnerAttack.push_back(handle);
            }
            for (int i = 1; i <= 4; ++i) {
                char path[256];
                sprintf_s(path, sizeof(path), "Assets/Characters/Runner/Hurt/frame_%02d.png", i);
                unsigned int handle = iLoadImage((char*)GetAssetPath(path).c_str());
                if (handle != 0) seqRunnerHurt.push_back(handle);
            }
            seqRunnerDeath.clear();
            if (!seqRunnerHurt.empty()) {
                seqRunnerDeath.push_back(seqRunnerHurt[0]);
                if (seqRunnerHurt.size() > 1) seqRunnerDeath.push_back(seqRunnerHurt[1]);
            }
            if (!seqWalkerDeath.empty()) {
                int startIdx = (int)seqWalkerDeath.size() - 3;
                if (startIdx < 0) startIdx = 0;
                for (size_t i = startIdx; i < seqWalkerDeath.size(); ++i) {
                    seqRunnerDeath.push_back(seqWalkerDeath[i]);
                }
            }
        }

        animIdle.Init(seqRunnerIdle, 5, true);     // 5 ticks per frame (feral 12 FPS idle)
        animWalk.Init(seqRunnerRun, 3, true);      // 3 ticks per frame (aggressive 20 FPS sprint, synced with vx=4.2)
        animAttack.Init(seqRunnerAttack, 4, false); // 4 ticks per frame (fast 15 FPS leap strike)
        animHurt.Init(seqRunnerHurt, 4, false);     // 4 ticks per frame (fast hit reaction)
        animDeath.Init(seqRunnerDeath, 6, false);   // 6 ticks per frame (clean collapse)
    }
    else if (type == TYPE_RAIDER) {
        if (seqRaiderIdle.empty() || seqRaiderIdle[0] == 0) {
            seqRaiderIdle.clear();
            seqRaiderWalk.clear();
            seqRaiderAttack.clear();
            seqRaiderHurt.clear();
            seqRaiderDeath.clear();

            for (int i = 1; i <= 6; ++i) {
                char path[256];
                sprintf_s(path, sizeof(path), "Assets/Characters/Raider/Idle/frame_%02d.png", i);
                unsigned int handle = iLoadImage((char*)GetAssetPath(path).c_str());
                if (handle != 0) seqRaiderIdle.push_back(handle);
            }
            for (int i = 1; i <= 8; ++i) {
                char path[256];
                sprintf_s(path, sizeof(path), "Assets/Characters/Raider/Walk/frame_%02d.png", i);
                unsigned int handle = iLoadImage((char*)GetAssetPath(path).c_str());
                if (handle != 0) seqRaiderWalk.push_back(handle);
            }
            for (int i = 1; i <= 8; ++i) {
                char path[256];
                sprintf_s(path, sizeof(path), "Assets/Characters/Raider/Machete Attack/frame_%02d.png", i);
                unsigned int handle = iLoadImage((char*)GetAssetPath(path).c_str());
                if (handle != 0) seqRaiderAttack.push_back(handle);
            }
            for (int i = 1; i <= 4; ++i) {
                char path[256];
                sprintf_s(path, sizeof(path), "Assets/Characters/Raider/Hurt/frame_%02d.png", i);
                unsigned int handle = iLoadImage((char*)GetAssetPath(path).c_str());
                if (handle != 0) seqRaiderHurt.push_back(handle);
            }
            for (int i = 1; i <= 8; ++i) {
                char path[256];
                sprintf_s(path, sizeof(path), "Assets/Characters/Raider/Death/raider_death_sprite_%02d.png", i);
                unsigned int handle = iLoadImage((char*)GetAssetPath(path).c_str());
                if (handle == 0) {
                    sprintf_s(path, sizeof(path), "Assets/Characters/Raider/Death/frame_%02d.png", i);
                    handle = iLoadImage((char*)GetAssetPath(path).c_str());
                }
                if (handle != 0) seqRaiderDeath.push_back(handle);
            }

            // Fallback to single static textures if subfolder frames are missing
            if (seqRaiderIdle.empty() && texRaiderIdle != 0) seqRaiderIdle.push_back(texRaiderIdle);
            if (seqRaiderWalk.empty()) {
                if (texRaiderWalk != 0) seqRaiderWalk.push_back(texRaiderWalk);
                else if (texRaiderRun != 0) seqRaiderWalk.push_back(texRaiderRun);
            }
            if (seqRaiderAttack.empty() && texRaiderAttackMelee != 0) seqRaiderAttack.push_back(texRaiderAttackMelee);
            if (seqRaiderHurt.empty() && texRaiderHurt != 0) seqRaiderHurt.push_back(texRaiderHurt);
            if (seqRaiderDeath.empty() && texRaiderDeath != 0) seqRaiderDeath.push_back(texRaiderDeath);
        }

        animIdle.Init(seqRaiderIdle, 8, true);      // 8 ticks/frame (7.5 FPS idle)
        animWalk.Init(seqRaiderWalk, 5, true);      // 5 ticks/frame (12 FPS walk)
        animAttack.Init(seqRaiderAttack, 5, false); // 5 ticks/frame (12 FPS machete slash)
        animHurt.Init(seqRaiderHurt, 6, false);     // 6 ticks/frame (10 FPS hurt)
        animDeath.Init(seqRaiderDeath, 7, false);   // 7 ticks/frame (8.5 FPS death)
    }
    else if (type == TYPE_HEAVY) {
        if (seqHeavyIdle.empty() || seqHeavyIdle[0] == 0) {
            seqHeavyIdle.clear();
            seqHeavyWalk.clear();
            seqHeavyAttack.clear();
            seqHeavyHurt.clear();
            seqHeavyDeath.clear();

            for (int i = 1; i <= 6; ++i) {
                char path[256];
                sprintf_s(path, sizeof(path), "Assets/Characters/Heavy Infected/idle/enemy_heavy_infected_idle_%02d.png", i);
                unsigned int handle = iLoadImage((char*)GetAssetPath(path).c_str());
                if (handle != 0) seqHeavyIdle.push_back(handle);
            }
            for (int i = 1; i <= 8; ++i) {
                char path[256];
                sprintf_s(path, sizeof(path), "Assets/Characters/Heavy Infected/walk/enemy_heavy_infected_walk_%02d_master.png", i);
                unsigned int handle = iLoadImage((char*)GetAssetPath(path).c_str());
                if (handle != 0) seqHeavyWalk.push_back(handle);
            }
            for (int i = 1; i <= 8; ++i) {
                char path[256];
                sprintf_s(path, sizeof(path), "Assets/Characters/Heavy Infected/attack/enemy_heavy_infected_attack_%02d_master.png", i);
                unsigned int handle = iLoadImage((char*)GetAssetPath(path).c_str());
                if (handle != 0) seqHeavyAttack.push_back(handle);
            }
            for (int i = 1; i <= 4; ++i) {
                char path[256];
                if (i == 1) {
                    sprintf_s(path, sizeof(path), "Assets/Characters/Heavy Infected/hurt/enemy_heavy_infected_hurt_01_master_1.png");
                } else {
                    sprintf_s(path, sizeof(path), "Assets/Characters/Heavy Infected/hurt/enemy_heavy_infected_hurt_%02d_master.png", i);
                }
                unsigned int handle = iLoadImage((char*)GetAssetPath(path).c_str());
                if (handle != 0) seqHeavyHurt.push_back(handle);
            }
            for (int i = 1; i <= 8; ++i) {
                char path[256];
                sprintf_s(path, sizeof(path), "Assets/Characters/Heavy Infected/death/enemy_heavy_infected_death_%02d_master.png", i);
                unsigned int handle = iLoadImage((char*)GetAssetPath(path).c_str());
                if (handle != 0) seqHeavyDeath.push_back(handle);
            }

            // Fallback to single static textures if subfolder frames are missing
            if (seqHeavyIdle.empty() && texHeavyIdle != 0) seqHeavyIdle.push_back(texHeavyIdle);
            if (seqHeavyWalk.empty()) {
                if (texHeavyWalk != 0) seqHeavyWalk.push_back(texHeavyWalk);
                else if (!seqHeavyIdle.empty()) seqHeavyWalk = seqHeavyIdle;
            }
            if (seqHeavyAttack.empty() && texHeavyAttack != 0) seqHeavyAttack.push_back(texHeavyAttack);
            if (seqHeavyHurt.empty() && texHeavyHurt != 0) seqHeavyHurt.push_back(texHeavyHurt);
            if (seqHeavyDeath.empty()) {
                if (texHeavyDeath != 0) seqHeavyDeath.push_back(texHeavyDeath);
                else if (!seqHeavyHurt.empty()) seqHeavyDeath = seqHeavyHurt;
            }
        }

        animIdle.Init(seqHeavyIdle, 9, true);      // 9 ticks/frame (6.6 FPS heavy breathing idle)
        animWalk.Init(seqHeavyWalk, 6, true);      // 6 ticks/frame (10 FPS heavy walk stride)
        animAttack.Init(seqHeavyAttack, 6, false); // 6 ticks/frame (10 FPS heavy smash)
        animHurt.Init(seqHeavyHurt, 5, false);     // 5 ticks/frame (12 FPS responsive hurt stagger)
        animDeath.Init(seqHeavyDeath, 7, false);   // 7 ticks/frame (8.5 FPS heavy collapse)
    }
    else if (type == TYPE_ABOMINATION) {
        if (seqAbominationIdle.empty() || seqAbominationIdle[0] == 0) {
            seqAbominationIdle.clear();
            seqAbominationWalk.clear();
            seqAbominationAttack.clear();
            seqAbominationHurt.clear();
            seqAbominationDeath.clear();

            for (int i = 1; i <= 8; ++i) {
                char path[256];
                sprintf_s(path, sizeof(path), "Assets/Characters/Mutated Brute/idle/muted_brute_Idle_%02d.png", i);
                unsigned int handle = iLoadImage((char*)GetAssetPath(path).c_str());
                if (handle != 0) seqAbominationIdle.push_back(handle);
            }
            for (int i = 1; i <= 8; ++i) {
                char path[256];
                sprintf_s(path, sizeof(path), "Assets/Characters/Mutated Brute/walk/muted_brute_Walk_%02d.png", i);
                unsigned int handle = iLoadImage((char*)GetAssetPath(path).c_str());
                if (handle != 0) seqAbominationWalk.push_back(handle);
            }
            for (int i = 1; i <= 8; ++i) {
                char path[256];
                sprintf_s(path, sizeof(path), "Assets/Characters/Mutated Brute/attack/muted_brute_Attack_%02d.png", i);
                unsigned int handle = iLoadImage((char*)GetAssetPath(path).c_str());
                if (handle != 0) seqAbominationAttack.push_back(handle);
            }
            for (int i = 1; i <= 8; ++i) {
                char path[256];
                sprintf_s(path, sizeof(path), "Assets/Characters/Mutated Brute/slam/muted_brute_Ground_slam_%02d.png", i);
                unsigned int handle = iLoadImage((char*)GetAssetPath(path).c_str());
                if (handle != 0) seqAbominationHurt.push_back(handle);
            }
            for (int i = 1; i <= 8; ++i) {
                char path[256];
                sprintf_s(path, sizeof(path), "Assets/Characters/Mutated Brute/death/muted_brute_Death_%02d.png", i);
                unsigned int handle = iLoadImage((char*)GetAssetPath(path).c_str());
                if (handle != 0) seqAbominationDeath.push_back(handle);
            }

            if (seqAbominationWalk.empty() && !seqAbominationIdle.empty()) seqAbominationWalk = seqAbominationIdle;
            if (seqAbominationAttack.empty() && !seqAbominationIdle.empty()) seqAbominationAttack = seqAbominationIdle;
            if (seqAbominationHurt.empty() && !seqAbominationIdle.empty()) seqAbominationHurt = seqAbominationIdle;
            if (seqAbominationDeath.empty() && !seqAbominationHurt.empty()) seqAbominationDeath = seqAbominationHurt;
        }

        animIdle.Init(seqAbominationIdle, 8, true);      // 8 ticks/frame
        animWalk.Init(seqAbominationWalk, 6, true);      // 6 ticks/frame
        animAttack.Init(seqAbominationAttack, 6, false); // 6 ticks/frame
        animHurt.Init(seqAbominationHurt, 5, false);     // 5 ticks/frame
        animDeath.Init(seqAbominationDeath, 7, false);   // 7 ticks/frame
    }
}

// ============================================================================
// AI State Machine & Physics Update
// ============================================================================
void Enemy::Update(double playerX, double playerY, bool playerIsAttacking) {
    EnemyState oldState = state;

    if (state == ENEMY_DEAD) {
        if (animDeath.IsValid()) {
            animDeath.Update();
            animFrame = animDeath.GetCurrentFrame();
        }
        return;
    }

    // Decrement attack cooldown timer if active
    if (attackCooldown > 0) {
        attackCooldown--;
    }

    double distToPlayer = std::abs(playerX - x);
    double dyToPlayer = std::abs(playerY - y);

    const double ATTACK_RANGE = (type == TYPE_RUNNER) ? 75.0 : ((type == TYPE_HEAVY) ? 75.0 : 65.0);     // Attack range threshold in pixels
    const double DETECTION_RANGE = (type == TYPE_RUNNER) ? 550.0 : ((type == TYPE_HEAVY) ? 480.0 : 500.0); // Detection & chase range threshold in pixels

    if (state == ENEMY_HURT) {
        inAttackRange = false;
        if (animHurt.IsValid()) {
            animHurt.Update();
            animFrame = animHurt.GetCurrentFrame();
            if (animHurt.IsFinished()) {
                state = (distToPlayer <= DETECTION_RANGE && dyToPlayer < 120.0) ? ENEMY_CHASE : ENEMY_PATROL;
                animHurt.Reset();
            }
        }
        else {
            state = (distToPlayer <= DETECTION_RANGE && dyToPlayer < 120.0) ? ENEMY_CHASE : ENEMY_PATROL;
        }
    }
    else if (state == ENEMY_ATTACK) {
        // Face player when executing attack stroke
        if (playerX > x + 8.0) isFacingRight = true;
        else if (playerX < x - 8.0) isFacingRight = false;

        // Hold position during attack animation (vx = 0)
        if (animAttack.IsValid()) {
            animAttack.Update();
            animFrame = animAttack.GetCurrentFrame();
            if (animAttack.IsFinished()) {
                state = (distToPlayer <= DETECTION_RANGE && dyToPlayer < 120.0) ? ENEMY_CHASE : ENEMY_PATROL;
                if (type == TYPE_RAIDER) {
                    attackCooldown = 40;
                    raiderBackstepTimer = 22; // Trigger step-backward evasion after attacking
                } else {
                    attackCooldown = (type == TYPE_RUNNER) ? 35 : ((type == TYPE_HEAVY) ? 75 : 60);
                }
                animFrame = 0;
                frameCounter = 0;
                stateTimer = 0;
                animAttack.Reset();
            }
        }
        else {
            state = (distToPlayer <= DETECTION_RANGE && dyToPlayer < 120.0) ? ENEMY_CHASE : ENEMY_PATROL;
            if (type == TYPE_RAIDER) {
                attackCooldown = 40;
                raiderBackstepTimer = 22;
            } else {
                attackCooldown = (type == TYPE_RUNNER) ? 35 : ((type == TYPE_HEAVY) ? 75 : 60);
            }
            animAttack.Reset();
        }
    }
    else {
        // --------------------------------------------------------------------
        // SPECIAL RAIDER COMBAT AI: Tactical Pacing, Katana Avoidance & Opening Counters
        // --------------------------------------------------------------------
        if (type == TYPE_RAIDER && distToPlayer <= DETECTION_RANGE && dyToPlayer < 120.0) {
            // 1. Post-attack Step Backward Evasion
            if (raiderBackstepTimer > 0) {
                raiderBackstepTimer--;
                state = ENEMY_CHASE;
                isFacingRight = (playerX > x);
                double backstepSpeed = 2.4;
                if (playerX > x) x -= backstepSpeed;
                else x += backstepSpeed;

                if (animWalk.IsValid()) {
                    animWalk.Update();
                    animFrame = animWalk.GetCurrentFrame();
                }
                return;
            }

            // 2. Katana Avoidance & Tactical Pacing (Outside immediate attack range 70px - 140px)
            if (distToPlayer > ATTACK_RANGE && distToPlayer <= 140.0) {
                isFacingRight = (playerX > x);
                // If Arin is currently swinging his katana, Raider stops/steps back outside reach
                if (playerIsAttacking) {
                    state = ENEMY_CHASE;
                    if (distToPlayer < 115.0) {
                        double stepBack = 1.8;
                        if (playerX > x) x -= stepBack;
                        else x += stepBack;
                    }
                    if (animIdle.IsValid()) {
                        animIdle.Update();
                        animFrame = animIdle.GetCurrentFrame();
                    }
                    else if (animWalk.IsValid()) {
                        animWalk.Update();
                        animFrame = animWalk.GetCurrentFrame();
                    }
                    return;
                }
                // If Arin is NOT swinging and Raider is ready, surge forward to punish the opening!
                else if (attackCooldown <= 0) {
                    state = ENEMY_CHASE;
                    double surgeSpeed = 3.6;
                    if (playerX > x + 8.0) x += surgeSpeed;
                    else if (playerX < x - 8.0) x -= surgeSpeed;

                    if (distToPlayer <= ATTACK_RANGE) {
                        state = ENEMY_ATTACK;
                        hasDealtDamage = false;
                        animFrame = 0;
                        frameCounter = 0;
                        stateTimer = 0;
                        animAttack.Reset();
                        return;
                    }

                    if (animWalk.IsValid()) {
                        animWalk.Update();
                        animFrame = animWalk.GetCurrentFrame();
                    }
                    return;
                }
            }
        }

        // Detect if player is within attack range
        if (distToPlayer <= ATTACK_RANGE && dyToPlayer < 120.0) {
            inAttackRange = true;
            // Face player when close with hysteresis threshold
            if (playerX > x + 8.0) isFacingRight = true;
            else if (playerX < x - 8.0) isFacingRight = false;

            if (attackCooldown <= 0) {
                state = ENEMY_ATTACK;
                hasDealtDamage = false;
                animFrame = 0;
                frameCounter = 0;
                stateTimer = 0;
                animAttack.Reset();
            }
            else {
                // Stand facing player while on attack cooldown
                state = ENEMY_CHASE;
                if (animIdle.IsValid()) {
                    animIdle.Update();
                    animFrame = animIdle.GetCurrentFrame();
                }
                else if (animWalk.IsValid()) {
                    animWalk.Update();
                    animFrame = animWalk.GetCurrentFrame();
                }
            }
        }
        else if (distToPlayer <= DETECTION_RANGE && dyToPlayer < 120.0) {
            inAttackRange = false;
            // Actively chase Arin when within detection range
            state = ENEMY_CHASE;
            double speed = (type == TYPE_RUNNER) ? 4.2 : ((type == TYPE_RAIDER) ? 1.8 : ((type == TYPE_HEAVY) ? 1.1 : 1.5));
            if (playerX > x + 8.0) {
                x += speed;
                isFacingRight = true;
            }
            else if (playerX < x - 8.0) {
                x -= speed;
                isFacingRight = false;
            }
            else {
                if (playerX > x) x += speed;
                else x -= speed;
            }

            if (animWalk.IsValid()) {
                animWalk.Update();
                animFrame = animWalk.GetCurrentFrame();
            }
        }
        else {
            inAttackRange = false;
            // Outside detection range: stay in patrol / idle
            state = ENEMY_PATROL;
            if (startX != endX && animWalk.IsValid()) {
                if (isFacingRight) {
                    x += (type == TYPE_RUNNER ? 2.0 : (type == TYPE_HEAVY ? 0.8 : 1.0));
                    if (x >= endX) isFacingRight = false;
                } else {
                    x -= (type == TYPE_RUNNER ? 2.0 : (type == TYPE_HEAVY ? 0.8 : 1.0));
                    if (x <= startX) isFacingRight = true;
                }
                animWalk.Update();
                animFrame = animWalk.GetCurrentFrame();
            }
            else if (animIdle.IsValid()) {
                animIdle.Update();
                animFrame = animIdle.GetCurrentFrame();
            }
            else if (animWalk.IsValid()) {
                animWalk.Update();
                animFrame = animWalk.GetCurrentFrame();
            }
        }
    }

    if (state != oldState) {
        if (state == ENEMY_PATROL) {
            if (startX != endX && animWalk.IsValid()) animWalk.Reset();
            else animIdle.Reset();
        }
        else if (state == ENEMY_CHASE) animWalk.Reset();
        else if (state == ENEMY_ATTACK) { animAttack.Reset(); hasDealtDamage = false; }
        else if (state == ENEMY_HURT) animHurt.Reset();
        else if (state == ENEMY_DEAD) animDeath.Reset();
    }
}

// ============================================================================
// Render Pipeline (Viewport Relative)
// ============================================================================
void Enemy::Render(double camX, double camY) {
    // Proportional visual draw sizes: Abomination 256, Heavy 230, Walker/Runner/Raider 192
    int drawSize = (type == TYPE_ABOMINATION) ? 256 : ((type == TYPE_HEAVY) ? 230 : 192);

    double drawX = x - camX;
    double drawY = y - camY;

    // Aligns bottom center of drawing box to collision bounds & ground baseline
    double drawXOffset = drawX - (drawSize - width) / 2.0;
    double drawYOffset = (type == TYPE_HEAVY) ? (drawY + 2.0) : ((type == TYPE_SPITTER || type == TYPE_RUNNER || type == TYPE_RAIDER || type == TYPE_ABOMINATION) ? (drawY - 6.0) : drawY);

    // Select active animation based on state
    const Animation* activeAnim = &animIdle;
    if (state == ENEMY_DEAD) {
        if (animDeath.IsValid()) activeAnim = &animDeath;
        else if (animHurt.IsValid()) activeAnim = &animHurt;
        else activeAnim = &animIdle;
    }
    else if (state == ENEMY_HURT && animHurt.IsValid()) {
        activeAnim = &animHurt;
    }
    else if (state == ENEMY_ATTACK && animAttack.IsValid()) {
        activeAnim = &animAttack;
    }
    else if (state == ENEMY_CHASE) {
        if (inAttackRange && animIdle.IsValid()) {
            activeAnim = &animIdle;
        }
        else if (animWalk.IsValid()) {
            activeAnim = &animWalk;
        }
    }
    else if (state == ENEMY_PATROL) {
        if (startX != endX && animWalk.IsValid()) {
            activeAnim = &animWalk;
        }
        else if (animIdle.IsValid()) {
            activeAnim = &animIdle;
        }
        else if (animWalk.IsValid()) {
            activeAnim = &animWalk;
        }
    }

    if (activeAnim != nullptr && activeAnim->IsValid()) {
        activeAnim->Render((int)drawXOffset, (int)drawYOffset, drawSize, drawSize, isFacingRight);
    }
    else {
        // High visibility fallback silhouette so enemies are always visible
        double px = drawXOffset;
        double py = drawYOffset;
        if (type == TYPE_SPITTER) {
            iSetColor(50, 120, 60);
            iFilledRectangle((int)px + 64, (int)py + 20, 64, 100);
            iSetColor(120, 200, 90);
            iFilledRectangle((int)px + 76, (int)py + 120, 40, 40);
            iSetColor(255, 30, 30);
            iFilledRectangle((int)px + (isFacingRight ? 100 : 80), (int)py + 138, 10, 8);
        } else {
            iSetColor(160, 40, 40);
            iFilledRectangle((int)px + 32, (int)py + 10, 64, 80);
            iSetColor(220, 80, 80);
            iFilledRectangle((int)px + 44, (int)py + 90, 40, 30);
            iSetColor(255, 220, 0);
            iFilledRectangle((int)px + (isFacingRight ? 68 : 48), (int)py + 102, 10, 8);
        }
    }
}

// ============================================================================
// Health & Collision System
// ============================================================================
void Enemy::TakeDamage(int amount) {
    if (state == ENEMY_DEAD) return;

    hp -= amount;
    if (hp <= 0) {
        hp = 0;
        state = ENEMY_DEAD;
        animDeath.Reset();
    }
    else {
        state = ENEMY_HURT;
        animHurt.Reset();
    }
}

bool Enemy::CheckPlayerCollision(double px, double py, int pw, int ph) {
    if (hp <= 0) return false;

    bool collisionX = (x + width >= px) && (px + pw >= x);
    bool collisionY = (y + height >= py) && (py + ph >= y);

    return collisionX && collisionY;
}

