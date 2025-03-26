TARGET?=carray_test
CXXFLAGS=-g -std=c++23 -Wall -O0 -I./inc
LDFLAGS?=
LDLIBS?=

$(TARGET).o:test/$(TARGET).cc
	$(CXX) $(CXXFLAGS) -c -o $@ $^

$(TARGET):$(TARGET).o
	${CXX} -o $(TARGET) $^ ${LDFLAGS} ${LDLIBS}

leak-check:$(TARGET)
	TARGET=carray_test valgrind --leak-check=full ./carray_test 

clean:
	$(RM) *.o

cleanall: clean
	$(RM) $(TARGET)

.DEFAULT_GOAL=$(TARGET)