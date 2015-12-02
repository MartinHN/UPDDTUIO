

LIBS=-L/Library/Application\ Support/UPDD/api -ltbapi -L/usr/local/lib -lace
HEADERS=-I/Library/Application\ Support/UPDD/api -IofxOsc/libs/oscpack/src 

all:
	g++ main.cpp TUIO/*.cpp ofxOsc/libs/oscpack/src/ip/*.cpp ofxOsc/libs/oscpack/src/osc/*.cpp ofxOsc/libs/oscpack/src/ip/posix/*.cpp $(HEADERS) $(LIBS)  -DMACX -framework Carbon  -stdlib=libstdc++ -o build/main


clean:
	rm build/main