CXX = g++
CFLAGS = -O3 -W -Wall -Wextra -pedantic

all: diffn

diffn: diffn_main.o difdef_impl.o
	$(CXX) $(CFLAGS) $^ -o $@

difdef_impl.o: difdef_impl.cc patience.cc
	$(CXX) $(CFLAGS) -c difdef_impl.cc -o $@

diffn_main.o: diffn_main.cc
	$(CXX) $(CFLAGS) -c $^ -o $@

clean:
	rm -f *.o diffn
