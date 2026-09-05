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
    TYPE_HEAVY,        // Heavy Infected
    TYPE_HUNTER,       // Fast, aggressive Infected Hunter
    TYPE_ALPHA_HUNTER  // Level 2 Boss - Alpha Hunter
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
    double vy; // Vertical velocity for jumping
    bool isGrounded; // Grounded state
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
    int raiderBackstepTimer;
    int raiderPauseTimer;
    int avoidTimer;
    int avoidDirection;
    int walkerHurtAudioCooldown;
    int runnerIdleTimer;
    int runnerHurtAudioCooldown;
    int raiderHurtAudioCooldown;
    static int s_globalWalkerAttackAudioCooldown;
    static int s_globalWalkerHurtAudioCooldown;
    static int s_globalRunnerAttackAudioCooldown;
    static int s_globalRunnerHurtAudioCooldown;
    static int s_globalRaiderAttackAudioCooldown;
    static int s_globalRaiderHurtAudioCooldown;
    // Static Texture Handles (Cached across all instances)
    static unsigned int texWalkerIdle, texWalkerWalk, texWalkerAttack, texWalkerHurt, texWalkerDeath;
    static unsigned int texRunnerIdle, texRunnerWalk, texRunnerRun, texRunnerAttack, texRunnerHurt, texRunnerDeath;
    static unsigned int texRaiderIdle, texRaiderWalk, texRaiderRun, texRaiderAttackMelee, texRaiderAttackRanged, texRaiderHurt, texRaiderDeath;
    static unsigned int texHeavyIdle, texHeavyWalk, texHeavyAttack, texHeavyHurt, texHeavyDeath;
    static unsigned int texHunterIdle, texHunterWalk, texHunterRun, texHunterAttack, texHunterHurt, texHunterDeath;

    // Static Frame Sequence Vectors (Walker, Runner, Raider, Heavy & Abomination)
    static std::vector<unsigned int> seqWalkerIdle, seqWalkerWalk, seqWalkerAttack, seqWalkerHurt, seqWalkerDeath;
    static std::vector<unsigned int> seqRunnerIdle, seqRunnerWalk, seqRunnerRun, seqRunnerAttack, seqRunnerHurt, seqRunnerDeath;
    static std::vector<unsigned int> seqRaiderIdle, seqRaiderWalk, seqRaiderAttack, seqRaiderHurt, seqRaiderDeath;
    static std::vector<unsigned int> seqHeavyIdle, seqHeavyWalk, seqHeavyAttack, seqHeavyHurt, seqHeavyDeath;
    static std::vector<unsigned int> seqAbominationIdle, seqAbominationWalk, seqAbominationAttack, seqAbominationHurt, seqAbominationDeath;
    static std::vector<unsigned int> seqHunterIdle, seqHunterWalk, seqHunterRun, seqHunterAttack, seqHunterHurt, seqHunterDeath;

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
    static void PreloadAllTextures();
    void Update(double playerX, double playerY, bool playerIsAttacking = false);
    void Render(double camX, double camY);
    void TakeDamage(int damage);
    bool CheckPlayerCollision(double px, double py, int pw, int ph);
};

#endif // ENEMY_H