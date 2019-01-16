public class NativeAllocPacketManager extends PacketManager
{
    private final NativePool pool;
    
    NativeAllocPacketManager (int size)
    {
        pool = new NativePool (size);
    }
    
    @Override
    public Packet get ()
    {
        int num = pool.get ();
        return new NativePacket (num, pool.buffer (num));
    }
    
    @Override
    public void free (Packet packet)
    {
        pool.free (((NativePacket) packet).num);
        packet.buf = null;
    }
    
    @Override
    public String toString ()
    {
        return "NATIVE-ALLOC";
    }
}
