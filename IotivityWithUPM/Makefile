
IOTIVITY = /home/sasmita/Desktop/iotivity

INC = -I $(IOTIVITY)/resource/include \
      -I $(IOTIVITY)/resource/csdk/stack/include \
      -I $(IOTIVITY)/resource/c_common \
      -I $(IOTIVITY)/resource/oc_logger/include \

LIBRARIES = -L$(IOTIVITY)/out/linux/x86_64/release \
	-loc -loctbstack -loc_logger \
        -lpthread \
	$(IOTIVITY)/out/linux/x86_64/release/libcoap.a \
	$(IOTIVITY)/out/linux/x86_64/release/liblogger.a 

FLAGS = -std=c++11

all: temperatureserver temperatureclient ledserver ledclient

temperatureclient.o: temperatureclient.cpp
	g++ -c $(FLAGS) temperatureclient.cpp $(INC)
temperatureclient: temperatureclient.o
	g++ -o temperatureclient $(FLAGS) temperatureclient.o $(LIBRARIES)

temperatureserver.o: temperatureserver.cpp
	g++ -c $(FLAGS) temperatureserver.cpp $(INC)
temperatureserver: temperatureserver.o
	g++ -o temperatureserver $(FLAGS) temperatureserver.o $(LIBRARIES)

ledclient.o: ledclient.cpp
	g++ -c $(FLAGS) ledclient.cpp $(INC)
ledclient: ledclient.o
	g++ -o ledclient $(FLAGS) ledclient.o $(LIBRARIES)

ledserver.o: ledserver.cpp
	g++ -c $(FLAGS) ledserver.cpp $(INC)
ledserver: ledserver.o
	g++ -o ledserver $(FLAGS) ledserver.o $(LIBRARIES)

clean:
	rm ledserver
	rm ledclient
	rm temperatureserver
	rm temperatureclient
	rm *.o
