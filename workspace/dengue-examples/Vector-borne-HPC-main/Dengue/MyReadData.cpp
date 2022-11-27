#ifndef MyReadData
#define MyReadData
#include <iostream> 
#include <fstream>
#include <string> 
#include <sstream>
#include <vector>
#include "MyReadData.h"
#include <boost/algorithm/string.hpp>
using namespace std; 

std::vector<std::vector<int>> ReadData::loadFromExcel(std::string fileName,int simulationTime){
    //the main array where all the data will be stored
    std::vector<std::vector<int>> allData;
    //the auxiliary arrays where the max and min temperatures are going to be stored in
    std::vector<int> list_tempMax;std::vector<int> list_tempMin;
    
    //name of the file where the data is stored in
    ifstream myFile;
    myFile.open(fileName);
    if (!myFile) { //if the file couldnt be opened
        cout<<"Couldn't find file \n";
        list_tempMax.push_back(0);list_tempMin.push_back(0);
        allData.push_back(list_tempMax);allData.push_back(list_tempMin);
    }
    else {
        std::string line;
        myFile>>line; //ignore the first line
        int cont=0;
        while(cont<simulationTime){//for every line do the following 
            if(myFile.eof()){
                break;
            }
            //take the line and parse it to save it to an array
            myFile>>line;
            std::vector<std::string> lineArray;
            boost::split(lineArray,line,boost::is_any_of(";"));
            //iterate through the array and save the element in column 2 and 3 in two separate arrays
            for(int i=0;i<lineArray.size();i++){
                if(i==2){
                    int tempMax;
                    stringstream ss(lineArray[i]); //created an object from the class stringstream
                    ss>>tempMax; //ss has the value of the maximum temperature and then we stream it to the variable tempMax
                    list_tempMax.push_back(tempMax); //the value of tempMax is pushed in to the arrays
                }
                if(i==3){
                    int tempMin;
                    stringstream ss(lineArray[i]); //created an object from the class stringstream
                    ss>>tempMin; //ss has the value of the minimum temperature and then we stream it to the variable tempMin
                    list_tempMin.push_back(tempMin); //the value of tempMin is pushed in to the arrays
                }
                
            } 
            cont++;
        }
        myFile.close();
        allData.push_back(list_tempMax);
        allData.push_back(list_tempMin);
    }
    return allData;
}
    

#endif