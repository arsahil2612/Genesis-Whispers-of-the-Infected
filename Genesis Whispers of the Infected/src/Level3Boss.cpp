#define _CRT_SECURE_NO_WARNINGS
#include "Level3Boss.h"
#include "asset_loader.h"
#include "igraphics_declarations.h"
#include <cmath>
#include <cstdio>
#include <GL/gl.h>

Level3Boss::Level3Boss() {
    x = 0;
    y = 185;
    width = 200;
    height = 220;
    hp = 1200;
    maxHp = 1200;
    isFacingRight = false;
    phase = L3_BOSS_INACTIVE;
    monsterState = MONSTER_IDLE;
    assetsLoaded = false;
    
    vx = 0.0;
    vy = 0.0;
    dashSpeed = 0.0;
    bodyDamageCooldown = 0.0;

    dialogueStep = 0;
    dialogueTimer = 0.0;
    introTimer = 0.0;
    transformTimer = 0.0;

    attackCooldownTimer = 2.0;
    droneCooldown = 0.0;
    stateTimer = 0.0;
    clawDamageDealt = false;
    chargeDamageDealt = false;
    chargeStartX = 0;
    chargeTargetX = 0;
}

void Level3Boss::PreloadAssets() {
    if (assetsLoaded && animHumanIdle.IsValid() && animHumanIdle.GetTextureID() != 0) {
        if (glIsTexture(animHumanIdle.GetTextureID())) return;
    }

    printf("[GENESIS Engine] Preloading Dr. Kael Assets...\n");

    auto loadSeq = [](const char* format, int count) {
        std::vector<unsigned int> seq;
        for (int i = 1; i <= count; ++i) {
            char path[160];
            sprintf_s(path, sizeof(path), format, i);
            unsigned int tex = iLoadImage((char*)GetAssetPath(path).c_str());
            if (tex != 0) seq.push_back(tex);
        }
        return seq;
    };

    // 1. Load Human Kael Animations (6 animation sets)
    animHumanIdle.InitSequence(loadSeq("Assets/Characters/Dr. Kael/Human/Idle/idle_%02d.png", 6), 12, true);
    animHumanWalk.InitSequence(loadSeq("Assets/Characters/Dr. Kael/Human/walk/walk_%02d.png", 8), 10, true);
    animHumanTalk.InitSequence(loadSeq("Assets/Characters/Dr. Kael/Human/talk/talk_%02d.png", 8), 10, true);
    animHumanHurt.InitSequence(loadSeq("Assets/Characters/Dr. Kael/Human/hurt/hurt_%02d.png", 4), 8, false);
    animHumanSummon.InitSequence(loadSeq("Assets/Characters/Dr. Kael/Human/summon drone/summon_drone_%02d.png", 8), 8, false);
    animHumanInject.InitSequence(loadSeq("Assets/Characters/Dr. Kael/Human/inject serum/inject_serum_%02d.png", 8), 12, false);
    animHumanTransform.InitSequence(loadSeq("Assets/Characters/Dr. Kael/Human/transformation to monster/transformation_%02d.png", 9), 14, false);

    // 2. Load Monster Kael Animations
    animMonsterIdle.InitSequence(loadSeq("Assets/Characters/Dr. Kael/Monster/idle/monster_idle_%02d.png", 6), 10, true);
    animMonsterWalk.InitSequence(loadSeq("Assets/Characters/Dr. Kael/Monster/walk/monster_walk_%02d.png", 10), 8, true);
    animMonsterClaw.InitSequence(loadSeq("Assets/Characters/Dr. Kael/Monster/claw attack/monster_claw_attack_%02d.png", 10), 6, false);
    animMonsterHurt.InitSequence(loadSeq("Assets/Characters/Dr. Kael/Monster/hurt/monster_hurt_%02d.png", 5), 8, false);
    animMonsterStagger.InitSequence(loadSeq("Assets/Characters/Dr. Kael/Monster/stagger/monster_stagger_%02d.png", 5), 10, false);
    animMonsterWarning.InitSequence(loadSeq("Assets/Characters/Dr. Kael/Monster/spike attack/Purple Ground Circle Warning Effect/purple_ground_circle_warning_effect_%02d.png", 5), 6, true);
    animMonsterSpike.InitSequence(loadSeq("Assets/Characters/Dr. Kael/Monster/spike attack/ground spikes/monster_ground_spike_attack_%02d.png", 8), 6, false);
    animMonsterCharge.InitSequence(loadSeq("Assets/Characters/Dr. Kael/Monster/spike attack/monster charge animation/monster_charge_animation_%02d.png", 8), 6, true);

    // 3. Preload Drone & Projectile Textures
    texDroneIdle = loadSeq("Assets/Characters/Dr. Kael/Human/Drone Attack/Combat Drone/Idle/drone_idle_%02d.png", 4);
    texDroneProj = loadSeq("Assets/Characters/Dr. Kael/Human/Drone Attack/Drone Energy Projectile/projectile_%02d.png", 6);

    assetsLoaded = true;
    printf("[GENESIS Engine] Dr. Kael Assets Preloaded. Idle frames: %d, Talk frames: %d (Valid: %s, TexID: %u)\n",
           animHumanIdle.GetFrameCount(), animHumanTalk.GetFrameCount(), animHumanIdle.IsValid() ? "YES" : "NO", animHumanIdle.GetTextureID());
}

