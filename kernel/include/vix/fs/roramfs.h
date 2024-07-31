#pragma once

namespace fs::filesystems::roramfs {
    void init(void *location);
    void deinit();
    void mountInVFS();
}
