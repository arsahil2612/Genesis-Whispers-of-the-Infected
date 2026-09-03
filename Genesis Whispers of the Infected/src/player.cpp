#include "player.h"
#include "igraphics_declarations.h"
#include <vector>
#include <chrono>
#include <cmath>
#include <windows.h>
#include <GL/gl.h>

#ifndef GLUT_KEY_LEFT
#define GLUT_KEY_LEFT 100
#define GLUT_KEY_UP 101
#define GLUT_KEY_RIGHT 102
#define GLUT_KEY_DOWN 103
#endif

// ============================================================================
// Constructor
// ============================================================================
Player::Player() {
    x = 150;
    y = 185;
    vx = 0;
    vy = 0;
    width = 64;
    height = 96;
    hp = maxHp = 100;
    displayedHp = 100.0;
    ammo = 30;
    medkits = 2;
    foodCount = 1;
    batteryCount = 1;
    scrapCount = 0;
    waterBottleCount = 2;
    staminaDouble = 100.0;
    stamina = maxStamina = 100;
    displayedStamina = 100.0;
    staminaRegenDelayTimer = 0.0;
    isExhausted = false;
    isGrounded = true;
    wasJumpPressed = false;
    isFacingRight = true;
    isInvulnerable = false;
    invulnerabilityTimer = 0;
    attackCooldownTimer = 0.0;
    hasDealtDamageThisAttack = false;
    rangedAttackTriggered = false;
    currentAttackID = 0;
    state = STATE_IDLE;
    currentAnimationFrame = 0;
}

