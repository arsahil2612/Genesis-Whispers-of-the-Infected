#include "Level3Manager.h"
#include "igraphics_declarations.h"
#include "asset_loader.h"
#include <cstdio>
#include <cmath>

Level3Manager::Level3Manager()
    : state(LEVEL3_PLAYING),
      loadingTimer(0.0f),
      minLoadingTime(3.0f),
      pulseTimer(0.0f),
      texLoadingBg(0),
      assetsLoaded(false),
      savedArenaX(7200.0)
{
}

Level3Manager::~Level3Manager()
{
}

void Level3Manager::Initialize()
{
    state = LEVEL3_PLAYING;
    loadingTimer = 0.0f;
    pulseTimer = 0.0f;
    assetsLoaded = false;
}

void Level3Manager::StartBossLoading(double arenaX)
{
    savedArenaX = arenaX;
    state = LEVEL3_BOSS_LOADING;
    loadingTimer = 0.0f;
    pulseTimer = 0.0f;
    assetsLoaded = false;
    printf("[GENESIS Level3Manager] Transitioning to LEVEL3_BOSS_LOADING for Arena at X=%.1f\n", savedArenaX);
}

void Level3Manager::StartBossFight()
{
    state = LEVEL3_BOSS_FIGHT;
    printf("[GENESIS Level3Manager] Transitioning to LEVEL3_BOSS_FIGHT\n");
}

void Level3Manager::Update(float dt, Player& player, Level3Boss& boss)
{
    pulseTimer += dt;

    if (state == LEVEL3_BOSS_LOADING)
    {
        loadingTimer += dt;

        // Stop normal player movement temporarily during loading
        player.vx = 0.0;
        player.vy = 0.0;
        if (player.state == STATE_RUN || player.state == STATE_WALK)
        {
            player.SetState(STATE_IDLE);
        }

        // Defer loading assets so the loading screen gets drawn first
        if (!assetsLoaded && loadingTimer > 0.1f)
        {
            boss.PreloadAssets();
            boss.Initialize(savedArenaX, 185.0); // 185.0 is kLevel1GroundY
            assetsLoaded = true;
        }

        // Check if minimum loading display time elapsed (3.0 seconds) and assets are ready
        if (loadingTimer >= minLoadingTime && assetsLoaded)
        {
            printf("[GENESIS Level3Manager] Loading Complete (%.2fs). Transitioning to LEVEL3_BOSS_INTRO\n", loadingTimer);
            state = LEVEL3_BOSS_INTRO;

            // Position Arin on the left side of arena facing right
            player.x = savedArenaX + 150.0;
            player.y = 185.0; // kLevel1GroundY
            player.vx = 0.0;
            player.vy = 0.0;
            player.isFacingRight = true;

            // Change phase to Intro
            boss.phase = L3_BOSS_HUMAN_INTRO;
        }
    }
    else if (state == LEVEL3_BOSS_INTRO || state == LEVEL3_BOSS_FIGHT)
    {
        // Lock player movement during intro, dialogue, and transformation cinematics
        if (boss.IsInIntro() || boss.IsInDialogue() || boss.IsPrepareSerumPhase() || boss.IsInjectSerumPhase() || boss.IsSerumCompletePhase() || boss.IsTransformationPreparePhase() || boss.IsKaelTransformingPhase())
        {
            player.vx = 0.0;
            player.vy = 0.0;
            if (player.state == STATE_RUN || player.state == STATE_WALK)
            {
                player.SetState(STATE_IDLE);
            }
        }

        // Advance Dr. Kael boss logic & AI steps
        boss.Update(player, dt);

        if (state == LEVEL3_BOSS_INTRO && (boss.IsActiveMonster() || boss.phase == L3_BOSS_MONSTER_ACTIVE))
        {
            state = LEVEL3_BOSS_FIGHT;
            printf("[GENESIS Level3Manager] Transitioned to LEVEL3_BOSS_FIGHT!\n");
        }
    }
}

