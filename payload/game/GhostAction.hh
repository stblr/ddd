#pragma once

class GhostAction {
public:
    enum {
        None = 0,
        Load = 1,
        Save = 2,
        Check = 3,
    };

private:
    GhostAction();
};
