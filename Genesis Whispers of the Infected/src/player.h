#ifndef PLAYER_H
#define PLAYER_H

#include <vector>
#include <chrono>
#include "animation.h"

// ============================================================================
// Player Character Animation & Physics States
// ============================================================================
enum PlayerState {
    STATE_IDLE,
    STATE_WALK,
    STATE_RUN,
    STATE_JUMP,
    STATE_ATTACK_MELEE,
    STATE_HURT,
    STATE_DEAD
};

// ============================================================================
// Player Class Definition
// ============================================================================
class Player {
public:
    // Transform & Movement Physics
    double x, y;
    double vx, vy;
    int width, height;

    // Attributes & Inventory
    int hp;
    int maxHp;
    double displayedHp;
    int ammo;
    int medkits;
    int foodCount;
    int batteryCount;
    int scrapCount;
    int waterBottleCount;
    int stamina;
    int maxStamina;
    double staminaDouble;
    double displayedStamina;
    double staminaRegenDelayTimer;
    bool isExhausted;

    // Grounding & Orientation Flags
    bool isGrounded;
    bool wasJumpPressed;
    bool wasAttackPressed;
    bool isFacingRight;
    bool isInvulnerable;
    int invulnerabilityTimer;

    // Combat Tracking Flags & Timers
    double attackCooldownTimer;
    bool hasDealtDamageThisAttack;
    bool hasPlayedMissSoundThisAttack;
    bool rangedAttackTriggered;
    int currentAttackID;

    // State & Animation Timing
    PlayerState state;
    int currentAnimationFrame;
    double footstepTimer;

    // Reusable Animation Objects
    Animation animIdle;
    Animation animWalk;
    Animation animRun;
    Animation animJump;
    Animation animAttack;
    Animation animHurt;
    Animation animDeath;

    // ========================================================================
    // Member Methods
    // ========================================================================
    Player();
    void Initialize(double startX, double startY);
    void Update(bool keys[], bool specialKeys[]);
    void Render(double camX, double camY);
    void TakeDamage(int damage);
    void AttackMelee();
    void AttackRanged();
    void UseHeal();
    void UseFood();
    void UseWaterBottle();
    void SetState(PlayerState newState);
    void ResetInputState();
};

#endif // PLAYER_H
