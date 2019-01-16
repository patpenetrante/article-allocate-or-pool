import java.nio.ByteBuffer;
import java.util.ArrayList;

public final class PoolingPacketManager extends PacketManager
{
    final ArrayList<Packet> pool;
    final int size;
    
    public PoolingPacketManager (int size)
    {
        this.size = size;
        pool = new ArrayList<Packet> (size);
        for (int i = 0; i < size; i++) {
            pool.add (alloc ());
        }
    }

    private Packet alloc ()
    {
        return new Packet (ByteBuffer.wrap (new byte [Packet.SIZE]));
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
        }
    }

    @Override
    public String toString ()
    {
        return "POOL, size = " + size;
    }
}
