CXX := g++
CXXFLAGS := -std=c++11 -g -Wall -I.

all: test_demosaic


test_demosaic: raw_bayer_demosaic.o tests/test_demosaic.o
	$(CXX) $(CXXFLAGS) $^ -o $@


%.o:%.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

tests/%.o:tests/%.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

clean:
	$(RM) *.o
	$(RM) tests/*.o
	$(RM) test_demosaic
