../jdk-11/bin/java -Djava.library.path=. -Xloggc:gclog  -Xms2g -Xmx2g -XX:+UnlockExperimentalVMOptions -XX:+UseZGC  -server Main $*
