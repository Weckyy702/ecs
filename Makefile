LINK.o = $(LINK.cc)

CXXFLAGS += -std=c++23 -Wall -Wextra -Wconversion -Wpedantic -Werror -ggdb -Og
CPPFLAGS += -MMD -MP -Iinclude
LDLIBS += -lfmt -lraylib 

.PHONY: clean cleanall

pong: pong.o src/socket.o

events: events.o src/EventManager.o src/EventClient.o

clean:
	$(RM) main events pong *.o src/*.o

cleanall: clean
	$(RM) *.d src/*.d

-include $(shell find -type f -name '*.d')
