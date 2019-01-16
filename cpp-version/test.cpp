#include <iostream>
#include <chrono>

#ifdef __linux__
#include <x86intrin.h>
#else
#include <intrin.h>
#endif

#include <queue>
#include <thread>
#include <stdio.h>
#include "packet.h"
#include "queue.h"
#include "timer.h"

const unsigned MAX_CAPTURING_DELAY_MS = 100;
const uint64_t REPORT_INTERVAL = 1000000;

const unsigned UDP_PROTOCOL = 1;
const unsigned TCP_PROTOCOL = 2;
const unsigned IP_ADDR_COUNT = 1024;
const unsigned PORT_COUNT = 1024;
const unsigned TCP_MODULE = 16;

const size_t A_INTERNAL_QUEUE_SIZE = 1000;
const size_t A_STORED_COUNT = 1000;

const size_t B_INTERNAL_QUEUE_SIZE = 100000;
const size_t B_STORED_COUNT = 100000;

const size_t C_INTERNAL_QUEUE_SIZE = 495000;
const size_t C_STORED_COUNT = 495000;

const size_t POOL_SIZE = 1000000;
const size_t MIX_POOL_SIZE = 200000;

template<class Q, class Timer> class DataSource
{
    Q &queue;
    Timer timer;

public:
    DataSource(Q &queue, int interval_ns) : queue(queue), timer(interval_ns)
    {
    }

    void operator()()
    {
        int32_t seq = 0;
        timer.start();
        for (uint64_t i = 0; ; i++) {
            timer.iteration(i);
            uint32_t elem = seq++;
            queue.write(elem);
        }
    }
};

class Reporter
{
    Queue<uint64_t> & queue;

public:
    Reporter(Queue<uint64_t> & queue) : queue(queue)
    {
    }

    void operator()()
    {
        auto start = std::chrono::system_clock::now();
        auto current_start = start;
        
        int count = 0;
        uint64_t total_lost = 0;
        uint64_t total_sent = 0;

        while (true) {
            uint64_t lost;
            queue.read(lost);

            auto now = std::chrono::system_clock::now();

            double since_start = std::chrono::duration<double>(now - start).count();
            double since_last = std::chrono::duration<double>(now - current_start).count();

            printf("%5.1f; %5.1f; lost: %8lu", since_start, since_last, (unsigned long) lost);
            ++count;
            if (count > 10) {
                total_lost += lost;
                total_sent += REPORT_INTERVAL + lost;
                double lost_frac = total_lost * 1.0 / total_sent;
                printf(" frac = %7.4f%% or %f", lost_frac * 100.0, lost_frac);
            }
            printf("\n");
            current_start = now;
        }
    }
};

class Test
{
    std::queue<Packet*> queue;
    PacketManager & manager;

    size_t INTERNAL_QUEUE_SIZE;
    size_t STORED_COUNT;

    unsigned num_gen = 0;
    std::vector<Packet *> parsed;

public:
    Test (PacketManager & manager, size_t internal_queue_size, size_t stored_count) :
        manager (manager), INTERNAL_QUEUE_SIZE(internal_queue_size), STORED_COUNT(stored_count)
    {
        parsed.resize(STORED_COUNT);
    }

private:

    unsigned next_positive()
    {
        return ++num_gen;
    }

    void simulate_packet(Packet * p)
    {
        size_t len = next_positive() % 64 + 50;
        WriteBuffer buf(p->buf, Packet::SIZE);
        buf.put32(IP_PROTOCOL);
        buf.put32(next_positive() % IP_ADDR_COUNT);
        buf.put32(next_positive() % IP_ADDR_COUNT);
        buf.put16((short)(next_positive() % PORT_COUNT));
        buf.put16((short)(next_positive() % PORT_COUNT));
        buf.put32(next_positive() % TCP_MODULE == 0 ? TCP_PROTOCOL : UDP_PROTOCOL);
        buf.put32((uint32_t) len);
        p->length = buf.length();
    }

    void enqueue(Packet * p)
    {
        queue.push(p);
    }

    void dequeue(Packet * p)
    {
        if (queue.size() >= INTERNAL_QUEUE_SIZE) {
            Packet * p = queue.front();
            queue.pop();
            if (!process(p)) {
                manager.free(p);
            }
        }
    }

