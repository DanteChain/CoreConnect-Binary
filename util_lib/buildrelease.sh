set -x

#cd event_engine2
#rm -f src/*.o
#g++ -O3 -Wall -Wno-deprecated -std=c++17 -I../ -I. -I./h -c src/ev_epoll.cpp -o src/ev_epoll.o
#g++ -O3 -Wall -Wno-deprecated -std=c++17 -I../ -I. -I./h -c src/ev_events.cpp -o src/ev_events.o
#g++ -O3 -Wall -Wno-deprecated -std=c++17 -I../ -I. -I./h -c src/ev_manager.cpp -o src/ev_manager.o
#cd -

cd file_config
rm -f src/*.o
g++ -O3 -Wall -Wno-deprecated -std=c++17 -I../ -I. -I./h -c src/file_config.cpp -o src/file_config.o
cd -

cd generic_framer
rm -f src/*.o
g++ -O3 -Wall -Wno-deprecated -std=c++17 -I../ -I. -I./h -c src/generic_framer.cpp -o src/generic_framer.o
g++ -O3 -Wall -Wno-deprecated -std=c++17 -I../ -I. -I./h -c src/generic_framer1.cpp -o src/generic_framer1.o
g++ -O3 -Wall -Wno-deprecated -std=c++17 -I../ -I. -I./h -c src/generic_framer2.cpp -o src/generic_framer2.o
cd -

cd latency
rm -f src/*.o
g++ -O3 -Wall -Wno-deprecated -std=c++17 -I../ -I. -I./h -c src/latency.cpp -o src/latency.o
cd -

cd logger
rm -f src/*.o
g++ -O3 -Wall -Wno-deprecated -std=c++17 -I../ -I. -I./h -c src/logger.cpp -o src/logger.o
cd -

cd tcp
rm -f src/*.o
g++ -O3 -Wall -Wno-deprecated -std=c++17 -I../ -I. -I./h -c src/tcp_server.cpp -o src/tcp_server.o
g++ -O3 -Wall -Wno-deprecated -std=c++17 -I../ -I. -I./h -c src/tcp_client.cpp -o src/tcp_client.o
g++ -O3 -Wall -Wno-deprecated -std=c++17 -I../ -I. -I./h -c src/tcp_connection.cpp -o src/tcp_connection.o
cd -

cd time
rm -f src/*.o
g++ -O3 -Wall -Wno-deprecated -std=c++17 -I../ -I. -I./h -c src/timestamp.cpp -o src/timestamp.o
g++ -O3 -Wall -Wno-deprecated -std=c++17 -I../ -I. -I./h -c src/datestamp.cpp -o src/datestamp.o
cd -

cd fields
rm -f src/*.o
g++ -O3 -Wall -Wno-deprecated -std=c++17 -I../ -I. -I./h -c src/fields.cpp -o src/fields.o
cd -

cd multicast
rm -f src/*.o
g++ -O3 -Wall -Wno-deprecated -std=c++17 -I../ -I. -I./h -c src/multicast.cpp -o src/multicast.o
cd -





cd moonx_protobuf
rm -f proto/*.h
rm -f proto/*.cc
rm -f proto/*.o
protoc --proto_path=./proto --cpp_out=./proto proto/MoonxESG.proto
protoc --proto_path=./proto --cpp_out=./proto proto/ExchangeMoonx.proto
protoc --proto_path=./proto --cpp_out=./proto proto/TypesCodes.proto
#g++  -Wall -std=c++17 -I. -c proto/ExchangeMoonx.pb.cc -o proto/ExchangeMoonx.pb.o
#g++  -Wall -std=c++17 -I. -c proto/MoonxESG.pb.cc -o proto/MoonxESG.pb.o
#g++  -Wall -std=c++17 -I. -c proto/TypesCodes.pb.cc -o proto/TypesCodes.pb.o
g++ -Wall -std=c++17 -I. -I../../3rdparty/google/protobuf/include -c proto/ExchangeMoonx.pb.cc -o proto/ExchangeMoonx.pb.o
g++ -Wall -std=c++17 -I. -I../../3rdparty/google/protobuf/include -c proto/MoonxESG.pb.cc -o proto/MoonxESG.pb.o
g++ -Wall -std=c++17 -I. -I../../3rdparty/google/protobuf/include -c proto/TypesCodes.pb.cc -o proto/TypesCodes.pb.o
cd -


rm -f lib/*
ar -r lib/libutil.a event_engine2/src/*.o
ar -r lib/libutil.a file_config/src/*.o
ar -r lib/libutil.a generic_framer/src/*.o
ar -r lib/libutil.a latency/src/*.o
ar -r lib/libutil.a logger/src/*.o
ar -r lib/libutil.a tcp/src/*.o
ar -r lib/libutil.a time/src/*.o
ar -r lib/libutil.a fields/src/*.o
ar -r lib/libutil.a multicast/src/*.o
ar -r lib/libutil.a moonx_protobuf/proto/*.o 
ar -t lib/libutil.a

