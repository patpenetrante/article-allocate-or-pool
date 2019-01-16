import java.nio.ByteBuffer;

public class NativeDummyPacketManager extends PacketManager
{
    final Packet packet = new Packet (ByteBuffer.allocateDirect (Packet.SIZE));
    
    public NativeDummyPacketManager ()
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
        return "NATIVE-DUMMY";
    }
}