    bool process(Packet * p)
    {
        ReadBuffer buf(p->buf, p->length);
        IPPacket * ip = p->parse_ip (buf);
        if (!ip) return false;
        if (ip->header->protocol == TCP_PROTOCOL) {
            size_t ind = next_positive () % STORED_COUNT;
            Packet * old = parsed[ind];
            if (old) {
                manager.free(old);
            }
            parsed[ind] = p;
        }
        else {
            manager.free(p);
        }
        return true;
    }

public:

    void run(Queue<uint32_t> &in_queue, Queue<uint64_t> &report_queue)
    {
        uint32_t seq = 0;
        uint64_t lost_count = 0;
        uint64_t received_count = 0;
        while (true) {
            uint32_t elem;
            in_queue.read(elem);
            lost_count += (int)(elem - seq);
            seq = elem + 1;
            ++received_count;
            if (received_count == REPORT_INTERVAL) {
                report_queue.write(lost_count);
                lost_count = received_count = 0;
            }
            Packet *p = manager.get();
            if (!p) {
                std::cout << "Failure to allocate packet\n";
                exit(1);
            }
            simulate_packet(p);
            enqueue(p);
            dequeue(p);
        }
    }

    void batch_run()
    {
        long N = REPORT_INTERVAL * 10;
        long total_count = -N * 5;
        auto t0 = std::chrono::system_clock::now();
        while (true) {
            auto t1 = std::chrono::system_clock::now();
            for (int i = 0; i < N; i++) {
                Packet *p = manager.get();
                if (!p) {
                    std::cout << "Failure to allocate packet\n";
                    std::exit(1);
                }
                simulate_packet(p);
                enqueue(p);
                dequeue(p);
            }
            total_count += N;
            if (total_count == 0) {
                t0 = std::chrono::system_clock::now();
            }
            auto t2 = std::chrono::system_clock::now();
            double t = std::chrono::duration<double> (t2 - t1).count();
            std::cout << "Time for " << N << ": " << t <<"; packets/sec: " << N / t * 1.0e-6 << "M; ns/packet: " << t * 1.0e9 / N;
            if (total_count > 0) {
                std::cout << "; avg: " << std::chrono::duration<double>(t2 - t0).count() * 1.0e9 / total_count;
            }
            std::cout << "\n";
        }
    }
};

int main(int argc, const char * argv[])
{
    if (argc <= 4) {
        std::cout << "Three arguments expected: test case, strategy and time interval\n";
        return 1;
    }

    size_t internal_queue_size;
    size_t stored_count;

    std::string test_case(argv[1]);
    if (test_case == "A") {
        internal_queue_size = A_INTERNAL_QUEUE_SIZE;
        stored_count = A_STORED_COUNT;
    } else if (test_case == "B") {
        internal_queue_size = B_INTERNAL_QUEUE_SIZE;
        stored_count = B_STORED_COUNT;
    } else if (test_case == "C") {
        internal_queue_size = C_INTERNAL_QUEUE_SIZE;
        stored_count = C_STORED_COUNT;
    } else {
        std::cout << "Case (A/B/C) not defined\n";
        return 1;
    }

    PacketManager * manager;
    std::string strategy (argv[2]);
    if (strategy == "alloc") {
        manager = new AllocatingPacketManager();
    } else if (strategy == "mix") {
        manager = new PoolingPacketManager(MIX_POOL_SIZE);
    } else if (strategy == "pool") {
        manager = new PoolingPacketManager(POOL_SIZE);
    } else if (strategy == "dummy") {
        manager = new DummyPacketManager();
    } else {
        std::cout << "Packet Manager not defined\n";
        return 1;
    }

    Test test(*manager, internal_queue_size, stored_count);
    std::string interval(argv[3]);
    if (interval == "batch") {
        test.batch_run();
    }
    else {
        hires_timer::calibrate();
        unsigned source_interval_ns = (unsigned) atoi(interval.c_str());
        size_t queue_size = (size_t) (MAX_CAPTURING_DELAY_MS * 1000000L / source_interval_ns);

        DualArrayAsyncQueue<uint32_t> in_queue(queue_size);
        DataSource<DualArrayAsyncQueue<uint32_t>, hires_timer> source(in_queue, source_interval_ns);
        DualArrayAsyncQueue<uint64_t> reporter_queue(100);
        Reporter reporter (reporter_queue);
        std::thread rep_thread (reporter);
        std::thread source_thread (source);
        test.run(in_queue, reporter_queue);
    }

    return 0;
}