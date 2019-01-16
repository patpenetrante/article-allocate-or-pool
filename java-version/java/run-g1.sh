java -Djava.library.path=. -Xloggc:gclog -Xms2g -Xmx2g -XX:+UseG1GC -XX:MaxGCPauseMillis=80 -server Main $*

