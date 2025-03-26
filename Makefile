TARGET?=carray_test
CXXFLAGS=-g -std=c++23 -Wall -O0 -I./inc -I /usr/include/eigen3 -march=native
LDFLAGS?=
LDLIBS?= 

$(TARGET).o:test/$(TARGET).cc
	$(CXX) $(CXXFLAGS) -c -o $@ $^

$(TARGET):$(TARGET).o
	${CXX} -o $(TARGET) $^ ${LDFLAGS} ${LDLIBS}

leak-check:$(TARGET)
	valgrind --leak-check=full ./carray_test 

benchmark.o:test/benchmark.cc
	$(CXX) $(CXXFLAGS) -c -o $@ $^

benchmark:benchmark.o
	${CXX} -o benchmark $^ ${LDFLAGS} ${LDLIBS}
	@(ulimit -s 4000000 && ./benchmark) #set stack size and run

clean:
	$(RM) *.o

cleanall: clean
	$(RM) $(TARGET) benchmark

.DEFAULT_GOAL=$(TARGET)