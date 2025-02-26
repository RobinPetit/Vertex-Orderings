FLAGS=-Wall -Wextra -std=c++20 -MMD -O3
# FLAGS=-Wall -Wextra -std=c++20 -MMD -g -fsanitize=address
LDFLAGS=-lnautypp -static-libsan

all: bin/orderings bin/gadget

bin/%: obj/%.o
	${CXX} -o $@ $< ${FLAGS} ${LDFLAGS}

obj/%.o: %.cpp
	${CXX} -o $@ -c $< ${FLAGS}

-include obj/*.d

clear:
	rm -f obj/*d obj/*.o bin/*

.PHONY: clear
