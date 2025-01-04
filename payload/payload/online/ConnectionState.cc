#include "ConnectionState.hh"

ConnectionState::ConnectionState(JKRHeap *heap, DH::PK serverPK)
    : m_heap(heap), m_serverPK(serverPK) {}

ConnectionState::~ConnectionState() {}