void Level3Boss::Initialize(double arenaX, double arenaY) {
    printf("[GENESIS Engine] HUMAN KAEL INITIALIZATION STARTED\n");
    x = arenaX + 750.0;
    y = arenaY;
    width = 135;
    height = 150;
    hp = 1200;
    maxHp = 1200;
    isFacingRight = false;
    phase = L3_BOSS_HUMAN_INTRO;
    monsterState = MONSTER_IDLE;
    
    vx = 0.0;
    vy = 0.0;
    dashSpeed = 0.0;
    bodyDamageCooldown = 0.0;

    dialogueStep = 0;
    dialogueTimer = 0.0;
    introTimer = 0.0;
    transformTimer = 0.0;
    attackCooldownTimer = 2.0;
    droneCooldown = 15.0;

    drones.clear();
    droneProjectiles.clear();
    spikes.clear();

    assetsLoaded = false;
    PreloadAssets();

    // First dialogue (Step 0)
    currentSpeaker = "Dr. Kael";
    currentText = "\"Ah, Arin... So you survived my test subjects. I am Dr. Kael, chief architect of Project Genesis.\"";
    animHumanTalk.Reset();
    animHumanIdle.Reset();
    animHumanWalk.Reset();

    printf("[GENESIS Engine] HUMAN KAEL INITIALIZED at X=%.1f, Y=%.1f (Phase: %d, Idle Valid: %s, TexID: %u)\n",
           x, y, (int)phase, animHumanIdle.IsValid() ? "YES" : "NO", animHumanIdle.GetTextureID());
}

void Level3Boss::AdvanceDialogue() {
    dialogueStep++;
    dialogueTimer = 0.0;
    if (dialogueStep == 1) {
        currentSpeaker = "Arin";
        currentText = "\"Kael! I know who you are. What have you done with Subject Luna? Where is she?!\"";
    }
    else if (dialogueStep == 2) {
        currentSpeaker = "Dr. Kael";
        currentText = "\"Luna? She was the key. Her DNA stabilized the Genesis virus—the ultimate catalyst for human evolution!\"";
        animHumanTalk.Reset();
    }
    else if (dialogueStep == 3) {
        currentSpeaker = "Arin";
        currentText = "\"You monster! She's a person, not your experiment! Give her back and stop this madness!\"";
    }
    else if (dialogueStep == 4) {
        currentSpeaker = "Dr. Kael";
        currentText = "\"Stop? Never! Humanity is weak, Arin. I am its architect, and Project Genesis will reshape the world!\"";
        animHumanTalk.Reset();
    }
    else if (dialogueStep == 5) {
        currentSpeaker = "Arin";
        currentText = "\"I won't let you hurt anyone else, Kael. Your twisted dream ends today!\"";
    }
    else if (dialogueStep == 6) {
        currentSpeaker = "Dr. Kael";
        currentText = "\"Insolent fool! You will be the first sacrifice for the new species!\"";
        animHumanTalk.Reset();
    }
    else if (dialogueStep >= 7) {
        phase = L3_BOSS_HUMAN_DIALOGUE_COMPLETE;
        currentSpeaker = "";
        currentText = "";
    }
}

