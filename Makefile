LINK.o = $(LINK.cc)

CXXFLAGS += -std=c++23 -Wall -Wextra -Wconversion -Wpedantic -Werror -ggdb -Og
CPPFLAGS += -MMD -MP -Iinclude
LDLIBS += -lfmt -lraylib 

.PHONY: clean cleanall

main:

pong: pong.o socket.o

clean:
	$(RM) main pong *.o

cleanall: clean
	$(RM) *.d

-include $(wildcard *.d)