// ============================================================================
// Initialization & Asset Preloading
// ============================================================================
void Player::Initialize(double startX, double startY) {
    x = startX;
    y = startY;
    vx = 0;
    vy = 0;
    hp = maxHp;
    displayedHp = (double)hp;
    ammo = 15;
    medkits = 2;
    foodCount = 1;
    batteryCount = 1;
    scrapCount = 0;
    waterBottleCount = 2;
    staminaDouble = 100.0;
    stamina = maxStamina = 100;
    displayedStamina = (double)stamina;
    staminaRegenDelayTimer = 0.0;
    isExhausted = false;
    isGrounded = true;
    wasJumpPressed = false;
    wasAttackPressed = false;
    isFacingRight = true;
    isInvulnerable = false;
    invulnerabilityTimer = 0;
    attackCooldownTimer = 0.0;
    hasDealtDamageThisAttack = false;
    rangedAttackTriggered = false;
    currentAttackID = 0;
    currentAnimationFrame = 0;

    // Load Arin full multi-frame animation sequences from asset directory statically once
    static std::vector<unsigned int> seqIdle, seqWalk, seqRun, seqJump, seqAttack, seqHurt, seqDeath;

    if (seqIdle.empty()) {
        for (int i = 1; i <= 6; ++i) {
            char path[256];
            sprintf_s(path, sizeof(path), "Assets/Characters/Arin/Idle/arin_idle_sprite_%02d.png", i);
            seqIdle.push_back(iLoadImage((char*)GetAssetPath(path).c_str()));
        }

        for (int i = 1; i <= 7; ++i) {
            char path[256];
            sprintf_s(path, sizeof(path), "Assets/Characters/Arin/Walk/arin_walk_sprite_%02d.png", i);
            seqWalk.push_back(iLoadImage((char*)GetAssetPath(path).c_str()));
        }

        for (int i = 1; i <= 8; ++i) {
            char path[256];
            sprintf_s(path, sizeof(path), "Assets/Characters/Arin/Run/arin_run_sprite_%02d.png", i);
            unsigned int tex = iLoadImage((char*)GetAssetPath(path).c_str());
            if (tex == 0) {
                sprintf_s(path, sizeof(path), "Assets/Characters/Arin/Run/run.png");
                tex = iLoadImage((char*)GetAssetPath(path).c_str());
            }
            if (tex == 0) {
                sprintf_s(path, sizeof(path), "Assets/Characters/Arin/run.png");
                tex = iLoadImage((char*)GetAssetPath(path).c_str());
            }
            if (tex == 0) {
                sprintf_s(path, sizeof(path), "run.png");
                tex = iLoadImage((char*)GetAssetPath(path).c_str());
            }
            if (tex != 0) {
                seqRun.push_back(tex);
            }
        }

        // Safety fallback: Ensure seqRun is never empty so animRun is always valid when Shift is pressed
        if (seqRun.empty()) {
            unsigned int singleRun = iLoadImage((char*)GetAssetPath("Assets/Characters/Arin/Run/run.png").c_str());
            if (singleRun == 0) singleRun = iLoadImage((char*)GetAssetPath("Assets/Characters/Arin/run.png").c_str());
            if (singleRun == 0) singleRun = iLoadImage((char*)GetAssetPath("run.png").c_str());

            if (singleRun != 0) {
                seqRun.push_back(singleRun);
            } else {
                seqRun = seqWalk;
            }
        }

        for (int i = 1; i <= 6; ++i) {
            char path[256];
            sprintf_s(path, sizeof(path), "Assets/Characters/Arin/Jump/arin_jump_sprite_%02d.png", i);
            unsigned int tex = iLoadImage((char*)GetAssetPath(path).c_str());
            if (tex == 0) {
                sprintf_s(path, sizeof(path), "Assets/Characters/Arin/Jump/jump.png");
                tex = iLoadImage((char*)GetAssetPath(path).c_str());
            }
            if (tex == 0) {
                sprintf_s(path, sizeof(path), "Assets/Characters/Arin/jump.png");
                tex = iLoadImage((char*)GetAssetPath(path).c_str());
            }
            if (tex == 0) {
                sprintf_s(path, sizeof(path), "jump.png");
                tex = iLoadImage((char*)GetAssetPath(path).c_str());
            }
            if (tex != 0) {
                seqJump.push_back(tex);
            }
        }

        // Safety fallback: Ensure seqJump is never empty so animJump is always valid while airborne
        if (seqJump.empty()) {
            unsigned int singleJump = iLoadImage((char*)GetAssetPath("Assets/Characters/Arin/Jump/jump.png").c_str());
            if (singleJump == 0) singleJump = iLoadImage((char*)GetAssetPath("Assets/Characters/Arin/jump.png").c_str());
            if (singleJump == 0) singleJump = iLoadImage((char*)GetAssetPath("jump.png").c_str());

            if (singleJump != 0) {
                seqJump.push_back(singleJump);
            } else {
                seqJump = seqIdle;
            }
        }

        for (int i = 1; i <= 8; ++i) {
            char path[256];
            sprintf_s(path, sizeof(path), "Assets/Characters/Arin/Attack/arin_attack_sprite_%02d.png", i);
            unsigned int tex = iLoadImage((char*)GetAssetPath(path).c_str());
            if (tex == 0) {
                sprintf_s(path, sizeof(path), "Assets/Characters/Arin/Attack/attack.png");
                tex = iLoadImage((char*)GetAssetPath(path).c_str());
            }
            if (tex == 0) {
                sprintf_s(path, sizeof(path), "Assets/Characters/Arin/attack.png");
                tex = iLoadImage((char*)GetAssetPath(path).c_str());
            }
            if (tex == 0) {
                sprintf_s(path, sizeof(path), "attack.png");
                tex = iLoadImage((char*)GetAssetPath(path).c_str());
            }
            if (tex != 0) {
                seqAttack.push_back(tex);
            }
        }

        // Safety fallback: Ensure seqAttack is never empty so animAttack is always valid
        if (seqAttack.empty()) {
            unsigned int singleAttack = iLoadImage((char*)GetAssetPath("Assets/Characters/Arin/Attack/attack.png").c_str());
            if (singleAttack == 0) singleAttack = iLoadImage((char*)GetAssetPath("Assets/Characters/Arin/attack.png").c_str());
            if (singleAttack == 0) singleAttack = iLoadImage((char*)GetAssetPath("attack.png").c_str());

            if (singleAttack != 0) {
                seqAttack.push_back(singleAttack);
            } else {
                seqAttack = seqIdle;
            }
        }

        for (int i = 1; i <= 4; ++i) {
            char path[256];
            sprintf_s(path, sizeof(path), "Assets/Characters/Arin/Hurt/arin_hurt_sprite_%02d.png", i);
            seqHurt.push_back(iLoadImage((char*)GetAssetPath(path).c_str()));
        }

        for (int i = 1; i <= 7; ++i) {
            char path[256];
            sprintf_s(path, sizeof(path), "Assets/Characters/Arin/Death/arin_death_sprite_%02d.png", i);
            unsigned int tex = iLoadImage((char*)GetAssetPath(path).c_str());
            if (tex == 0) {
                sprintf_s(path, sizeof(path), "Assets/Characters/Arin/Death/death.png");
                tex = iLoadImage((char*)GetAssetPath(path).c_str());
            }
            if (tex == 0) {
                sprintf_s(path, sizeof(path), "Assets/Characters/Arin/death.png");
                tex = iLoadImage((char*)GetAssetPath(path).c_str());
            }
            if (tex == 0) {
                sprintf_s(path, sizeof(path), "death.png");
                tex = iLoadImage((char*)GetAssetPath(path).c_str());
            }
            if (tex != 0) {
                seqDeath.push_back(tex);
            }
        }

        // Safety fallback: Ensure seqDeath is never empty so animDeath is always valid upon death
        if (seqDeath.empty()) {
            unsigned int singleDeath = iLoadImage((char*)GetAssetPath("Assets/Characters/Arin/Death/death.png").c_str());
            if (singleDeath == 0) singleDeath = iLoadImage((char*)GetAssetPath("Assets/Characters/Arin/death.png").c_str());
            if (singleDeath == 0) singleDeath = iLoadImage((char*)GetAssetPath("death.png").c_str());

            if (singleDeath != 0) {
                seqDeath.push_back(singleDeath);
            } else {
                seqDeath = seqHurt;
            }
        }
    }

    // Initialize Animation instances with tuned consistent frame tick durations:
    // Idle: 6 frames @ 8 ticks (looping), Walk: 7 frames @ 5 ticks (looping), Run: 8 frames @ 3 ticks (looping),
    // Jump: 6 frames @ 5 ticks (non-looping airborne sequence), Attack: 8 frames @ 3 ticks (non-looping 0.4s),
    // Hurt: 4 frames @ 5 ticks (non-looping 0.4s), Death: 7 frames @ 7 ticks (non-looping)
    animIdle.Init(seqIdle, 8, true);
    animWalk.Init(seqWalk, 5, true);
    animRun.Init(seqRun, 3, true);
    animJump.Init(seqJump, 5, false);
    animAttack.Init(seqAttack, 3, false);
    animHurt.Init(seqHurt, 5, false);
    animDeath.Init(seqDeath, 7, false);

    state = STATE_IDLE;
    animIdle.Reset();
}