void Level3Boss::SpawnDrone(double spawnX, double spawnY) {
    Drone d;
    d.x = spawnX;
    d.y = spawnY;
    d.active = true;
    d.hp = 160;
    
    if (!texDroneIdle.empty()) {
        d.anim.InitSequence(texDroneIdle, 8, true);
    } else {
        std::vector<unsigned int> seq;
        for (int i = 1; i <= 4; ++i) {
            char p[160];
            sprintf_s(p, sizeof(p), "Assets/Characters/Dr. Kael/Human/Drone Attack/Combat Drone/Idle/drone_idle_%02d.png", i);
            unsigned int tex = iLoadImage((char*)GetAssetPath(p).c_str());
            if (tex != 0) seq.push_back(tex);
        }
        d.anim.InitSequence(seq, 8, true);
    }
    drones.push_back(d);
    printf("DRONE SPAWNED\n");
}

void Level3Boss::TriggerSpikeAttack(double targetX, double targetY) {
    SpikeEffect sp;
    sp.x = targetX;
    sp.y = targetY;
    sp.timer = 0.0;
    sp.isWarning = true;
    sp.damageDealt = false;
    sp.animWarning = animMonsterWarning;
    sp.animSpike = animMonsterSpike;
    spikes.push_back(sp);
}

void Level3Boss::StartDroneAttack() {
    printf("DRONE ATTACK STARTED\n");
    drones.clear();
    droneProjectiles.clear();
    SpawnDrone(x - 200, y + 120);
    SpawnDrone(x + 200, y + 120);
}

void Level3Boss::UpdateDroneAttack(Player& player, float dt) {
    for (auto& d : drones) {
        if (!d.active) continue;
        d.anim.Update();
        d.timer += dt;

        // Drone hovering AI targeting Arin at chest/head level
        d.targetX = player.x + (d.x < player.x ? -160.0 : 160.0);
        d.targetY = player.y + 65.0 + sin(d.timer * 3.0) * 20.0;
        d.x += (d.targetX - d.x) * 0.05;
        d.y += (d.targetY - d.y) * 0.05;

        // Controlled projectile attack timing
        d.attackCooldown += dt;
        if (d.attackCooldown >= 1.8) {
            d.attackCooldown = 0.0;
            DroneProjectile p;
            p.x = d.x;
            p.y = d.y - 10;
            double targetX = player.x + 30.0;
            double targetY = player.y + 80.0;
            double dx = targetX - d.x;
            double dy = targetY - d.y;
            double dist = sqrt(dx*dx + dy*dy);
            if (dist > 0.1) {
                p.vx = (dx / dist) * 450.0;
                p.vy = (dy / dist) * 450.0;
            }
            p.damage = 10;
            p.active = true;
            
            if (!texDroneProj.empty()) {
                p.anim.InitSequence(texDroneProj, 2, true);
            } else {
                std::vector<unsigned int> seq;
                for (int i = 1; i <= 6; ++i) {
                    char b[160];
                    sprintf_s(b, sizeof(b), "Assets/Characters/Dr. Kael/Human/Drone Attack/Drone Energy Projectile/projectile_%02d.png", i);
                    unsigned int tex = iLoadImage((char*)GetAssetPath(b).c_str());
                    if (tex != 0) seq.push_back(tex);
                }
                p.anim.InitSequence(seq, 2, true);
            }
            droneProjectiles.push_back(p);
            printf("DRONE PROJECTILE FIRED\n");
        }
    }
}

