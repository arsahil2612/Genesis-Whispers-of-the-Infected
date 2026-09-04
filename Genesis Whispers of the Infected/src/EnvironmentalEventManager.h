#ifndef ENVIRONMENTAL_EVENT_MANAGER_H
#define ENVIRONMENTAL_EVENT_MANAGER_H

#include <vector>

struct ActiveAmbientEvent {
    int eventID;
    float remainingTime;
};

// ============================================================================
// Environmental Event Manager
// Triggers non-combat ambient visual/audio beats
// ============================================================================

class EnvironmentalEventManager {
public:
    EnvironmentalEventManager();

    void Update(float dt);
    void Fire(int eventID);
    void Reset();

private:
    std::vector<ActiveAmbientEvent> m_activeEvents;
};

#endif // ENVIRONMENTAL_EVENT_MANAGER_H
