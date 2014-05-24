CXX=clang++
CXXOPTS=-I. -pthread --std=c++11 -Weverything -Wno-c++98-compat -g3 -O2
OBJ=main.o
PRG=genetic
CPUS?=0

all: $(PRG)

run: clean all
	./$(PRG)

$(PRG): $(OBJ)
	$(CXX) -o $@ $^ $(CXXOPTS)

clean: mostlyclean
	find . -name $(PRG) -delete

mostlyclean:
	find . -name '*.o' -delete

%.o: %.cc
	$(CXX) -c -o $@ $< $(CXXOPTS) -DCPUS=$(CPUS)
