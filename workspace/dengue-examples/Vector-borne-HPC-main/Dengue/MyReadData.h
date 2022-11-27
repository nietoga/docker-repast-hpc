#include <iostream> 
#include <fstream>
#include <string> 
#include <vector>
using namespace std; 

class ReadData{
    public:
        //first method
        std::vector<std::vector<int>> loadFromExcel(std::string fileName,int simulationTime);
};