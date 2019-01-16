#example; set it to your Java directory if not set already
#JAVA_HOME=/usr/java/8/x86_64/jdk

    c++ -m64 -std=c++11  -Wall  -fPIC -O3 -shared -o libNativeDataSource.so \
	-I${JAVA_HOME}/include{,/linux} \
	NativeDataSource.cpp \
	-lpthread

