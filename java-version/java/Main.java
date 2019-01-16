import java.nio.ByteBuffer;
import java.util.ArrayDeque;

public class Main
{
    PacketManager manager;
    IntQueue in_queue;
    IntQueue report_queue = new DualArrayAsyncIntQueue (1000);

    static final int MAX_CAPTURING_DELAY_MS = 100;
    
    static final int REPORT_INTERVAL = 1000000;
    
    static final int POOL_SIZE = 1000000;
    static final int MIX_POOL_SIZE = 200000;

    static int INTERNAL_QUEUE_SIZE;
    static int STORED_COUNT;
    
    static final int A_INTERNAL_QUEUE_SIZE = 1000;
    static final int A_STORED_COUNT = 1000;

    static final int B_INTERNAL_QUEUE_SIZE = 100000;
    static final int B_STORED_COUNT = 100000;

    static final int C_INTERNAL_QUEUE_SIZE = 495000;
    static final int C_STORED_COUNT = 495000;
    
    static final int IP_PROTOCOL = 1;
    static final int UDP_PROTOCOL = 1;
    static final int TCP_PROTOCOL = 2;
    static final int IP_ADDR_COUNT = 1024;
    static final int PORT_COUNT = 1024;
    static final int TCP_MODULE = 16;
    
    ArrayDeque<Packet> deque = new ArrayDeque<> ();
    Packet[] parsed = new Packet [STORED_COUNT];
    
    Main (PacketManager manager, IntQueue in_queue)
    {
        this.manager = manager;
        this.in_queue  = in_queue;
        new Reporter (report_queue).start ();
    }
    
    int count = 0;
    
    private int next_positive ()
    {
        // use instead of Random as a faster version
        return (++ count) & 0x7FFFFFFF;
    }
    
    void run (int intervalNS)
    {
        new NativeDataSource (in_queue, intervalNS).start ();
        while (true) {
            int seq = 0;
            int lost_count = 0;
            int received_count = 0;
            while (true) {
                int elem = in_queue.read ();
                lost_count += elem - seq;
                seq = elem + 1;
                ++received_count;
                if (received_count == REPORT_INTERVAL) {
                    report_queue.write(lost_count);
                    lost_count = received_count = 0;
                }
                Packet p = manager.get ();
                if (p == null) {
                    throw new RuntimeException ("Failure to allocate packet");
                }
                simulate_packet (p);
                enqueue (p);
                dequeue ();
            }
        }
    }
    
    private void simulate_packet (Packet packet)
    {
        int len = next_positive () % 64 + 50;
        ByteBuffer buf = packet.buf;
        buf.clear ();
        buf.putInt (IP_PROTOCOL);
        buf.putInt (next_positive () % IP_ADDR_COUNT);
        buf.putInt (next_positive () % IP_ADDR_COUNT);
        buf.putShort ((short) (next_positive () % PORT_COUNT));
        buf.putShort ((short) (next_positive () % PORT_COUNT));
        buf.putInt (next_positive () % TCP_MODULE == 0 ? TCP_PROTOCOL : UDP_PROTOCOL);
        buf.putInt (len);
// not filling with anything to save time        

        buf.flip ();
    }
    
    void enqueue (Packet p)
    {
        deque.add (p);
    }

    void dequeue ()
    {
        if (deque.size () >= INTERNAL_QUEUE_SIZE) {
            Packet p = deque.remove ();
            if (! process (p)) {
                manager.free (p);
            }
        }
    }
    
    boolean process (Packet p)
    {
        ByteBuffer buf = p.buf;
        if (buf.getInt () != IP_PROTOCOL) return false;
        
        IPHeader header = new IPHeader (buf);
        IPPacket ip = new IPPacket (header, buf.asReadOnlyBuffer ());
        p.ip_packet = ip;
        if (ip.header.protocol == TCP_PROTOCOL) {
            int ind = next_positive () % STORED_COUNT;
            Packet old = parsed [ind];
            if (old != null) {
                manager.free (old);
            }
            parsed [ind] = p;
        } else {
            manager.free (p);
        }
        return true;
    }
    
    private void batch_test ()
    {
        int N = REPORT_INTERVAL * 10;
        long total_count = - N * 5;
        long t0 = 0;
        while (true) {
            long t1 = System.currentTimeMillis ();
            for (int i = 0; i < N; i++) {
                Packet p = manager.get ();
                if (p == null) {
                    throw new RuntimeException ("Failure to allocate packet");
                }
                simulate_packet (p);
                enqueue (p);
                dequeue ();
            }
            total_count += N;
            if (total_count == 0) t0 = System.currentTimeMillis ();
            long t2 = System.currentTimeMillis ();
            long t = t2 - t1;
            System.out.print ("Time for " + N + ": " + t + "; packets/sec: " + N / 1000.0 / t + "M; ns/packet: " + t * 1000000.0 / N);
            if (total_count > 0) {
                System.out.print ("; avg: " + (t2-t0) * 1000000.0 / total_count);
            }
            System.out.println ();
        }
    }
    
    public static void main (String [] args) throws Exception
    {
        if (args.length < 3) {
            System.out.println ("Usage: Main [case label] [strategy] [batch/interval]");
            return;
        }
        
        PacketManager manager = null;
        switch (args [0].toLowerCase ()) {
        case "a": 
            INTERNAL_QUEUE_SIZE = A_INTERNAL_QUEUE_SIZE;
            STORED_COUNT = A_STORED_COUNT;
            break;

        case "b": 
            INTERNAL_QUEUE_SIZE = B_INTERNAL_QUEUE_SIZE;
            STORED_COUNT = B_STORED_COUNT;
            break;

        case "c": 
            INTERNAL_QUEUE_SIZE = C_INTERNAL_QUEUE_SIZE;
            STORED_COUNT = C_STORED_COUNT;
            break;
        default:
            System.out.println ("Case (A/B/C) not defined");
            return;
        }
        
        switch (args [1]) {
        case "alloc": manager = new AllocatingPacketManager (); break;
        case "pool" : manager = new PoolingPacketManager (POOL_SIZE); break;
        case "mix" : manager = new PoolingPacketManager (MIX_POOL_SIZE); break;
        case "dummy": manager = new DummyPacketManager (); break;
        case "native-alloc": manager = new NativeAllocPacketManager (POOL_SIZE); break;
        case "native-pool": manager = new NativePoolingPacketManager (POOL_SIZE, POOL_SIZE); break;
        case "native-mix": manager = new NativePoolingPacketManager (MIX_POOL_SIZE, POOL_SIZE); break;
        case "native-dummy": manager = new NativeDummyPacketManager (); break;
        default:
            System.out.println ("Unknown packet manager " + args [0]);
            return;
        }
        
        if (args[2].equals ("batch")) {
            new Main (manager, null).batch_test ();
            return;
        }
        
        int source_interval_ns = Integer.parseInt (args[2]);
        int queue_size = MAX_CAPTURING_DELAY_MS * 1000000 / source_interval_ns;
        IntQueue in_queue  = new ByteBufferAsyncIntQueue (queue_size);

        System.out.println ("Test: " + manager);
        System.out.println ("Input queue size: " + queue_size);
        System.out.println ("Input queue capacity, ms: " + 1.0E-6 * queue_size * source_interval_ns);

        System.out.println ("Internal queue: " + INTERNAL_QUEUE_SIZE + " = " + 1.0 * INTERNAL_QUEUE_SIZE * source_interval_ns / 1000000 + " ms");
        System.out.println ("Stored: " + STORED_COUNT);
        new Main (manager, in_queue).run (source_interval_ns);
    }
}
