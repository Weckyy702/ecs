ASAN = -fsanitize=address
DBG = -ggdb -Og

CXXFLAGS += -std=c++23 -Wall -Wextra -Wconversion -Werror $(DBG) $(ASAN)
LDFLAGS += -lfmt -lraylib $(ASAN)
