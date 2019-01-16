import java.nio.ByteBuffer;

public class NativePool
{
    final ByteBuffer heap;
    final int[] pool;
    int available = 0;
    
    public NativePool (int size)
    {
        heap = ByteBuffer.allocateDirect (size * Packet.SIZE);
        pool = new int [size];
        for (int i = 0; i < size; i++) {
            free (i);
        }
    }

    public int get ()
    {
        if (available == 0) throw new RuntimeException ("Out of pooled native buffers");
        return pool [--available];
    }
    
    public ByteBuffer buffer (int num)
    {
        heap.limit ((num + 1) * Packet.SIZE);
        heap.position (num * Packet.SIZE);
        return heap.slice ();
    }

    public void free (int num)
    {
        pool [available++] = num;
    }
}
