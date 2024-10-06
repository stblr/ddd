#pragma once

#include "payload/StorageScanner.hh"

#include <common/Ring.hh>
#include <common/UniquePtr.hh>
#include <common/storage/Storage.hh>
#include <jsystem/JKRHeap.hh>

class ServerManager : public StorageScanner {
public:
    enum {
        MaxServerCount = 32,
    };

    struct Address {
        Array<u8, 4> address;
        u16 port;
    };

    class Server {
    public:
        Server(Array<char, INIFieldSize> name, Array<char, INIFieldSize> address,
                Array<u8, 32> publicKey);
        ~Server();

        const char *name() const;
        const char *address() const;
        Array<u8, 32> publicKey() const;

    private:
        Array<char, INIFieldSize> m_name;
        Array<char, INIFieldSize> m_address;
        Array<u8, 32> m_publicKey;
    };

    void start();

    u32 serverCount() const;
    const Server &server(u32 index) const;

    static void Init();
    static ServerManager *Instance();

private:
    struct ServerINI {
        Array<Array<char, INIFieldSize>, KartLocale::Language::Count> localizedNames;
        Array<char, INIFieldSize> fallbackName;
        Array<char, INIFieldSize> address;
        Array<char, INIFieldSize> publicKey;
    };

    ServerManager();

    OSThread &thread() override;
    void process() override;

    void addServers(Array<char, 256> &path, Storage::NodeInfo &nodeInfo);
    void addServer(const Array<char, 256> &path);
    void sortServersByName();

    static bool CompareServersByName(const UniquePtr<Server> &a, const UniquePtr<Server> &b);

    Array<u8, 16 * 1024> m_stack;
    OSThread m_thread;
    JKRHeap *m_heap;
    Ring<UniquePtr<Server>, MaxServerCount> m_servers;

    static ServerManager *s_instance;
};
