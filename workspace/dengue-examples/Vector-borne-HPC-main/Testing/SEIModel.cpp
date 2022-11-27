#include "SEIModel.h"
#include <random>
#include <iostream>
using namespace std;

//constructor
SEIModel:: SEIModel(double suceptibleM, double exposedM, double infectedM, double temp){
    suceptibleMosquitoes=suceptibleM;
    exposedMosquitoes=exposedM;
    infectedMosquitoes=infectedM;
    temperature=temp;
}


//getters
double SEIModel::getSuceptibleMosquitoes(){
    return suceptibleMosquitoes;
}

double SEIModel::getExposedMosquitoes(){
    return exposedMosquitoes;
}

double SEIModel::getInfectedMosquitoes(){
    return infectedMosquitoes;
}

double SEIModel::getTemp(){
    return temperature;
}

//setters(a setter for type is not required because patched dont change type)
void SEIModel::setSuceptibleMosquitoes(double suceptibleM){
    suceptibleMosquitoes=suceptibleM;
}

void SEIModel::setExposedMosquitoes(double exposedM){
    exposedMosquitoes=exposedM;
}

void SEIModel::setInfectedMosquitoes(double infectedM){
    infectedMosquitoes=infectedM;
}

void SEIModel::setTemp(double temp){
    temperature=temp;
}

//most impotant methods methods
void SEIModel::recalculateSEI (double h){
    //get initial SEI
    double s0=suceptibleMosquitoes;
    double e0=exposedMosquitoes;
    double i0=infectedMosquitoes;
    
        
    //calculate birthRate and infectionRate
    double birthRate=calculateBirthRate();
	double infectionRate=calculateInfectionRate();

    double niter=1/h;
    int iter=1;

    //cout <<"i:"<<infectedMosquitoes<<"\n";
    //cout <<"e:"<<exposedMosquitoes<<"\n";
    //cout <<"s:"<<suceptibleMosquitoes<<"\n";
    //cout <<"birthRate:"<<birthRate<<"\n";
    //cout <<"infectionRate:"<<infectionRate<<"\n";
    while (iter<=niter) {
        //cout <<iter<<"\n";
        double k1i=h*infected_function(s0,e0,i0);
        double k1e=h*exposed_function(s0,e0,i0,infectionRate);
        double k1s=h*suceptible_function(s0,e0,i0,birthRate,infectionRate);
        
        
        
        double k2i=h*infected_function(s0+k1s/2,e0+k1e/2,i0+k1i/2);
        double k2e=h*exposed_function(s0+k1s/2,e0+k1e/2,i0+k1i/2,infectionRate);
        double k2s=h*suceptible_function(s0+k1s/2,e0+k1e/2,i0+k1i/2,birthRate,infectionRate);
    
        
        double k3i=h*infected_function(s0+k2s/2,e0+k2e/2,i0+k2i/2);
        double k3e=h*exposed_function(s0+k2s/2,e0+k2e/2,i0+k2i/2,infectionRate);
        double k3s=h*suceptible_function(s0+k2s/2,e0+k2e/2,i0+k2i/2,birthRate,infectionRate);
        
        double k4i=h*infected_function(s0+k3s,e0+k3e,i0+k3i);
        double k4e=h*exposed_function(s0+k3s,e0+k3e,i0+k3i,infectionRate);
        double k4s=h*suceptible_function(s0+k3s,e0+k3e,i0+k3i,birthRate,infectionRate);
    
        
        i0=i0+(k1i+2*k2i+2*k3i+k4i)/6;
        infectedMosquitoes=i0;
        //cout <<"i:"<<infectedMosquitoes<<"\n";
        
        e0=e0+(k1e+2*k2e+2*k3e+k4e)/6;
        exposedMosquitoes=e0;
        //cout <<"e:"<<exposedMosquitoes<<"\n";
        
        s0=s0+(k1s+2*k2s+2*k3s+k4s)/6;
        suceptibleMosquitoes=s0;
        //cout <<"s:"<<suceptibleMosquitoes<<"\n";
        
        //recalculate birthRate and infectionRate
        birthRate=calculateBirthRate();
        //cout <<"birthRate:"<<birthRate<<"\n";
        infectionRate=calculateInfectionRate();
        //cout <<"infectionRate:"<<infectionRate<<"\n";
        iter=iter+1;
    }
}

