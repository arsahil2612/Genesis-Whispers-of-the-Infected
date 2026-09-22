#include "Inventory.h"
#include <windows.h>

InventoryState InventorySystem::state = INVENTORY_MAIN;
float InventorySystem::controlsFadeAlpha = 0.0f;

void InventorySystem::Initialize() {
    state = INVENTORY_MAIN;
    controlsFadeAlpha = 0.0f;
}

void InventorySystem::OpenControls() {
    state = CONTROLS_SCREEN;
    controlsFadeAlpha = 0.0f;
}

void InventorySystem::CloseControls() {
    state = INVENTORY_MAIN;
}

void InventorySystem::Update(float dt) {
    if (state == CONTROLS_SCREEN) {
        controlsFadeAlpha += dt * 5.0f;
        if (controlsFadeAlpha > 1.0f) controlsFadeAlpha = 1.0f;
    } else {
        controlsFadeAlpha -= dt * 5.0f;
        if (controlsFadeAlpha < 0.0f) controlsFadeAlpha = 0.0f;
    }
}
