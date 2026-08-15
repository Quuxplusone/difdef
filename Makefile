CXX = g++
CXXFLAGS = -Ilibsrc -O2 -g -W -Wall -Wextra -pedantic

all: difdef

difdef: main.o ifdefs.o recurse.o unified.o verify.o difdef_impl.o getline.o
	$(CXX) $(CXXFLAGS) $^ -o $@

difdef_impl.o: libsrc/difdef_impl.cc libsrc/patience.cc libsrc/classical.cc
	$(CXX) $(CXXFLAGS) -c libsrc/difdef_impl.cc -o $@

getline.o: libsrc/getline.cc
	$(CXX) $(CXXFLAGS) -c $^ -o $@

%.o: src/%.cc
	$(CXX) $(CXXFLAGS) -c $^ -o $@

clean:
	rm -f *.o difdef
