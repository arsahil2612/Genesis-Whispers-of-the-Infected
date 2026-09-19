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

    dialogueStep = 0;
    dialogueTimer = 0.0;
    introTimer = 0.0;
    transformTimer = 0.0;

    attackCooldownTimer = 2.0;
    stateTimer = 0.0;
    clawDamageDealt = false;
    chargeDamageDealt = false;
    chargeStartX = 0;
    chargeTargetX = 0;
}

void Level3Boss::Initialize(double arenaX, double arenaY) {
    x = arenaX;
    y = arenaY;
    width = 180;
    height = 200;
    hp = 1200;
    maxHp = 1200;
    isFacingRight = false;
    phase = L3_BOSS_HUMAN_DIALOGUE;
    monsterState = MONSTER_IDLE;

    dialogueStep = 0;
    dialogueTimer = 0.0;
    introTimer = 0.0;
    transformTimer = 0.0;
    attackCooldownTimer = 2.0;

    drones.clear();
    droneProjectiles.clear();
    spikes.clear();

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

    // 1. Load Human Kael Animations
    animHumanIdle.InitSequence(loadSeq("Assets/Characters/Dr. Kael/Human/Idle/idle_%02d.png", 6), 12, true);
    animHumanTalk.InitSequence(loadSeq("Assets/Characters/Dr. Kael/Human/talk/talk_%02d.png", 8), 10, true);
    animHumanSummon.InitSequence(loadSeq("Assets/Characters/Dr. Kael/Human/summon drone/summon_drone_%02d.png", 8), 8, true);
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

    // First dialogue
    currentSpeaker = "Dr. Kael";
    currentText = "\"Ah, Arin... You survived my little biological experiments. But Project Genesis cannot be stopped!\"";
}

void Level3Boss::AdvanceDialogue() {
    dialogueStep++;
    if (dialogueStep == 1) {
        currentSpeaker = "Arin";
        currentText = "\"Where is Subject Luna, Kael?! What have you done with the antidote?\"";
    }
    else if (dialogueStep == 2) {
        currentSpeaker = "Dr. Kael";
        currentText = "\"Antidote? Creation requires destruction! Behold the future of humanity! Drones, purge the intruder!\"";
    }
    else if (dialogueStep >= 3) {
        phase = L3_BOSS_HUMAN_ANGRY_DRONES;
        introTimer = 0.0;
        // Spawn opening drone assault
        SpawnDrone(x - 200, y + 250);
        SpawnDrone(x + 200, y + 250);
    }
}

