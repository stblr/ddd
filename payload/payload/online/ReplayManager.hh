#pragma once

#include "payload/StorageScanner.hh"

class ReplayManager : public StorageScanner {
public:
    static void Init();
    static ReplayManager *Instance();

private:
    ReplayManager();

    OSThread &thread() override;
    void process() override;

    void addReplays(Array<char, 256> &path, Storage::NodeInfo &nodeInfo);
    void addReplay(const Array<char, 256> &path);

    Array<u8, 4 * 1024> m_stack;
    OSThread m_thread;

    static ReplayManager *s_instance;
};
