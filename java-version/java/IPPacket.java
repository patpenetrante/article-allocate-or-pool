import java.nio.ByteBuffer;

public class IPPacket
{
    IPHeader header;
    ByteBuffer buf;
    
    public IPPacket (IPHeader header, ByteBuffer buf)
    {
        this.header = header;
        this.buf = buf;
    }
}