void Level3Boss::UpdateDroneProjectiles(Player& player, float dt) {
    for (auto& p : droneProjectiles) {
        if (!p.active) continue;
        p.x += p.vx * dt;
        p.y += p.vy * dt;
        p.anim.Update();

        if (abs(p.x - (player.x + player.width / 2.0)) < 40.0 &&
            abs(p.y - (player.y + player.height / 2.0)) < 50.0) {
            player.TakeDamage(p.damage);
            p.active = false;
        }

        if (p.x < player.x - 1000 || p.x > player.x + 1000 || p.y < 0 || p.y > 800) {
            p.active = false;
        }
    }
}

void Level3Boss::Update(Player& player, float dt) {
    if (phase == L3_BOSS_INACTIVE) return;

    UpdateDroneAttack(player, dt);
    UpdateDroneProjectiles(player, dt);

    // ------------------------------------------------------------------------
    // PHASE 1: Human Intro, Dialogue & Tactical Drone Attack
    // ------------------------------------------------------------------------
    if (phase == L3_BOSS_HUMAN_INTRO) {
        introTimer += dt;
        isFacingRight = (player.x > x);
        
        // Dr. Kael walks left towards dialogue standing position
        if (x > player.x + 300.0) {
            x -= 120.0 * dt;
            animHumanWalk.Update();
        } else {
            animHumanIdle.Update();
        }

        if (introTimer >= 2.0) {
            phase = L3_BOSS_HUMAN_DIALOGUE_READY;
            animHumanIdle.Reset();
        }
        return;
    }

    if (phase == L3_BOSS_HUMAN_DIALOGUE_READY) {
        isFacingRight = (player.x > x);
        animHumanIdle.Update();
        return;
    }

    if (phase == L3_BOSS_HUMAN_DIALOGUE) {
        isFacingRight = (player.x > x);
        if (currentSpeaker == "Dr. Kael" || currentSpeaker == "DR. KAEL" || currentSpeaker == "Dr Kael") {
            animHumanTalk.Update();
        } else {
            animHumanIdle.Update();
        }

        // Auto-advance dialogue step after 4 seconds per line if player does not press buttons
        dialogueTimer += dt;
        if (dialogueTimer >= 4.0) {
            AdvanceDialogue();
        }
        return;
    }

    if (phase == L3_BOSS_HUMAN_DIALOGUE_COMPLETE) {
        phase = L3_BOSS_HUMAN_IDLE;
        stateTimer = 0.0;
        isFacingRight = (player.x > x);
        animHumanIdle.Reset();
        return;
    }

    if (phase == L3_BOSS_HUMAN_IDLE) {
        stateTimer += dt;
        isFacingRight = (player.x > x);
        animHumanIdle.Update();
        if (stateTimer >= 1.5) {
            phase = L3_BOSS_HUMAN_ANGER;
            stateTimer = 0.0;
            animHumanWalk.Reset();
        }
        return;
    }

    if (phase == L3_BOSS_HUMAN_ANGER) {
        stateTimer += dt;
        isFacingRight = (player.x > x);

        // Dr Kael walks towards player position
        x += (isFacingRight ? 90.0 : -90.0) * dt;
        animHumanWalk.Update();

        if (stateTimer >= 2.5) {
            phase = L3_BOSS_HUMAN_SUMMON_DRONE;
            stateTimer = 0.0;
            animHumanSummon.Reset();
        }
        return;
    }

    if (phase == L3_BOSS_HUMAN_SUMMON_DRONE) {
        stateTimer += dt;
        isFacingRight = (player.x > x);
        animHumanSummon.Update();
        if (animHumanSummon.IsFinished() || stateTimer >= 1.2) {
            phase = L3_BOSS_HUMAN_DRONE_SUMMON_COMPLETE;
            stateTimer = 0.0;
        }
        return;
    }

    if (phase == L3_BOSS_HUMAN_DRONE_SUMMON_COMPLETE) {
        phase = L3_BOSS_HUMAN_DRONE_ATTACK;
        stateTimer = 0.0;
        StartDroneAttack();
        isFacingRight = (player.x > x);
        animHumanIdle.Reset();
        return;
    }

    if (phase == L3_BOSS_HUMAN_DRONE_ATTACK) {
        stateTimer += dt;
        isFacingRight = (player.x > x);
        x += (isFacingRight ? 30.0 : -30.0) * dt;
        animHumanIdle.Update();

        // Drone attack phase duration (7.0 seconds)
        if (stateTimer >= 7.0) {
            drones.clear();
            droneProjectiles.clear();
            phase = L3_BOSS_HUMAN_PREPARE_SERUM;
            stateTimer = 0.0;
        }
        return;
    }

    if (phase == L3_BOSS_HUMAN_PREPARE_SERUM) {
        stateTimer += dt;
        isFacingRight = (player.x > x);
        animHumanIdle.Update();
        if (stateTimer >= 1.0) {
            phase = L3_BOSS_HUMAN_INJECT_SERUM;
            stateTimer = 0.0;
            animHumanInject.Reset();
        }
        return;
    }

    if (phase == L3_BOSS_HUMAN_INJECT_SERUM) {
        stateTimer += dt;
        isFacingRight = (player.x > x);
        animHumanInject.Update();
        if (animHumanInject.IsFinished() || stateTimer >= 1.8) {
            phase = L3_BOSS_HUMAN_SERUM_COMPLETE;
            stateTimer = 0.0;
        }
        return;
    }

    if (phase == L3_BOSS_HUMAN_SERUM_COMPLETE) {
        phase = L3_BOSS_TRANSFORMATION_PREPARE;
        stateTimer = 0.0;
        isFacingRight = (player.x > x);
        return;
    }

    if (phase == L3_BOSS_TRANSFORMATION_PREPARE) {
        phase = L3_BOSS_KAEL_TRANSFORMING;
        stateTimer = 0.0;
        animHumanTransform.Reset();
        isFacingRight = (player.x > x);
        return;
    }

    if (phase == L3_BOSS_KAEL_TRANSFORMING || phase == L3_BOSS_HUMAN_TRANSFORMING) {
        stateTimer += dt;
        isFacingRight = (player.x > x);
        animHumanTransform.Update();
        if (animHumanTransform.IsFinished() || stateTimer >= 2.5) {
            phase = L3_BOSS_MONSTER_KAEL_INITIALIZE;
            stateTimer = 0.0;
        }
        return;
    }

    if (phase == L3_BOSS_MONSTER_KAEL_INITIALIZE) {
        phase = L3_BOSS_MONSTER_KAEL_IDLE;
        width = 220;
        height = 230;
        hp = 1200;
        maxHp = 1200;
        monsterState = MONSTER_IDLE;
        stateTimer = 0.0;
        animMonsterIdle.Reset();
        isFacingRight = (player.x > x);
        return;
    }

    if (phase == L3_BOSS_MONSTER_KAEL_IDLE) {
        stateTimer += dt;
        isFacingRight = (player.x > x);
        animMonsterIdle.Update();
        if (stateTimer >= 1.5) {
            phase = L3_BOSS_MONSTER_ACTIVE;
            monsterState = MONSTER_IDLE;
            attackCooldownTimer = 1.0;
            stateTimer = 0.0;
            vx = 0.0;
            dashSpeed = 0.0;
        }
        return;
    }

    // ------------------------------------------------------------------------
    // Ground Spikes Sub-System Updates
    // ------------------------------------------------------------------------

    for (auto& sp : spikes) {
        sp.timer += dt;
        if (sp.isWarning) {
            sp.animWarning.Update();
            if (sp.timer >= 1.0) {
                sp.isWarning = false;
                sp.timer = 0.0;
            }
        } else {
            sp.animSpike.Update();
            if (!sp.damageDealt && abs(sp.x - player.x) < 70.0 && abs(sp.y - player.y) < 60.0) {
                player.TakeDamage(35);
                sp.damageDealt = true;
            }
        }
    }

    // ------------------------------------------------------------------------
    // PHASE 2: Transformed Monster Kael Boss Combat AI
    // ------------------------------------------------------------------------
    if (phase == L3_BOSS_MONSTER_ACTIVE) {
        isFacingRight = (player.x > x);
        attackCooldownTimer -= dt;
        
        x += vx * dt;
        y += vy * dt;
        
        if (bodyDamageCooldown > 0) bodyDamageCooldown -= dt;

        switch (monsterState) {
        case MONSTER_IDLE:
            animMonsterIdle.Update();
            vx = 0.0;
            if (droneCooldown > 0) droneCooldown -= dt;
            
            if (droneCooldown <= 0.0) {
                monsterState = MONSTER_SUMMON_DRONES;
                StartDroneAttack();
                droneCooldown = 15.0; // Reset cooldown
                stateTimer = 0.0;
            } else if (attackCooldownTimer <= 0.0) {
                double dist = abs(player.x - x);
                if (dist < 190.0) {
                    monsterState = MONSTER_ATTACK_CLAW;
                    animMonsterClaw.Reset();
                    clawDamageDealt = false;
                    stateTimer = 0.0;
                } else if (dist > 350.0 && (hp < maxHp * 0.75)) {
                    monsterState = MONSTER_ATTACK_CHARGE;
                    animMonsterCharge.Reset();
                    chargeStartX = x;
                    chargeTargetX = player.x;
                    chargeDamageDealt = false;
                    stateTimer = 0.0;
                    dashSpeed = 420.0;
                    printf("DASH START\n");
                } else if (rand() % 2 == 0) {
                    monsterState = MONSTER_CHASE;
                    animMonsterWalk.Reset();
                    stateTimer = 0.0;
                } else {
                    monsterState = MONSTER_ATTACK_SPIKES;
                    TriggerSpikeAttack(player.x, 185.0);
                    stateTimer = 0.0;
                }
            }
            break;

        case MONSTER_SUMMON_DRONES:
            animMonsterIdle.Update();
            vx = 0.0;
            stateTimer += dt;
            if (stateTimer >= 1.0) {
                monsterState = MONSTER_IDLE;
                attackCooldownTimer = 1.0;
            }
            break;

        case MONSTER_CHASE:
            animMonsterWalk.Update();
            vx = isFacingRight ? 160.0 : -160.0;

            if (abs(player.x - x) < 190.0) {
                vx = 0.0;
                monsterState = MONSTER_ATTACK_CLAW;
                animMonsterClaw.Reset();
                clawDamageDealt = false;
                stateTimer = 0.0;
            }
            break;

        case MONSTER_ATTACK_CLAW:
            animMonsterClaw.Update();
            vx = 0.0;
            stateTimer += dt;
            if (!clawDamageDealt && stateTimer >= 0.35) {
                if (abs(player.x - x) < 220.0 && abs(player.y - y) < 150.0) {
                    player.TakeDamage(40);
                    clawDamageDealt = true;
                    printf("DAMAGE APPLIED\n");
                }
            }
            if (stateTimer >= 1.0 || animMonsterClaw.IsFinished()) {
                monsterState = MONSTER_IDLE;
                attackCooldownTimer = (hp < 400) ? 0.6 : 1.2;
            }
            break;

        case MONSTER_ATTACK_SPIKES:
            animMonsterIdle.Update();
            vx = 0.0;
            stateTimer += dt;
            if (stateTimer >= 1.2) {
                monsterState = MONSTER_IDLE;
                attackCooldownTimer = (hp < 400) ? 0.8 : 1.5;
            }
            break;

        case MONSTER_ATTACK_CHARGE:
            animMonsterCharge.Update();
            stateTimer += dt;
            vx = isFacingRight ? dashSpeed : -dashSpeed;

            if (!chargeDamageDealt && abs(player.x - x) < 180.0 && abs(player.y - y) < 150.0) {
                player.TakeDamage(50);
                chargeDamageDealt = true;
                printf("DAMAGE APPLIED\n");
            }
            if (stateTimer >= 1.2 || abs(x - chargeStartX) > 650.0) {
                printf("DASH END\n");
                vx = 0.0;
                dashSpeed = 0.0;
                monsterState = MONSTER_IDLE;
                attackCooldownTimer = 1.8;
            }
            break;

        case MONSTER_STAGGER:
            animMonsterStagger.Update();
            vx = 0.0;
            dashSpeed = 0.0;
            stateTimer += dt;
            if (stateTimer >= 0.7) {
                monsterState = MONSTER_IDLE;
                attackCooldownTimer = 0.5;
            }
            break;

        case MONSTER_DEAD:
            vx = 0.0;
            dashSpeed = 0.0;
            animMonsterHurt.Update();
            break;
        }
    }
}