// ============================================================================
// State Switcher Helper Method
// ============================================================================
void Player::SetState(PlayerState newState) {
    if (state == newState) return;

    state = newState;
    currentAnimationFrame = 0;

    // Reset target animation to frame 0 with tick counter cleared for flicker-free sequential playback
    switch (state) {
    case STATE_IDLE:          animIdle.Reset(); break;
    case STATE_WALK:          animWalk.Reset(); break;
    case STATE_RUN:           animRun.Reset(); break;
    case STATE_JUMP:          animJump.Reset(); break;
    case STATE_ATTACK_MELEE:  animAttack.Reset(); break;
    case STATE_HURT:          animHurt.Reset(); break;
    case STATE_DEAD:          animDeath.Reset(); break;
    }
}

// ============================================================================
// Physics Movement & State Machine Update
// ============================================================================
void Player::Update(bool keys[], bool specialKeys[]) {
    // 1. Timestep updates using fixed 60 FPS timestep (0.016667s) for Physics stability
    double dt = 0.016667;

    // Update invulnerability timer after taking damage
    if (isInvulnerable) {
        invulnerabilityTimer--;
        if (invulnerabilityTimer <= 0) {
            invulnerabilityTimer = 0;
            isInvulnerable = false;
        }
    }

    // Update 0.4s melee attack cooldown timer
    if (attackCooldownTimer > 0.0) {
        attackCooldownTimer -= dt;
        if (attackCooldownTimer < 0.0) {
            attackCooldownTimer = 0.0;
        }
    }

    // 2. Physics movement constants (px/sec & px/sec^2) - Tuned for smooth responsive control
    const double WALK_SPEED = 180.0;    // Walking target speed
    const double RUN_SPEED = 320.0;     // Running target speed
    const double ACCELERATION = 2400.0; // Acceleration rate (px/sec^2) for smooth speed ramp-up
    const double DECELERATION = 3600.0; // Deceleration rate (px/sec^2) for crisp stopping (no ice-sliding)
    const double TURN_ACCEL = 4800.0;   // Turnaround rate (px/sec^2) when reversing direction

    // 3. Jump System: Natural jump arc, variable jump height, apex floatiness, no double jump
    bool physSpaceDown = ((GetAsyncKeyState(VK_SPACE) & 0x8000) != 0);
    bool jumpPressed = (keys != NULL && (keys[' '] || keys[32])) || physSpaceDown;

    if (!physSpaceDown) {
        if (keys != NULL) {
            keys[' '] = false;
            keys[32] = false;
        }
        wasJumpPressed = false;
    }

    if (jumpPressed && !wasJumpPressed && isGrounded && state != STATE_ATTACK_MELEE && state != STATE_DEAD && state != STATE_HURT) {
        if (staminaDouble >= 10.0) {
            vy = 520.0; // Initial smooth upward launch velocity (px/sec)
            isGrounded = false;
            SetState(STATE_JUMP);
            staminaDouble -= 10.0;
            if (staminaDouble <= 0.0) {
                staminaDouble = 0.0;
                isExhausted = true;
            }
            staminaRegenDelayTimer = 1.0;
        }
    }
    wasJumpPressed = jumpPressed;

    // 3b. Katana Melee Attack System: Triggered by J key (Edge Detected Single Key Press)
    bool attackPressed = keys['j'] || keys['J'];
    if (attackPressed && !wasAttackPressed && attackCooldownTimer <= 0.0 && state != STATE_ATTACK_MELEE && state != STATE_DEAD && state != STATE_HURT) {
        SetState(STATE_ATTACK_MELEE);
        attackCooldownTimer = 0.45;
        hasDealtDamageThisAttack = false;
        currentAttackID++;
    }
    wasAttackPressed = attackPressed;

    // Apply gravity & natural jump arc physics when airborne
    if (!isGrounded) {
        const double BASE_GRAVITY = 1300.0; // px/sec^2
        double gravityScale = 1.0;

        // Variable jump height: cut upward velocity if space bar is released early during ascent
        if (!jumpPressed && vy > 120.0) {
            vy *= 0.88;
        }

        // Apex floatiness: near top of jump arc (vy between -80 and +80), decrease gravity for weightless feel
        if (std::abs(vy) < 80.0) {
            gravityScale = 0.65;
        }
        // Falling descent: apply slightly higher gravity for snappy, responsive landing feel
        else if (vy < 0.0) {
            gravityScale = 1.25;
        }

        vy -= (BASE_GRAVITY * gravityScale) * dt;
        y += vy * dt;
    }

    // 4. Determine target horizontal velocity based on input (Hardware verified key state)
    bool moveLeft = (GetAsyncKeyState('A') & 0x8000) != 0 || (GetAsyncKeyState(VK_LEFT) & 0x8000) != 0;
    bool moveRight = (GetAsyncKeyState('D') & 0x8000) != 0 || (GetAsyncKeyState(VK_RIGHT) & 0x8000) != 0;
    bool isShiftHeld = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0 || (GetAsyncKeyState(VK_LSHIFT) & 0x8000) != 0;

    // Automatic Sprinting at Exit Gate (Level 1 Final Approach: 12800.0 <= x < 13050.0)
    bool isNearExitGate = (x >= 12800.0 && x < 13050.0);
    if (isNearExitGate && (state != STATE_DEAD && state != STATE_ATTACK_MELEE && state != STATE_HURT)) {
        moveRight = true;
        isFacingRight = true;
        isShiftHeld = true;
    }

    // Restrict sprinting if exhausted or zero stamina
    if (isExhausted || staminaDouble <= 0.0) {
        isShiftHeld = false;
    }

    bool isRunning = isShiftHeld && (moveLeft || moveRight);

    // Sprinting Stamina Drainage (18.0 stamina per second)
    if (isRunning) {
        staminaDouble -= 18.0 * dt;
        staminaRegenDelayTimer = 1.0;
        if (staminaDouble <= 0.0) {
            staminaDouble = 0.0;
            isExhausted = true;
            isRunning = false;
        }
    }

    // Stamina Auto-Regeneration when not sprinting (1.0s delay, 22.0 stamina/sec)
    if (!isRunning && isGrounded) {
        if (staminaRegenDelayTimer > 0.0) {
            staminaRegenDelayTimer -= dt;
            if (staminaRegenDelayTimer < 0.0) staminaRegenDelayTimer = 0.0;
        } else {
            staminaDouble += 22.0 * dt;
            if (staminaDouble > (double)maxStamina) {
                staminaDouble = (double)maxStamina;
            }
        }
    }

    // Clear exhaustion once stamina recovers above 15%
    if (isExhausted && staminaDouble >= 15.0) {
        isExhausted = false;
    }

    // Sync integer stamina value
    stamina = (int)std::round(staminaDouble);
    if (stamina < 0) stamina = 0;
    if (stamina > maxStamina) stamina = maxStamina;

    double targetSpeed = isRunning ? RUN_SPEED : WALK_SPEED;
    double targetVx = 0.0;

    // Temporarily stop horizontal movement while attacking, hurt, or dead
    if (state == STATE_ATTACK_MELEE || state == STATE_DEAD || state == STATE_HURT) {
        targetVx = 0.0;
        vx = 0.0;
    }
    else if (moveLeft && !moveRight) {
        targetVx = -targetSpeed;
        isFacingRight = false;
    }
    else if (moveRight && !moveLeft) {
        targetVx = targetSpeed;
        isFacingRight = true;
    }

    // 5. Smooth acceleration and deceleration physics
    if (targetVx != 0.0) {
        // Player is actively providing directional input
        if ((targetVx > 0.0 && vx < 0.0) || (targetVx < 0.0 && vx > 0.0)) {
            // Reversing direction: use responsive turnaround acceleration
            if (vx < targetVx) {
                vx += TURN_ACCEL * dt;
                if (vx > targetVx) vx = targetVx;
            }
            else {
                vx -= TURN_ACCEL * dt;
                if (vx < targetVx) vx = targetVx;
            }
        }
        else {
            // Accelerating towards target speed in current direction
            if (vx < targetVx) {
                vx += ACCELERATION * dt;
                if (vx > targetVx) vx = targetVx;
            }
            else if (vx > targetVx) {
                vx -= ACCELERATION * dt;
                if (vx < targetVx) vx = targetVx;
            }
        }
    }
    else {
        // No directional input: decelerate velocity to 0 cleanly with snap threshold
        if (vx > 0.0) {
            vx -= DECELERATION * dt;
            if (vx <= 5.0) vx = 0.0;
        }
        else if (vx < 0.0) {
            vx += DECELERATION * dt;
            if (vx >= -5.0) vx = 0.0;
        }
    }

    // 6. Animation state machine transitions hierarchy evaluation
    if (state == STATE_DEAD) {
        // Highest priority lock: stay dead
    }
    else if (state == STATE_HURT) {
        // Lock state in HURT until hurt animation finishes or invulnerability ends
        if (animHurt.IsFinished() || invulnerabilityTimer <= 0) {
            if (!isGrounded) {
                SetState(STATE_JUMP);
            }
            else if (std::abs(vx) > 5.0 || moveLeft || moveRight) {
                SetState(isRunning ? STATE_RUN : STATE_WALK);
            }
            else {
                SetState(STATE_IDLE);
            }
        }
    }
    else if (state == STATE_ATTACK_MELEE) {
        // Lock state in ATTACK until melee animation finishes
        if (animAttack.IsFinished()) {
            if (!isGrounded) {
                SetState(STATE_JUMP);
            }
            else if (std::abs(vx) > 5.0 || moveLeft || moveRight) {
                SetState(isRunning ? STATE_RUN : STATE_WALK);
            }
            else {
                SetState(STATE_IDLE);
            }
        }
    }
    else if (!isGrounded) {
        SetState(STATE_JUMP);
    }
    else if (std::abs(vx) > 5.0 || moveLeft || moveRight) {
        SetState(isRunning ? STATE_RUN : STATE_WALK);
    }
    else {
        SetState(STATE_IDLE);
    }

    // 7. Dynamic locomotion animation speed matching physical movement velocity
    double currentSpeed = std::abs(vx);
    if (currentSpeed > 5.0) {
        int walkTicks = (int)(5.0 * (WALK_SPEED / currentSpeed));
        if (walkTicks < 2) walkTicks = 2;
        if (walkTicks > 8) walkTicks = 8;
        animWalk.SetFrameDuration(walkTicks);

        int runTicks = (int)(4.0 * (RUN_SPEED / currentSpeed));
        if (runTicks < 2) runTicks = 2;
        if (runTicks > 6) runTicks = 6;
        animRun.SetFrameDuration(runTicks);
    }

    // Select active animation for playback update
    Animation* activeAnim = &animIdle;
    if (state == STATE_WALK && animWalk.IsValid()) activeAnim = &animWalk;
    else if (state == STATE_RUN && animRun.IsValid()) activeAnim = &animRun;
    else if (state == STATE_JUMP && animJump.IsValid()) activeAnim = &animJump;
    else if (state == STATE_ATTACK_MELEE && animAttack.IsValid()) activeAnim = &animAttack;
    else if (state == STATE_HURT && animHurt.IsValid()) activeAnim = &animHurt;
    else if (state == STATE_DEAD && animDeath.IsValid()) activeAnim = &animDeath;

    activeAnim->Update();
    currentAnimationFrame = activeAnim->GetCurrentFrame();

    // 8. Integrate position over delta time
    x += vx * dt;

    // 9. Keep player within level left boundary
    if (x < 0) {
        x = 0;
        vx = 0.0;
    }

    // 10. Smooth animated Health & Stamina bar transitions for HUD
    double hpSpeed = 10.0 * dt;
    if (hpSpeed > 1.0) hpSpeed = 1.0;
    displayedHp += ((double)hp - displayedHp) * hpSpeed;
    if (std::abs((double)hp - displayedHp) < 0.1) {
        displayedHp = (double)hp;
    }

    double stamSpeed = 10.0 * dt;
    if (stamSpeed > 1.0) stamSpeed = 1.0;
    displayedStamina += (staminaDouble - displayedStamina) * stamSpeed;
    if (std::abs(staminaDouble - displayedStamina) < 0.1) {
        displayedStamina = staminaDouble;
    }
}

