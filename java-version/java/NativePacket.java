import java.nio.ByteBuffer;

public class NativePacket extends Packet
{
    int num;
    
    NativePacket (int num, ByteBuffer buf)
    {
        super (buf);
        this.num = num;
    }

}
