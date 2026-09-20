#ifndef PLAYER_H
#define PLAYER_H

#include <vector>
#include <chrono>
#include "animation.h"

// SMG Configurable Weapon Parameters
const int kSmgDamage = 28;
const double kSmgFireInterval = 0.12; // Cooldown interval in seconds (0.10 - 0.15s)
const int kSmgMaxMag = 30;
const int kSmgMaxReserve = 120;

// Shotgun Configurable Weapon Parameters
const int kShotgunMaxMag = 6;
const int kShotgunMaxReserve = 24;
const int kShotgunPelletsPerShot = 6;
const int kShotgunPelletDamage = 20;
const double kShotgunFireCooldown = 0.8;

// Weapon Selection Taxonomy
enum WeaponType {
    WEAPON_KATANA = 1,
    WEAPON_PISTOL = 2,
    WEAPON_SMG = 3,
    WEAPON_GRENADE = 4,
    WEAPON_SHOTGUN = 5
};

// ============================================================================
// Player Character Animation & Physics States
// ============================================================================
enum PlayerState {
    STATE_IDLE,
    STATE_WALK,
    STATE_RUN,
    STATE_JUMP,
    STATE_ATTACK_MELEE,
    STATE_ATTACK_PISTOL,
    STATE_ATTACK_SMG,
    STATE_RELOAD_SMG,
    STATE_ATTACK_GRENADE,
    STATE_ATTACK_SHOTGUN,
    STATE_RELOAD_SHOTGUN,
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

    // Weapon & SMG & Grenade & Shotgun System Attributes
    WeaponType currentWeapon;
    bool hasSMG;
    int smgMag;
    int smgReserve;
    double smgFireCooldownTimer;
    double reloadTimer;
    bool hasGrenade;
    int grenadeCount;
    bool grenadeSpawnedThisThrow;
    bool hasShotgun;
    int shotgunMag;
    int shotgunReserve;
    double shotgunFireCooldownTimer;
    double shotgunReloadTimer;

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
    Animation animPistol;
    Animation animSMG;
    Animation animSMGReload;
    Animation animGrenadeThrow;
    Animation animShotgun;
    Animation animShotgunReload;
    Animation animHurt;
    Animation animDeath;

    // ========================================================================
    // Member Methods
    // ========================================================================
    Player();
    void Initialize(double startX, double startY);
    void Update(double dt = 0.016667, bool keys[] = NULL, bool specialKeys[] = NULL);
    void Render(double camX, double camY);
    void TakeDamage(int damage);
    void AttackMelee();
    void AttackRanged();
    void AttackSMG();
    void AttackGrenade();
    void AttackShotgun();
    void ReloadWeapon();
    void ReloadShotgun();
    void SwitchWeapon(WeaponType type);
    void UseHeal();
    void UseFood();
    void UseWaterBottle();
    void SetState(PlayerState newState);
    void ResetInputState();
};

#endif // PLAYER_H
