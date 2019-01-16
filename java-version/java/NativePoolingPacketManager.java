import java.util.ArrayList;

public final class NativePoolingPacketManager extends PacketManager
{
    final NativePool native_pool;
    final ArrayList<Packet> pool;
    final int size;
    
    public NativePoolingPacketManager (int size, int native_size)
    {
        native_pool = new NativePool (native_size);
        
        this.size = size;
        pool = new ArrayList<Packet> (size);
        for (int i = 0; i < size; i++) {
            pool.add (alloc ());
        }
    }

    private Packet alloc ()
    {
        int num = native_pool.get ();
        return new NativePacket (num, native_pool.buffer (num));
    }
    
    @Override
    public Packet get ()
    {
        return pool.size () == 0 ? alloc () : pool.remove (pool.size () - 1);
    }

    @Override
    public void free (Packet packet)
    {
        if (pool.size () < size) {
            packet.free ();
            pool.add (packet);
        } else {
            native_pool.free (((NativePacket) packet).num);
        }
    }

    @Override
    public String toString ()
    {
        return "NATIVE POOL, size = " + size;
    }
}