// ============================================================================
// Render Method (Viewport Relative)
// ============================================================================
void Player::Render(double camX, double camY) {
    // Blink effect if invulnerable
    if (isInvulnerable && (invulnerabilityTimer / 5) % 2 == 0) {
        return;
    }

    // Coordinates relative to camera viewport
    double drawX = x - camX;
    double drawY = y - camY;

    // Sprite drawing dimensions (Increased slightly to 195px to match environment scale perfectly)
    int drawW = 195;
    int drawH = 195;

    // Offset drawing position so Arin's boots align precisely with top of ground baseline without sinking underground
    double drawXOffset = drawX - (drawW - width) / 2.0;
    double drawYOffset = drawY - 6.0;

    // Render active state animation via reusable Animation class
    const Animation* activeAnim = &animIdle;
    if (state == STATE_WALK && animWalk.IsValid()) activeAnim = &animWalk;
    else if (state == STATE_RUN && animRun.IsValid()) activeAnim = &animRun;
    else if (state == STATE_JUMP && animJump.IsValid()) activeAnim = &animJump;
    else if (state == STATE_ATTACK_MELEE && animAttack.IsValid()) activeAnim = &animAttack;
    else if (state == STATE_HURT && animHurt.IsValid()) activeAnim = &animHurt;
    else if (state == STATE_DEAD && animDeath.IsValid()) activeAnim = &animDeath;

    activeAnim->Render((int)drawXOffset, (int)drawYOffset, drawW, drawH, isFacingRight);
}

