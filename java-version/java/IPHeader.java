import java.nio.ByteBuffer;

public final class IPHeader
{
    final IPAddress src_addr;
    final IPAddress dst_addr;
    final int src_port;
    final int dst_port;
    final int protocol;

    public IPHeader (ByteBuffer buf)
    {
        this.src_addr = new IP4Address (buf.getInt ());
        this.dst_addr = new IP4Address (buf.getInt ());
        this.src_port = buf.getShort () & 0xFFFF;
        this.dst_port = buf.getShort () & 0xFFFF;
        this.protocol = buf.getInt ();
    }
}
