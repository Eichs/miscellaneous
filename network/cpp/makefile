CXX = g++
CXXFLAGS = -std=c++11 -I$(CURDIR)/proto -I$(CURDIR)
LDFLAGS = -lpthread -lprotobuf -static

SOURCES = main.cpp \
packet/packet.cpp

PROTO_SOURCES = $(wildcard $(CURDIR)/proto/server_only/*.cc) \
                $(wildcard $(CURDIR)/proto/cmd/*.cc) \
                $(wildcard $(CURDIR)/proto/pb/*.cc) \
                $(wildcard $(CURDIR)/proto/*.cc)

OBJECTS = $(SOURCES:.cpp=.o) $(PROTO_SOURCES:.cc=.o)

TARGET = main

CORES = $(shell nproc)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@

.cc.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean-all:
	rm -f $(OBJECTS) $(TARGET)

clean:
	rm -f $(SOURCES:.cpp=.o)
