#pragma once

#include <common/storage/Storage.hh>
extern "C" {
#include <dolphin/OSMessage.h>
#include <dolphin/OSThread.h>
}

class AssetsDirCreator : public Storage::Observer {
public:
    static void Init();

private:
    AssetsDirCreator();

    void onAdd(const char *prefix) override;
    void onRemove(const char *prefix) override;

    void *run();

    static void *Run(void *param);

    OSMessageQueue m_queue;
    Array<OSMessage, 1> m_messages;
    Array<u8, 4 * 1024> m_stack;
    OSThread m_thread;

    static AssetsDirCreator *s_instance;
};
