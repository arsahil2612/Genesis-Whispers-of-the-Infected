#ifndef INVENTORY_H
#define INVENTORY_H

enum InventoryState {
    INVENTORY_MAIN,
    CONTROLS_SCREEN
};

class InventorySystem {
public:
    static InventoryState state;
    static float controlsFadeAlpha;

    static void Initialize();
    static void OpenControls();
    static void CloseControls();
    static void Update(float dt);
};

#endif // INVENTORY_H
