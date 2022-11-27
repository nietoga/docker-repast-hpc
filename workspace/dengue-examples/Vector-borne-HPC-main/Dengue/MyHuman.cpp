#include "MyHuman.h"
//constructor that takes arguments 
Human:: Human(repast::AgentId id, string InfectionState, int Age, int TimeSinceSuccesfullBite,int TimeSinceInfection,std::vector<int> HomeLocation,std::vector<std::vector<int>> Activities){
    id_=id;
    infectionState=InfectionState;
    age=Age;
    timeSinceSuccesfullBite=TimeSinceSuccesfullBite;
    timeSinceInfection=TimeSinceInfection;
    homeLocation=HomeLocation;
    activities=Activities;
}

//getters
string Human::getInfectionState(){
    return infectionState;
}
int Human::getAge(){
    return age;
}
int Human::getTimeSinceSuccesfullBite(){
    return timeSinceSuccesfullBite;
}
int Human::getTimeSinceInfection(){
    return timeSinceInfection;
}
std::vector<int> Human::getHomeLocation(){
    return homeLocation;
}
std::vector<std::vector<int>> Human::getActivities(){
    return activities;
}


//setters
void Human::setAll(std::string InfectionState,int Age,int TimeSinceSuccesfullBite,int TimeSinceInfection,std::vector<int> HomeLocation,std::vector<std::vector<int>> Activities){
    infectionState=InfectionState;
    age=Age;
    timeSinceSuccesfullBite=TimeSinceSuccesfullBite;
    timeSinceInfection=TimeSinceInfection;
    homeLocation=HomeLocation;
    activities=Activities;
}
void Human::setInfectionState(string InfectionState){
    infectionState=InfectionState;
}
void Human::setAge(int Age){
    age=Age;
}
void Human::setTimeSinceSuccesfullBite(int TimeSinceSuccesfullBite){
    timeSinceSuccesfullBite=TimeSinceSuccesfullBite;
}
void Human::setTimeSinceInfection(int TimeSinceInfection){
    timeSinceInfection=TimeSinceInfection;
}
void Human::setHomeLocation(std::vector<int> HomeLocation){
    homeLocation=HomeLocation;
}
void Human::setActivities(std::vector<std::vector<int>> Activities){
    activities=Activities;
}
//Print human info
void Human::printHumanInfo(){
    std::cout<<"id: "<<id_<<"age: "<<age<<", infecion state: "<<infectionState<<", timeSinceSuccesfullBite: "<<timeSinceSuccesfullBite<<", timeSinceInfection: "<<timeSinceInfection<<", homeLocation:"<<homeLocation[0]
	<<",  "<< homeLocation[1]<<", currentLoc:"<<"\n"; 
}

//ACTIONS
//metodo para actualizar los tiempos de infeccion y tiempos desde mordida 
void Human::actualizeTimes(){
    //solo se le suma 1 dia si la variable es mayor o igual a 0 (es no nula)
    if(timeSinceSuccesfullBite>=0){
        timeSinceSuccesfullBite=timeSinceSuccesfullBite+1;
    }
    //solo se le suma 1 dia si la variable es mayor o igual a 0 (es no nula)
    if(timeSinceInfection>=0){
        timeSinceInfection=timeSinceInfection+1;
    }
}

//Metodo para actualizar variable de estado SEIR
void Human::actualizeSEIRStatus(repast::SharedContext<Human>* context) {
    //si la persona esta en estado suceptible se calcula la probabilidad de pasar a infectado y se determina si pasa a infectado o no
    if(infectionState == "susceptible") {
        //hallar la probabilidad de infeccion de este patch en el que el humano esta parado        
        double probabilityOfInfectionHuman=calculateInfectionProbabilityHuman();
        if(repast::Random::instance()->nextDouble()<= probabilityOfInfectionHuman) {
            infectionState = "exposed";
            timeSinceSuccesfullBite = 0;    
        }   
    }

    //si la persona esta en estado expuesto se determina si pasa a infectado 
    if (infectionState == "exposed") {
        //Hallemos la siguiente probabilidad p(tiempoDeIncubacion<=tiempoDesdeMordidaExitosa)
        //parametros distribucion gamma de tiempo de incubacion para Dengue
        double alpha=5.5;
        double beta=1.12;
        double acumProb = boost::math::gamma_p(alpha,timeSinceSuccesfullBite/beta);
        if (repast::Random::instance()->nextDouble() <= acumProb) {
            infectionState = "infected";
            timeSinceInfection = 0;
        }
    }


    //si la persona esta infectada se determina si pasa a recuperado
    if(infectionState == "infected") {
        //Hallemos la siguiente probabilidad p(tiempoDeInfeccion<=tiempoDesdeInfectado)
        //parametros distribucion uniforme de tiempo de infeccion para el dengue
        double a=2;
        double b=7;
        //se calcula la probbilidad acumulada de forma manual
        double acumProb=(timeSinceInfection-a)/(b-a);
        if (repast::Random::instance()->nextDouble() <= acumProb) {
            infectionState = "recovered";
        }
    }

} 

//auxiliares
//ESTA POR EL MOMENTO ES TEMPORAL HASTA QUE SE DEFINA EL ESPACIO 
double Human::calculateInfectionProbabilityHuman(){
    return 0.3;
}


/* serializable Agent package Data*/
HumanPackage::HumanPackage(){ }
HumanPackage::HumanPackage(int Id,int Rank, int Type, int CurrentRank, string InfectionState, int Age, int TimeSinceSuccesfullBite,int TimeSinceInfection, std::vector<int> HomeLocation, std::vector<std::vector<int>> Activities){
    id=Id;
    rank=Rank;
    type=Type;
    currentRank=CurrentRank;
    infectionState=InfectionState;
    age=Age;
    timeSinceSuccesfullBite=TimeSinceSuccesfullBite;
    timeSinceInfection=TimeSinceInfection;
    homeLocation=HomeLocation;
    activities=Activities;
} 