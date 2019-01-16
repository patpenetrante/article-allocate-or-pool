
public final class IP4Address extends IPAddress
{
    final int addr;
    
    public IP4Address (int addr)
    {
        this.addr = addr;
    }

    @Override
    public int hashCode ()
    {
        return addr;
    }

    @Override
    public boolean equals (Object obj)
    {
        return obj != null && obj instanceof IP4Address && ((IP4Address) obj).addr == addr;
    }
}
