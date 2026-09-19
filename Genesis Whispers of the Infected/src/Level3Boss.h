#ifndef LEVEL3_BOSS_H
#define LEVEL3_BOSS_H

#include "animation.h"
#include "player.h"
#include <vector>
#include <string>

enum Level3BossPhase {
    L3_BOSS_INACTIVE,
    L3_BOSS_HUMAN_DIALOGUE,
    L3_BOSS_HUMAN_ANGRY_DRONES,
    L3_BOSS_HUMAN_INJECT_SERUM,
    L3_BOSS_HUMAN_TRANSFORMING,
    L3_BOSS_MONSTER_ACTIVE,
    L3_BOSS_DEFEATED
};

enum MonsterState {
    MONSTER_IDLE,
    MONSTER_CHASE,
    MONSTER_ATTACK_CLAW,
    MONSTER_ATTACK_SPIKES,
    MONSTER_ATTACK_CHARGE,
    MONSTER_SUMMON_DRONES,
    MONSTER_STAGGER,
    MONSTER_DEAD
};

struct Drone {
    double x, y;
    double vx, vy;
    double targetX, targetY;
    int hp;
    bool active;
    double timer;
    double attackCooldown;
    Animation anim;
    
    Drone() : x(0), y(0), vx(0), vy(0), targetX(0), targetY(0), hp(30), active(false), timer(0), attackCooldown(0) {}
};

struct DroneProjectile {
    double x, y;
    double vx, vy;
    int damage;
    bool active;
    Animation anim;

    DroneProjectile() : x(0), y(0), vx(0), vy(0), damage(10), active(false) {}
};

struct SpikeEffect {
    double x, y;
    double timer;
    bool isWarning; // true = purple warning circle, false = spike eruption
    bool damageDealt;
    Animation animWarning;
    Animation animSpike;

    SpikeEffect() : x(0), y(0), timer(0), isWarning(true), damageDealt(false) {}
};

class Level3Boss {
public:
    double x, y;
    int width, height;
    int hp;
    int maxHp;
    bool isFacingRight;
    Level3BossPhase phase;
    MonsterState monsterState;

    // Phase 1 (Human) Dialogue & Intro State
    int dialogueStep;
    double dialogueTimer;
    double introTimer;
    double transformTimer;
    std::string currentSpeaker;
    std::string currentText;

    // Sub-systems
    std::vector<Drone> drones;
    std::vector<DroneProjectile> droneProjectiles;
    std::vector<SpikeEffect> spikes;

    // Human Kael Animations
    Animation animHumanIdle;
    Animation animHumanTalk;
    Animation animHumanSummon;
    Animation animHumanInject;
    Animation animHumanTransform;

    // Monster Kael Animations
    Animation animMonsterIdle;
    Animation animMonsterWalk;
    Animation animMonsterClaw;
    Animation animMonsterHurt;
    Animation animMonsterStagger;
    Animation animMonsterWarning;
    Animation animMonsterSpike;
    Animation animMonsterCharge;

    // AI Timers
    double attackCooldownTimer;
    double stateTimer;
    bool clawDamageDealt;
    bool chargeDamageDealt;
    double chargeStartX;
    double chargeTargetX;

public:
    Level3Boss();
    
    void Initialize(double arenaX, double arenaY);
    void Update(Player& player, float dt);
    void Render(double camX, double camY);

    void TakeDamage(int damage);
    bool CheckPlayerCollision(double px, double py, int pw, int ph, Player& player);

    void AdvanceDialogue();
    bool IsInDialogue() const { return phase == L3_BOSS_HUMAN_DIALOGUE; }
    bool IsTransforming() const { return phase == L3_BOSS_HUMAN_INJECT_SERUM || phase == L3_BOSS_HUMAN_TRANSFORMING; }
    bool IsActiveMonster() const { return phase == L3_BOSS_MONSTER_ACTIVE; }
    bool IsDefeated() const { return phase == L3_BOSS_DEFEATED; }
    
    void SpawnDrone(double spawnX, double spawnY);
    void TriggerSpikeAttack(double targetX, double targetY);
};

#endif // LEVEL3_BOSS_H