void Level3Boss::SpawnDrone(double spawnX, double spawnY) {
    Drone d;
    d.x = spawnX;
    d.y = spawnY;
    d.active = true;
    d.hp = 35;
    
    std::vector<unsigned int> seq;
    for (int i = 1; i <= 4; ++i) {
        char p[160];
        sprintf_s(p, sizeof(p), "Assets/Characters/Dr. Kael/Human/Drone Attack/Combat Drone/Idle/drone_idle_%02d.png", i);
        unsigned int tex = iLoadImage((char*)GetAssetPath(p).c_str());
        if (tex != 0) seq.push_back(tex);
    }
    d.anim.InitSequence(seq, 8, true);
    drones.push_back(d);
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

void Level3Boss::Update(Player& player, float dt) {
    // ------------------------------------------------------------------------
    // PHASE 1: Human Dialogue & Intro Sequence
    // ------------------------------------------------------------------------
    if (phase == L3_BOSS_HUMAN_DIALOGUE) {
        animHumanTalk.Update();
        return;
    }

    if (phase == L3_BOSS_HUMAN_ANGRY_DRONES) {
        introTimer += dt;
        animHumanSummon.Update();

        // Update drones
        for (auto& d : drones) {
            if (!d.active) continue;
            d.anim.Update();
            d.timer += dt;

            // Drone hovering AI
            d.targetX = player.x + (d.x < player.x ? -180.0 : 180.0);
            d.targetY = player.y + 160.0 + sin(d.timer * 3.0) * 30.0;
            d.x += (d.targetX - d.x) * 0.05;
            d.y += (d.targetY - d.y) * 0.05;

            d.attackCooldown += dt;
            if (d.attackCooldown >= 2.0) {
                d.attackCooldown = 0.0;
                // Fire projectile at Arin
                DroneProjectile p;
                p.x = d.x;
                p.y = d.y - 20;
                double dx = player.x - d.x;
                double dy = (player.y + 60) - d.y;
                double dist = sqrt(dx*dx + dy*dy);
                if (dist > 0.1) {
                    p.vx = (dx / dist) * 450.0;
                    p.vy = (dy / dist) * 450.0;
                }
                p.active = true;
                
                std::vector<unsigned int> seq;
                for (int i = 1; i <= 6; ++i) {
                    char b[160];
                    sprintf_s(b, sizeof(b), "Assets/Characters/Dr. Kael/Human/Drone Attack/Drone Energy Projectile/projectile_%02d.png", i);
                    unsigned int tex = iLoadImage((char*)GetAssetPath(b).c_str());
                    if (tex != 0) seq.push_back(tex);
                }
                p.anim.InitSequence(seq, 6, true);
                droneProjectiles.push_back(p);
            }
        }

        // After 4.0s of drone attack, transition to serum injection
        if (introTimer >= 4.0) {
            phase = L3_BOSS_HUMAN_INJECT_SERUM;
            transformTimer = 0.0;
            animHumanInject.Reset();
        }
    }
    else if (phase == L3_BOSS_HUMAN_INJECT_SERUM) {
        transformTimer += dt;
        animHumanInject.Update();
        if (transformTimer >= 2.0) {
            phase = L3_BOSS_HUMAN_TRANSFORMING;
            transformTimer = 0.0;
            animHumanTransform.Reset();
        }
    }
    else if (phase == L3_BOSS_HUMAN_TRANSFORMING) {
        transformTimer += dt;
        animHumanTransform.Update();
        if (transformTimer >= 3.0) {
            phase = L3_BOSS_MONSTER_ACTIVE;
            width = 280;
            height = 280;
            monsterState = MONSTER_IDLE;
            attackCooldownTimer = 1.5;
        }
    }

    // Update Drone Projectiles
    for (auto& p : droneProjectiles) {
        if (!p.active) continue;
        p.x += p.vx * dt;
        p.y += p.vy * dt;
        p.anim.Update();

        // Player collision
        if (abs(p.x - (player.x + player.width / 2.0)) < 40.0 &&
            abs(p.y - (player.y + player.height / 2.0)) < 50.0) {
            player.TakeDamage(p.damage);
            p.active = false;
        }

        // Out of bounds cleanup
        if (p.x < player.x - 1000 || p.x > player.x + 1000 || p.y < 0 || p.y > 800) {
            p.active = false;
        }
    }

    // Update Ground Spikes
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

        switch (monsterState) {
        case MONSTER_IDLE:
            animMonsterIdle.Update();
            if (attackCooldownTimer <= 0.0) {
                double dist = abs(player.x - x);
                if (dist < 180.0) {
                    monsterState = MONSTER_ATTACK_CLAW;
                    animMonsterClaw.Reset();
                    clawDamageDealt = false;
                    stateTimer = 0.0;
                } else if (dist > 350.0 && (hp < maxHp * 0.66)) {
                    monsterState = MONSTER_ATTACK_CHARGE;
                    animMonsterCharge.Reset();
                    chargeStartX = x;
                    chargeTargetX = player.x;
                    chargeDamageDealt = false;
                    stateTimer = 0.0;
                } else {
                    monsterState = MONSTER_ATTACK_SPIKES;
                    TriggerSpikeAttack(player.x, 185.0);
                    stateTimer = 0.0;
                }
            }
            break;

        case MONSTER_CHASE:
            animMonsterWalk.Update();
            if (isFacingRight) x += 140.0 * dt;
            else x -= 140.0 * dt;

            if (abs(player.x - x) < 180.0) {
                monsterState = MONSTER_ATTACK_CLAW;
                animMonsterClaw.Reset();
                clawDamageDealt = false;
                stateTimer = 0.0;
            }
            break;

        case MONSTER_ATTACK_CLAW:
            animMonsterClaw.Update();
            stateTimer += dt;
            if (!clawDamageDealt && stateTimer >= 0.4) {
                if (abs(player.x - x) < 210.0 && abs(player.y - y) < 150.0) {
                    player.TakeDamage(40);
                    clawDamageDealt = true;
                }
            }
            if (stateTimer >= 1.0) {
                monsterState = MONSTER_IDLE;
                attackCooldownTimer = (hp < 400) ? 0.8 : 1.5;
            }
            break;

        case MONSTER_ATTACK_SPIKES:
            animMonsterIdle.Update();
            stateTimer += dt;
            if (stateTimer >= 1.2) {
                monsterState = MONSTER_IDLE;
                attackCooldownTimer = (hp < 400) ? 1.0 : 2.0;
            }
            break;

        case MONSTER_ATTACK_CHARGE:
            animMonsterCharge.Update();
            stateTimer += dt;
            if (isFacingRight) x += 380.0 * dt;
            else x -= 380.0 * dt;

            if (!chargeDamageDealt && abs(player.x - x) < 160.0 && abs(player.y - y) < 150.0) {
                player.TakeDamage(50);
                chargeDamageDealt = true;
            }
            if (stateTimer >= 1.2 || abs(x - chargeStartX) > 600.0) {
                monsterState = MONSTER_IDLE;
                attackCooldownTimer = 2.0;
            }
            break;

        case MONSTER_STAGGER:
            animMonsterStagger.Update();
            stateTimer += dt;
            if (stateTimer >= 0.8) {
                monsterState = MONSTER_IDLE;
            }
            break;

        case MONSTER_DEAD:
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
    return (abs(px - x) < (pw + width) / 2.0 && abs(py - y) < (ph + height) / 2.0);
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
            p.anim.Render(px - 25, py - 25, 50, 50, true);
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
    if (phase == L3_BOSS_HUMAN_DIALOGUE) {
        animHumanTalk.Render(renderX - width/2, renderY, width, height, isFacingRight);
    }
    else if (phase == L3_BOSS_HUMAN_ANGRY_DRONES) {
        animHumanSummon.Render(renderX - width/2, renderY, width, height, isFacingRight);
    }
    else if (phase == L3_BOSS_HUMAN_INJECT_SERUM) {
        animHumanInject.Render(renderX - width/2, renderY, width, height, isFacingRight);
    }
    else if (phase == L3_BOSS_HUMAN_TRANSFORMING) {
        animHumanTransform.Render(renderX - width/2, renderY, width + 40, height + 40, isFacingRight);
    }
    // 5. Render Transformed Monster Kael Form
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
}
