include ./env

.PHONY: all
all: clean_output_files clean_compiled_files ./bin/Demo_03.exe

.PHONY: clean_output_files
clean_output_files:
	rm -f ./output/*.csv
	rm -f ./output/*.txt

.PHONY: clean_compiled_files
clean_compiled_files:
	rm -f ./bin/*.exe
	rm -f ./object/*.o

./bin/Demo_03.exe: ./objects/Demo_03_Main.o ./objects/Demo_03_Model.o ./objects/Demo_03_Agent.o
	$(MPICXX) $(BOOST_LIB_DIR) $(REPAST_HPC_LIB_DIR) -o ./bin/Demo_03.exe  ./objects/Demo_03_Main.o ./objects/Demo_03_Model.o ./objects/Demo_03_Agent.o $(REPAST_HPC_LIB) $(BOOST_LIBS)

./objects/Demo_03_Main.o: ./src/Demo_03_Main.cpp ./src/Demo_03_Model.h
	$(MPICXX) $(REPAST_HPC_DEFINES) $(BOOST_INCLUDE) $(REPAST_HPC_INCLUDE) -I./include -c ./src/Demo_03_Main.cpp -o ./objects/Demo_03_Main.o

./objects/Demo_03_Model.o: ./src/Demo_03_Model.cpp ./src/Demo_03_Model.h
	$(MPICXX) $(REPAST_HPC_DEFINES) $(BOOST_INCLUDE) $(REPAST_HPC_INCLUDE) -I./include -c ./src/Demo_03_Model.cpp -o ./objects/Demo_03_Model.o

./objects/Demo_03_Agent.o: ./src/Demo_03_Agent.cpp ./include/Demo_03_Agent.h
	$(MPICXX) $(REPAST_HPC_DEFINES) $(BOOST_INCLUDE) $(REPAST_HPC_INCLUDE) -I./include -c ./src/Demo_03_Agent.cpp -o ./objects/Demo_03_Agent.o
