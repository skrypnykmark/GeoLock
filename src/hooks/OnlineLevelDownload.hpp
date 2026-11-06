#pragma once

#include <Geode/utils/function.hpp>

class GJGameLevel;

namespace geolock {
    bool requestOnlineLevel(int id, geode::Function<void(GJGameLevel*)> callback);
    void cancelOnlineLevelRequest(int id);
}
