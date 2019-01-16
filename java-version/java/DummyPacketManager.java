import java.nio.ByteBuffer;

public final class DummyPacketManager extends PacketManager
{
    final Packet packet = new Packet (ByteBuffer.wrap (new byte [Packet.SIZE]));
    
    public DummyPacketManager ()
    {
    }

    @Override
    public Packet get ()
    {
        return packet;
    }

    @Override
    public void free (Packet packet)
    {
    }

    @Override
    public String toString ()
    {
        return "DUMMY";
    }
}