void Level3Boss::TakeDamage(int damage) {
    if (phase != L3_BOSS_MONSTER_ACTIVE) return;

    hp -= damage;
    if (hp <= 0) {
        hp = 0;
        phase = L3_BOSS_DEFEATED;
        monsterState = MONSTER_DEAD;
    } else if (damage > 30 && monsterState != MONSTER_ATTACK_CLAW && monsterState != MONSTER_ATTACK_CHARGE) {
        monsterState = MONSTER_STAGGER;
        animMonsterStagger.Reset();
        stateTimer = 0.0;
    }
}

bool Level3Boss::CheckPlayerCollision(double px, double py, int pw, int ph, Player& player) {
    if (phase != L3_BOSS_MONSTER_ACTIVE) return false;
    
    // Check general bounding box collision
    if (abs(px - x) < (pw + width) / 2.0 && abs(py - y) < (ph + height) / 2.0) {
        // Prevent continuous body damage during normal movement, only damage during attacks
        if (monsterState == MONSTER_ATTACK_CLAW || monsterState == MONSTER_ATTACK_CHARGE) {
            if (bodyDamageCooldown <= 0.0) {
                bodyDamageCooldown = 1.0;
                printf("DAMAGE APPLIED\n");
                return true;
            }
        } else {
            // Apply a small pushback or generic contact logic if we want,
            // but we must not apply massive damage every frame.
            if (bodyDamageCooldown <= 0.0) {
                // If they just bump into him, we can choose to apply minor contact damage or none.
                // According to instructions: "damage should happen once per attack".
                // We'll just return true to let game_manager register the hit if it's the player attacking him,
                // Wait, CheckPlayerCollision in game_manager is used for: 
                // 1) Player melee hitting Boss -> returns true so Boss takes damage!
                // So if we return false, player can't melee him!
                // So we MUST return true here for the bounding box check!
                return true; 
            }
        }
        return true;
    }
    return false;
}

