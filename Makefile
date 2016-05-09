CXX =g++ -std=gnu++11

test:test.o ini.o Utf8_16.o
	$(CXX) $^ -o $@

%.o:%.cpp
	$(CXX) -c $^ -o $@

.PHONY:clean
clean:
	rm -f test *.o
