#include "EnvironmentalEventManager.h"

EnvironmentalEventManager::EnvironmentalEventManager() {
    Reset();
}

void EnvironmentalEventManager::Reset() {
    m_activeEvents.clear();
}

void EnvironmentalEventManager::Fire(int eventID) {
    ActiveAmbientEvent ev;
    ev.eventID = eventID;
    ev.remainingTime = 3.0f; // Ambient effect duration
    m_activeEvents.push_back(ev);
}

void EnvironmentalEventManager::Update(float dt) {
    for (size_t i = 0; i < m_activeEvents.size(); ) {
        m_activeEvents[i].remainingTime -= dt;
        if (m_activeEvents[i].remainingTime <= 0.0f) {
            m_activeEvents.erase(m_activeEvents.begin() + i);
        } else {
            ++i;
        }
    }
}