void Level3Boss::Render(double camX, double camY) {
    if (phase == L3_BOSS_INACTIVE) return;

    int renderX = (int)(x - camX);
    int renderY = (int)(y - camY);

    // 1. Render Spikes & Warnings
    for (auto& sp : spikes) {
        int sx = (int)(sp.x - camX);
        int sy = (int)(sp.y - camY);
        if (sp.isWarning) {
            sp.animWarning.Render(sx - 60, sy, 120, 120, true);
        } else if (sp.timer < 1.0) {
            sp.animSpike.Render(sx - 70, sy, 140, 160, true);
        }
    }

    // 2. Render Drone Projectiles
    for (auto& p : droneProjectiles) {
        if (p.active) {
            int px = (int)(p.x - camX);
            int py = (int)(p.y - camY);
            p.anim.Render(px - 25, py - 25, 50, 50, (p.vx >= 0));
        }
    }

    // 3. Render Drones
    for (auto& d : drones) {
        if (d.active) {
            int dx = (int)(d.x - camX);
            int dy = (int)(d.y - camY);
            d.anim.Render(dx - 45, dy - 40, 90, 80, (d.x < x));
        }
    }

    // 4. Render Human Dr. Kael Forms
    if (phase == L3_BOSS_HUMAN_INTRO || phase == L3_BOSS_HUMAN_ANGER) {
        if (animHumanWalk.IsValid()) animHumanWalk.Render(renderX - width/2, renderY, width, height, isFacingRight);
        else animHumanIdle.Render(renderX - width/2, renderY, width, height, isFacingRight);
    }
    else if (phase == L3_BOSS_HUMAN_SUMMON_DRONE || phase == L3_BOSS_HUMAN_ANGRY_DRONES || phase == L3_BOSS_HUMAN_DRONE_SUMMON_COMPLETE) {
        if (animHumanSummon.IsValid()) animHumanSummon.Render(renderX - width/2, renderY, width, height, isFacingRight);
        else animHumanIdle.Render(renderX - width/2, renderY, width, height, isFacingRight);
    }
    else if (phase == L3_BOSS_HUMAN_INJECT_SERUM || phase == L3_BOSS_HUMAN_SERUM_COMPLETE) {
        if (animHumanInject.IsValid()) animHumanInject.Render(renderX - width/2, renderY, width, height, isFacingRight);
        else animHumanIdle.Render(renderX - width/2, renderY, width, height, isFacingRight);
    }
    else if (phase == L3_BOSS_HUMAN_DIALOGUE || phase == L3_BOSS_HUMAN_DIALOGUE_READY) {
        if ((currentSpeaker == "Dr. Kael" || currentSpeaker == "DR. KAEL" || currentSpeaker == "Dr Kael") && animHumanTalk.IsValid()) {
            animHumanTalk.Render(renderX - width/2, renderY, width, height, isFacingRight);
        } else {
            animHumanIdle.Render(renderX - width/2, renderY, width, height, isFacingRight);
        }
    }
    else if (phase == L3_BOSS_KAEL_TRANSFORMING || phase == L3_BOSS_HUMAN_TRANSFORMING || phase == L3_BOSS_TRANSFORMATION_PREPARE) {
        if (animHumanTransform.IsValid()) animHumanTransform.Render(renderX - width/2, renderY, width + 40, height + 40, isFacingRight);
        else animHumanIdle.Render(renderX - width/2, renderY, width, height, isFacingRight);
    }
    else if (phase == L3_BOSS_HUMAN_DRONE_ATTACK || phase == L3_BOSS_HUMAN_PREPARE_SERUM || phase == L3_BOSS_HUMAN_IDLE || phase == L3_BOSS_HUMAN_DIALOGUE_COMPLETE) {
        if (animHumanIdle.IsValid()) animHumanIdle.Render(renderX - width/2, renderY, width, height, isFacingRight);
    }
    // 5. Render Transformed Monster Kael Form
    else if (phase == L3_BOSS_MONSTER_KAEL_INITIALIZE || phase == L3_BOSS_MONSTER_KAEL_IDLE) {
        animMonsterIdle.Render(renderX - width/2, renderY, width, height, isFacingRight);
    }
    else if (phase == L3_BOSS_MONSTER_ACTIVE || phase == L3_BOSS_DEFEATED) {
        switch (monsterState) {
        case MONSTER_IDLE:
            animMonsterIdle.Render(renderX - width/2, renderY, width, height, isFacingRight);
            break;
        case MONSTER_CHASE:
            animMonsterWalk.Render(renderX - width/2, renderY, width, height, isFacingRight);
            break;
        case MONSTER_ATTACK_CLAW:
            animMonsterClaw.Render(renderX - width/2, renderY, width + 50, height + 20, isFacingRight);
            break;
        case MONSTER_ATTACK_CHARGE:
            animMonsterCharge.Render(renderX - width/2, renderY, width + 60, height, isFacingRight);
            break;
        case MONSTER_STAGGER:
            animMonsterStagger.Render(renderX - width/2, renderY, width, height, isFacingRight);
            break;
        case MONSTER_DEAD:
            animMonsterHurt.Render(renderX - width/2, renderY, width, height, isFacingRight);
            break;
        default:
            animMonsterIdle.Render(renderX - width/2, renderY, width, height, isFacingRight);
            break;
        }
    }
    else if (phase != L3_BOSS_INACTIVE) {
        // Fail-safe render for all active Human Kael phases
        if (animHumanIdle.IsValid()) {
            animHumanIdle.Render(renderX - width/2, renderY, width, height, isFacingRight);
        }
    }
}