// ============================================================================
// Health & Damage Handlers
// ============================================================================
void Player::TakeDamage(int damage) {
    if (isInvulnerable || state == STATE_DEAD) return;

    hp -= damage;
    if (hp <= 0) {
        hp = 0;
        SetState(STATE_DEAD);
    }
    else {
        SetState(STATE_HURT);
        isInvulnerable = true;
        invulnerabilityTimer = 30;
    }
}

// ============================================================================
// Player Actions (Combat & Healing)
// ============================================================================
void Player::AttackMelee() {
    if (state == STATE_DEAD || state == STATE_HURT) return;
    if (state == STATE_ATTACK_MELEE || attackCooldownTimer > 0.0) return; // Prevent attack spamming

    currentAttackID++;
    SetState(STATE_ATTACK_MELEE);
    hasDealtDamageThisAttack = false;
    attackCooldownTimer = 0.45; // 0.45s attack and recovery cooldown
}

void Player::AttackRanged() {
    if (state == STATE_DEAD || state == STATE_HURT) return;
    if (ammo > 0) {
        ammo--;
        rangedAttackTriggered = true;
    }
}

void Player::UseHeal() {
    if (state == STATE_DEAD || state == STATE_HURT) return;
    if (medkits > 0 && hp < maxHp) {
        medkits--;
        hp = maxHp;
        displayedHp = (double)hp;
    }
}

void Player::UseFood() {
    if (state == STATE_DEAD || state == STATE_HURT) return;
    if (foodCount > 0 && (staminaDouble < maxStamina || hp < maxHp)) {
        foodCount--;
        staminaDouble = (double)maxStamina;
        stamina = maxStamina;
        displayedStamina = (double)maxStamina;
        isExhausted = false;
        hp = (hp + 25 > maxHp) ? maxHp : hp + 25;
        displayedHp = (double)hp;
    }
}

void Player::UseWaterBottle() {
    if (state == STATE_DEAD || state == STATE_HURT) return;
    if (waterBottleCount > 0 && staminaDouble < (double)maxStamina) {
        waterBottleCount--;
        staminaDouble = (staminaDouble + 25.0 > (double)maxStamina) ? (double)maxStamina : staminaDouble + 25.0;
        stamina = (int)std::round(staminaDouble);
        displayedStamina = staminaDouble;
        if (staminaDouble >= 15.0) {
            isExhausted = false;
        }
    }
}

void Player::ResetInputState() {
    wasJumpPressed = false;
    wasAttackPressed = false;
}