void SEIModel::updateTemp(){ ////TEMPORAL: se deben leer los datos del file con las temperaturas diaras
    int newTemperature= 20;
    temperature=newTemperature;
}

//auxiliaryMethods
double SEIModel::suceptible_function(double suceptible,double exposed,double infected,double birthRate, double infectionRate) {
    double s1=birthRate-infectionRate*suceptible-deathRate*suceptible;
	return s1;
}
	
double SEIModel::exposed_function(double suceptible,double exposed,double infected,double infectionRate) {
	double exposedToinfectedRate=calculateExposedToinfectedRate();
	double e1=infectionRate*suceptible-exposedToinfectedRate*exposed-deathRate*exposed;
	return e1;
}
	
double SEIModel::infected_function(double suceptible,double exposed,double infected) {
    double exposedToinfectedRate=calculateExposedToinfectedRate();
    double i1=exposedToinfectedRate*exposed-deathRate*infected;
    return i1;
}

double SEIModel::calculateBirthRate() {
    double totalMosquitoes=suceptibleMosquitoes+infectedMosquitoes+exposedMosquitoes;
    double mosquitoPopulationGrowthRate=naturalEmergenceRate-deathRate;
    double birthRate=totalMosquitoes*(naturalEmergenceRate-mosquitoPopulationGrowthRate*totalMosquitoes/mosquitoCarryingCapacity);
    return birthRate;
}
   
double SEIModel::calculateInfectionRate() { //depende del numero de humanos en el patch   
    int totalHumans=calculateTotalHumansInPatch();
    int humansInfected=calculateInfectedHumansInPatch();
    double totalMosquitoes=suceptibleMosquitoes+infectedMosquitoes+exposedMosquitoes;
    
    double totalSuccesfulBites=(mosquitoBiteDemand*totalMosquitoes*maxBitesPerHuman*totalHumans)/(mosquitoBiteDemand*totalMosquitoes+maxBitesPerHuman*totalHumans);
    double successfulBitesPerMosquito=totalSuccesfulBites/totalMosquitoes;
    double infectionRateMosquitoes=0;
    if (totalHumans>0) {
        infectionRateMosquitoes=successfulBitesPerMosquito*probabilityOfTransmissionHToM*((humansInfected+0.0)/totalHumans);
    }
    return infectionRateMosquitoes;
}

double SEIModel::calculateExposedToinfectedRate() {
    double exposedToInfectedRate;
    if (temperature<15) {
        exposedToInfectedRate=0;
    }
    else {
        double patchIncubationPeriod = 0;
        if (15<temperature && temperature<21) {
            std::uniform_real_distribution<double> myUnifDist(10,25);//generate uniform distribution
            std::default_random_engine re;//generate random number generator
            re.seed(std::random_device{}()); //generate seed of random number generator 
            patchIncubationPeriod=myUnifDist(re);
        } else if (21<=temperature && temperature<26) {
            std::uniform_real_distribution<double> myUnifDist(7,10);//generate uniform distribution
            std::default_random_engine re;//generate random number generator
            re.seed(std::random_device{}()); //generate seed of random number generator 
            patchIncubationPeriod=myUnifDist(re);
        } else if (26<=temperature && temperature<31) {
            std::uniform_real_distribution<double> myUnifDist(4,7);//generate uniform distribution
            std::default_random_engine re;//generate random number generator
            re.seed(std::random_device{}()); //generate seed of random number generator 
            patchIncubationPeriod=myUnifDist(re);
        }
        patchIncubationPeriod=10;
        exposedToInfectedRate=1/patchIncubationPeriod;
    }
    return exposedToInfectedRate;
}

int SEIModel::calculateTotalHumansInPatch() {	// TEMPORAL->se deben contar la cantidad de humanos en el patch
    return 10;
}

int SEIModel::calculateInfectedHumansInPatch() { // TEMPORAL->se deben contar la cantidad de humanos infectados en el patch
    return 5;
}	
