include ./env

all: clean build

clean:
	rm -f *.exe
	rm -f *.o

build: clean main.exe

main.exe: main.o model.o
	$(MPICXX) $(BOOST_LIB_DIR) $(REPAST_HPC_LIB_DIR) $(GEOS_LIB_DIR) -o main.exe main.o model.o $(REPAST_HPC_LIB) $(BOOST_LIBS) $(GEOS_LIB)

main.o:
	$(MPICXX) $(BOOST_INCLUDE) $(REPAST_HPC_INCLUDE) $(GEOS_INCLUDE) -c main.cpp -o main.o

model.o:
	$(MPICXX) $(BOOST_INCLUDE) $(REPAST_HPC_INCLUDE) $(GEOS_INCLUDE) -c model.cpp -o model.o
