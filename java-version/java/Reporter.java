
public class Reporter extends Thread
{
    private final IntQueue report_queue;
    
    public Reporter (IntQueue report_queue)
    {
        super ("Reporter");
        this.report_queue = report_queue;
    }

    @Override
    public void run ()
    {
        long start = System.nanoTime ();
        long current_start = start;
        int count = 0;
        long total_lost = 0;
        long total_sent = 0;
        long old_mem = 0;

        while (true) {
            int lost = report_queue.read();
            long now = System.nanoTime ();
            
            long since_start = now - start;
            long since_last = now - current_start;

            System.out.printf("%5.1f; %5.1f; lost: %8d"
                , since_start * 1.0E-9
                , since_last * 1.0E-9
                , lost
            );
            ++ count;
            if (count > 10) {
                total_lost += lost;
                total_sent += Main.REPORT_INTERVAL + lost;
                double lost_frac = total_lost * 1.0 / total_sent;
                long mem = Runtime.getRuntime ().totalMemory () - Runtime.getRuntime ().freeMemory ();
                System.out.printf (" frac = %7.4f%% or %f; used=%10d", lost_frac * 100.0, lost_frac, mem);
                if (mem < old_mem) {
                    System.out.printf ("; %11d", mem);
                }
                old_mem = mem;
            }
            System.out.println ();
            current_start = now;
        }
    }

}
