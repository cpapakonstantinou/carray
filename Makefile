TARGET ?= carray_test
CXXFLAGS = -g -std=c++23 -Wall -O3 -I./inc -I /usr/include/eigen3 
LDFLAGS ?=
LDLIBS ?= 
HEADERS = inc/carray.h inc/aligned_memory.h
PREFIX ?= /usr
INSTALLDIR ?= $(PREFIX)/include/carray

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

install: inc/carray.h inc/aligned_memory.h
	install -d $(INSTALLDIR)
	install -m 644 $(HEADERS) $(INSTALLDIR)

# Uninstall rule
uninstall:
	$(RM) -r $(INSTALLDIR)


.DEFAULT_GOAL=$(TARGET)