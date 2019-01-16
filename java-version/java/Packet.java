import java.nio.ByteBuffer;

public class Packet
{
    public static final int SIZE = 1024;
    
    ByteBuffer buf;
    IPPacket ip_packet = null;
    boolean in_use = false;
    
    Packet (ByteBuffer buf)
    {
        this.buf = buf;
    }
    
    
    public final void free ()
    {
        ip_packet = null;
        in_use = false;
    }
}
