OBJS = chatnet.o common.o main.o
chatnets: $(OBJS)
	g++ -o chatnets $(OBJS) -lpthread
main.o: main.cpp chatnet.h
	g++ -c main.cpp
chatnet.o: chatnet.cpp chatnet.h global.h
	g++ chatnet.cpp -c -lpthread -lsocket

common.o: common.cpp global.h
	g++ common.cpp -c

.PHONY : clean
clean:
	rm $(OBJS)

.PHONY : install

install:
	cp chatnets /usr/bin

.PHONY : uninstall

uninstall:
	rm /usr/bin/chatnets 