void Level3Manager::Draw()
{
    if (state != LEVEL3_BOSS_LOADING)
    {
        return;
    }

    // Lazy load cinematic loading background image
    if (texLoadingBg == 0)
    {
        std::string bgPath = GetAssetPath("Assets/Backgrounds/Level3/Final arena entering background/genesis_final_arena_loading.png");
        texLoadingBg = iLoadImage((char*)bgPath.c_str());
    }

    // 1. Display full screen loading image covering game window (1280x720)
    if (texLoadingBg != 0)
    {
        iShowImage(0, 0, 1280, 720, texLoadingBg);
    }
    else
    {
        // Dark theme fallback
        iSetColor(12, 14, 22);
        iFilledRectangle(0, 0, 1280, 720);
    }

    // 2. Draw cinematic dark gradient overlay bar at bottom
    for (int y = 0; y < 190; ++y)
    {
        float alphaRatio = (190.0f - y) / 190.0f;
        int darkVal = (int)(18.0f * alphaRatio);
        iSetColor(darkVal, darkVal, darkVal + 6);
        iLine(0, y, 1280, y);
    }

    // Calculate progress ratio (0.0 to 1.0)
    float progressRatio = loadingTimer / minLoadingTime;
    if (progressRatio > 1.0f) progressRatio = 1.0f;
    int percent = (int)(progressRatio * 100.0f);

    // 3. Progress Bar Geometry
    int barW = 720;
    int barH = 24;
    int barX = (1280 - barW) / 2;
    int barY = 95;

    // Progress Bar Container (Dark frame with glowing crimson border)
    iSetColor(18, 22, 32);
    iFilledRectangle(barX - 4, barY - 4, barW + 8, barH + 8);
    iSetColor(140, 35, 50); // Dark Crimson Red Border
    iRectangle(barX - 4, barY - 4, barW + 8, barH + 8);

    // Progress Bar Fill (Glowing Crimson/Neon Red Gradient)
    int fillW = (int)(barW * progressRatio);
    if (fillW > 0)
    {
        iSetColor(190, 45, 65);
        iFilledRectangle(barX, barY, fillW, barH);

        // Highlight glow strip
        float pulse = 0.5f + 0.5f * sinf(pulseTimer * 6.0f);
        int glowR = (int)(225 + 30 * pulse);
        int glowG = (int)(70 + 40 * pulse);
        int glowB = (int)(90 + 40 * pulse);
        iSetColor(glowR, glowG, glowB);
        iFilledRectangle(barX, barY + barH / 2, fillW, barH / 2);
    }

    // Inner Border
    iSetColor(255, 95, 110);
    iRectangle(barX, barY, barW, barH);

    // 4. Animated Loading Indicator (Rotating Dot Ring)
    int indicatorX = barX - 40;
    int indicatorY = barY + barH / 2;
    float spinAngle = pulseTimer * 5.0f;
    for (int i = 0; i < 8; ++i)
    {
        float angle = spinAngle + i * (3.14159f / 4.0f);
        float radius = 14.0f;
        int dx = (int)(indicatorX + cosf(angle) * radius);
        int dy = (int)(indicatorY + sinf(angle) * radius);
        int dotBrightness = (int)(110 + 145 * ((float)i / 7.0f));
        iSetColor(255, dotBrightness, dotBrightness);
        iFilledCircle(dx, dy, (i == 7) ? 4 : 2);
    }

    // 5. Loading Text & Status Indicators
    char percentBuf[32];
    sprintf_s(percentBuf, sizeof(percentBuf), "%d%%", percent);
    iSetColor(255, 235, 240);
    iText(barX + barW + 15, barY + 5, percentBuf, GLUT_BITMAP_HELVETICA_18);

    // Main Status Text with Animated Ellipsis
    char titleBuf[64];
    int dotCount = ((int)(pulseTimer * 3.5f)) % 4;
    if (dotCount == 0) sprintf_s(titleBuf, sizeof(titleBuf), "PREPARING FINAL ARENA");
    else if (dotCount == 1) sprintf_s(titleBuf, sizeof(titleBuf), "PREPARING FINAL ARENA.");
    else if (dotCount == 2) sprintf_s(titleBuf, sizeof(titleBuf), "PREPARING FINAL ARENA..");
    else sprintf_s(titleBuf, sizeof(titleBuf), "PREPARING FINAL ARENA...");

    iSetColor(255, 255, 255);
    iText(barX, barY + barH + 18, titleBuf, GLUT_BITMAP_HELVETICA_18);

    // Dynamic Asset Loading Status Message
    const char* statusMsg = (loadingTimer < 1.2f) ? "LOADING DR. KAEL SPRITES & ANIMATION SEQUENCES..." :
                            (loadingTimer < 2.2f) ? "LOADING COMBAT DRONES & PROJECTILE EFFECTS..." :
                                                    "PRELOADING AUDIO ASSETS & ARENA ENVIRONMENT...";
    iSetColor(210, 190, 200);
    iText(barX, barY - 24, (char*)statusMsg, GLUT_BITMAP_HELVETICA_12);
}
