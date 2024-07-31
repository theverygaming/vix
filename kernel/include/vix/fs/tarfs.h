#pragma once

namespace fs::filesystems::tarfs {
    bool init(void *location);
    void deinit();
    void mountInVFS();
}
