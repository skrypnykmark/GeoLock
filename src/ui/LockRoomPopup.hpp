#pragma once

namespace geolock {
    enum class LockRoomContext { General, TargetPage, PracticePage };

    void showLockRoom(LockRoomContext context = LockRoomContext::General);
}
