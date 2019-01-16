java -Djava.library.path=. -Xloggc:gclog -Xms1g -Xmx1g -XX:MaxDirectMemorySize=2g  -server Main $*
