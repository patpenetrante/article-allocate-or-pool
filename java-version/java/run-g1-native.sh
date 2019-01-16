java -Djava.library.path=. -Xloggc:gclog -Xms1g -Xmx1g -XX:MaxDirectMemorySize=2g -XX:+UseG1GC -XX:MaxGCPauseMillis=80 -server Main $*

