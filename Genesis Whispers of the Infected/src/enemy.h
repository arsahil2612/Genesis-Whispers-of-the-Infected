#ifndef ENEMY_H
#define ENEMY_H

#include <vector>
#include "animation.h"

// ============================================================================
// Enemy Taxonomy & State Enums
// ============================================================================
enum EnemyType {
    TYPE_RUNNER,
    TYPE_SPITTER,      // Represents Infected Walker
    TYPE_ABOMINATION,  // Mutated Brute (Mini Boss)
    TYPE_RAIDER,       // Raider
    TYPE_HEAVY         // Heavy Infected
};

enum EnemyState {
    ENEMY_PATROL,
    ENEMY_CHASE,
    ENEMY_ATTACK,
    ENEMY_HURT,
    ENEMY_DEAD
};

// Mutated Brute (mini boss) attack types
enum BruteAttackType {
    BRUTE_PUNCH,
    BRUTE_SLAM,
    BRUTE_CHARGE
};

// ============================================================================
// Enemy Class Definition
// ============================================================================
class Enemy {
public:
    // Transform & Bounds
    double x, y;
    double startX, endX; // Patrol bounds
    double vx;
    int width, height;

    // Attributes & State Flags
    int hp;
    int maxHp;
    int damage;
    bool isFacingRight;
    bool inAttackRange;
    bool hasDealtDamage;

    EnemyType type;
    EnemyState state;

    int animFrame;
    int frameCounter;

    // AI Timers & Special Attack Flags
    int attackCooldown;
    int stateTimer;
    bool rangedShotFired;
    int lastHitAttackID;
    BruteAttackType bruteAttack;
    int bruteAttackTimer;

    // Static Texture Handles (Cached across all instances)
    static unsigned int texWalkerIdle, texWalkerWalk, texWalkerAttack, texWalkerHurt, texWalkerDeath;
    static unsigned int texRunnerIdle, texRunnerWalk, texRunnerRun, texRunnerAttack, texRunnerHurt, texRunnerDeath;
    static unsigned int texRaiderIdle, texRaiderWalk, texRaiderRun, texRaiderAttackMelee, texRaiderAttackRanged, texRaiderHurt, texRaiderDeath;
    static unsigned int texHeavyIdle, texHeavyWalk, texHeavyAttack, texHeavyHurt, texHeavyDeath;

    // Static Frame Sequence Vectors (Walker, Runner, Raider, Heavy & Abomination)
    static std::vector<unsigned int> seqWalkerIdle, seqWalkerWalk, seqWalkerAttack, seqWalkerHurt, seqWalkerDeath;
    static std::vector<unsigned int> seqRunnerIdle, seqRunnerWalk, seqRunnerRun, seqRunnerAttack, seqRunnerHurt, seqRunnerDeath;
    static std::vector<unsigned int> seqRaiderIdle, seqRaiderWalk, seqRaiderAttack, seqRaiderHurt, seqRaiderDeath;
    static std::vector<unsigned int> seqHeavyIdle, seqHeavyWalk, seqHeavyAttack, seqHeavyHurt, seqHeavyDeath;
    static std::vector<unsigned int> seqAbominationIdle, seqAbominationWalk, seqAbominationAttack, seqAbominationHurt, seqAbominationDeath;

    // Reusable Animation Objects
    Animation animIdle;
    Animation animWalk;
    Animation animAttack;
    Animation animHurt;
    Animation animDeath;

    // ========================================================================
    // Member Methods
    // ========================================================================
    Enemy(double startX, double endX, double y, EnemyType type);
    void Update(double playerX, double playerY);
    void Render(double camX, double camY);
    void TakeDamage(int damage);
    bool CheckPlayerCollision(double px, double py, int pw, int ph);
};

#endif // ENEMY_H