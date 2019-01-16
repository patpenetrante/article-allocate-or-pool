#ifndef _PACKET_H
#define _PACKET_H

#include <cstdint>
#include <vector>

#include "buffer.h"

#define ALLOCATION 0

const unsigned IP_PROTOCOL = 1;

class IPAddress
{
};

#if ALLOCATION

class IP4Address : public IPAddress
{
    const uint32_t addr;

public:
    IP4Address (uint32_t addr) : addr (addr)
    {
    }
};

class IPHeader
{
public:

    const IPAddress * const src_addr;
    const IPAddress * const dst_addr;
    const uint32_t src_port;
    const uint32_t dst_port;
    const uint32_t protocol;

    IPHeader(ReadBuffer &buf) :
        src_addr (new IP4Address(buf.get_32())),
        dst_addr (new IP4Address(buf.get_32())),
        src_port (buf.get_16 ()),
        dst_port (buf.get_16 ()),
        protocol (buf.get_32 ())
    {
    }

    ~IPHeader()
    {
        delete src_addr;
        delete dst_addr;
    }
};

class IPPacket
{
public:
    const IPHeader * const header;

private:
    const ReadBuffer buf;

public:
    IPPacket (const IPHeader *header, const ReadBuffer & buf) : header (header), buf (buf)
    {
    }

    ~IPPacket()
    {
        delete header;
    }
};

class Packet
{
public:
    static const size_t SIZE = 1024;
    unsigned char * const buf;
    size_t length;
    IPPacket *ip_packet;

    Packet() : buf(new unsigned char[SIZE]), length (0), ip_packet (nullptr)
    {
    }

    ~Packet()
    {
        delete[] buf;
        delete ip_packet;
    }

    void free()
    {
        delete ip_packet;
    }

    IPPacket * parse_ip(ReadBuffer & buf)
    {
        if (buf.get_32() != IP_PROTOCOL) return nullptr;
        IPHeader * header = new IPHeader(buf);
        ip_packet = new IPPacket(header, buf);
        return ip_packet;
    }
};

#else // NO_ALLOCATION

class IP4Address : public IPAddress
{
    uint32_t addr;

public:
    IP4Address() : addr(0)
    {
    }

    void set (uint32_t addr)
    {
        this->addr = addr;
    }
};

class IPHeader
{
public:
    const IPAddress * src_addr;
    const IPAddress * dst_addr;
    uint32_t src_port;
    uint32_t dst_port;
    uint32_t protocol;

private:
    IP4Address _src_addr;
    IP4Address _dst_addr;

public:
    IPHeader() : src_addr (nullptr), dst_addr (nullptr)
    {}

    void resd(ReadBuffer &buf)
    {
        src_addr = &_src_addr;
        dst_addr = &_dst_addr;
        _src_addr.set(buf.get_32());
        _dst_addr.set(buf.get_32());
        src_port = buf.get_16();
        dst_port = buf.get_16();
        protocol = buf.get_32();
    }

    void free()
    {
        src_addr = nullptr;
        dst_addr = nullptr;
    }

    ~IPHeader()
    {
        free();
    }
};

class IPPacket
{
public:
    const IPHeader * header;

private:
    ReadBuffer buf;
    IPHeader _header;

public:
    IPPacket() : header(nullptr), buf (nullptr, 0)
    {
    }

    void read (ReadBuffer & buf)
    {
        _header.resd(buf);
        header = &_header;
        this->buf = buf;
    }

    void free()
    {
        _header.free();
        header = nullptr;
    }

    ~IPPacket()
    {
        free();
    }
};

class Packet
{
public:
    static const size_t SIZE = 1024;
    unsigned char * const buf;
    size_t length;
    IPPacket *ip_packet;

private:
    IPPacket _ip_packet;

public:
    Packet() : buf(new unsigned char[SIZE]), length(0), ip_packet(nullptr)
    {
    }

    ~Packet()
    {
        delete[] buf;
    }

    void free()
    {
        _ip_packet.free();
        ip_packet = nullptr;
    }

    IPPacket * parse_ip(ReadBuffer & buf)
    {
        if (buf.get_32() != IP_PROTOCOL) return nullptr;
        _ip_packet.read(buf);
        ip_packet = &_ip_packet;
        return ip_packet;
    }
};
#endif

class PacketManager
{
public:
    virtual Packet* get() = 0;
    virtual void free(Packet *packet) = 0;
};

class DummyPacketManager : public PacketManager
{
    Packet* const packet = new Packet();

public:
    Packet* get()
    {
        return packet;
    }

    void free(Packet* packet)
    {
    }
};

class AllocatingPacketManager : public PacketManager
{
public:
    Packet* get()
    {
        return new Packet ();
    }

    void free(Packet* packet)
    {
        delete packet;
    }
};

class PoolingPacketManager : public PacketManager
{
    std::vector<Packet*> pool;
    const size_t size;

public:
    PoolingPacketManager(size_t size) : size (size)
    {
        for (size_t i = 0; i < size; i++) {
            pool.push_back(new Packet());
        }
    }

    ~PoolingPacketManager()
    {
        for (Packet* p : pool) {
            delete p;
        }
        pool.clear();
    }

    Packet* get()
    {
        if (pool.empty()) {
            return new Packet();
        }
        Packet * p = pool.back();
        pool.pop_back();
        return p;
    }

    void free(Packet* packet)
    {
        if (pool.size() < size) {
            packet->free();
            pool.push_back(packet);
        }
        else {
            delete packet;
        }
    }
};

#endif

