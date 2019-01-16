import java.nio.ByteBuffer;

public final class AllocatingPacketManager extends PacketManager
{
    @Override
    public Packet get ()
    {
        return new Packet (ByteBuffer.wrap (new byte [Packet.SIZE]));
    }
    
    @Override
    public void free (Packet packet)
    {
    }
    
    @Override
    public String toString ()
    {
        return "ALLOC";
    }
}
