#include "ReplayManager.hh"

#include <cube/Arena.hh>
#include <portable/Algorithm.hh>

extern "C" {
#include <stdio.h>
#include <string.h>
}

void ReplayManager::Init() {
    s_instance = new (MEM1Arena::Instance(), 0x4) ReplayManager;
}

ReplayManager *ReplayManager::Instance() {
    return s_instance;
}

ReplayManager::ReplayManager() {
    StorageScanner *param = this;
    OSCreateThread(&m_thread, Run, param, m_stack.values() + m_stack.count(), m_stack.count(), 27,
            0);
}

OSThread &ReplayManager::thread() {
    return m_thread;
}

void ReplayManager::process() {
    Array<char, 256> path;
    snprintf(path.values(), path.count(), "main:/ddd/replays");
    Storage::CreateDir(path.values(), Storage::Mode::WriteAlways);
    Storage::NodeInfo nodeInfo;
    addReplays(path, nodeInfo);
}

void ReplayManager::addReplays(Array<char, 256> &path, Storage::NodeInfo &nodeInfo) {
    u32 length = strlen(path.values());
    for (Storage::DirHandle dir(path.values()); dir.read(nodeInfo);) {
        snprintf(path.values() + length, path.count() - length, "/%s", nodeInfo.name.values());
        if (nodeInfo.type == Storage::NodeType::Dir) {
            addReplays(path, nodeInfo);
        } else {
            addReplay(path);
        }
    }
    path[length] = '\0';
}

void ReplayManager::addReplay(const Array<char, 256> &path) {
    alignas(0x20) u8 buffer[512];
    u32 size;
    if (!Storage::ReadFile(path.values(), buffer, Count(buffer), &size)) {
        return;
    }
}

ReplayManager *ReplayManager::s_instance = nullptr;
