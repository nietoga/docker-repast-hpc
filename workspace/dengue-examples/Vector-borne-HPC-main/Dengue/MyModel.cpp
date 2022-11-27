
#include <stdio.h>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <boost/mpi.hpp>
#include "repast_hpc/AgentId.h"
#include "repast_hpc/RepastProcess.h"
#include "repast_hpc/Utilities.h"
#include "repast_hpc/Properties.h"
#include "repast_hpc/initialize_random.h"
#include "repast_hpc/Point.h"
#include <utility>
#include "MyModel.h"
#include "SEIModel.h"
#include "MyReadData.h"



//OBLIGATORY (NO SE MUY BIEN PARA QUE ES)
HumanPackageProvider::HumanPackageProvider(repast::SharedContext<Human>* agentPtr): agents(agentPtr){ }

//OBLIGATORY (NO SE MUY BIEN PARA QUE ES)
void HumanPackageProvider::providePackage(Human * agent, std::vector<HumanPackage>& out){
	repast::AgentId id = agent->getId();
	HumanPackage package(id.id(), id.startingRank(), id.agentType(), id.currentRank(),agent->getInfectionState(), agent->getAge(), agent->getTimeSinceSuccesfullBite(), agent->getTimeSinceInfection(), agent->getHomeLocation(), agent->getActivities());
	out.push_back(package);
}

//OBLIGATORY (NO SE MUY BIEN PARA QUE ES)
void HumanPackageProvider::provideContent(repast::AgentRequest req, std::vector<HumanPackage>& out){
    std::vector<repast::AgentId> ids = req.requestedAgents();
    for(size_t i = 0; i < ids.size(); i++){
        providePackage(agents->getAgent(ids[i]), out);
    }
}

//OBLIGATORY (NO SE MUY BIEN PARA QUE ES)
HumanPackageReceiver::HumanPackageReceiver(repast::SharedContext<Human>* agentPtr): agents(agentPtr){}

//OBLIGATORY (NO SE MUY BIEN PARA QUE ES)
Human * HumanPackageReceiver::createAgent(HumanPackage package){
    repast::AgentId id(package.id, package.rank, package.type, package.currentRank);
    return new Human(id, package.infectionState, package.age, package.timeSinceSuccesfullBite, package.timeSinceInfection, package.homeLocation, package.activities);

}

//OBLIGATORY (NO SE MUY BIEN PARA QUE ES)
void HumanPackageReceiver::updateAgent(HumanPackage package){
    repast::AgentId id(package.id, package.rank, package.type,package.currentRank);//creo un id
    Human * agent = agents->getAgent(id);//cojo el agente que tiene ese id
    agent->setAll(package.infectionState, package.age, package.timeSinceSuccesfullBite, package.timeSinceInfection, package.homeLocation, package.activities);//le cambio las variables de estado
}

//CONSTRUCTOR 
RepastHPCModel::RepastHPCModel(std::string propsFile, int argc, char** argv, boost::mpi::communicator* comm): context(comm){
	props = new repast::Properties(propsFile, argc, argv, comm); 
	//atributes that come from the model.props file
	stopAt = repast::strToInt(props->getProperty("stop.at")); 
	countOfHumans= repast::strToInt(props->getProperty("count.of.humans"));
	countOfInfectedHumans= repast::strToInt(props->getProperty("count.of.infected.humans"));
	xdim=repast::strToInt(props->getProperty("x.dim"));
	ydim=repast::strToInt(props->getProperty("y.dim"));



	//manage randomness
	initializeRandom(*props,comm);

	/* CREATE INSTANCE OF SPATIAL PROJECTION */
	
	//define the dimensions
	repast::Point<double> origin(0,0);
	repast::Point<double> extent (xdim+1,ydim+1);//this will extent xdim units in the x asis and ydim units in the y axis
	repast::GridDimensions gd(origin,extent);
	
	//define process dimensions 
	std::vector<int> processDims;
	processDims.push_back(2);//there will be 2 proceses in the x dimension
	processDims.push_back(2);//there will be 2 proceses in the y dimension
	//this indicates that we will be runnning the simulation with 4 proceses

	//create a discrete space
	discreteSpace=new repast::SharedDiscreteSpace<Human,repast::StrictBorders , repast::SimpleAdder<Human> >("AgentDiscreteSpace",gd,processDims,0,comm);
	//we are chosing strict borders instead of wrap around
	//0 makes reference to the length of the buffer zone. In this case it is zero
	// comm= is the instance of the  boost::mpi::communicator*
	
	//add discreteSpace to context
	context.addProjection(discreteSpace);


	/* CREATE GRID VALUE LAYERS */
	//ValueLayerND(vector<int> processesPerDim, GridDimensions globalBoundaries, int bufferSize,bool periodic);
	int bufferSize=xdim/2;
	valueLayerSuceptibleMosquitoes=new repast::ValueLayerND<double>(processDims, gd,bufferSize,false);
	valueLayerExposedMosquitoes=new repast::ValueLayerND<double>(processDims, gd, bufferSize, false);
	valueLayerInfectedMosquitoes=new repast::ValueLayerND<double>(processDims, gd, bufferSize, false);    
	valueLayerTemperature=new repast::ValueLayerND<double>(processDims, gd, bufferSize, false);   
	valueLayerType=new repast::ValueLayerND<int>(processDims, gd, bufferSize, false);

	//read the temperature file and create the list that contains the data ----->NEW
	ReadData myData;
	std::string fileName ="temperatura-precipitacion-bello.csv"; 
	int simulationTime(stopAt);
	dataTemperatures=myData.loadFromExcel(fileName,simulationTime);
	
	//NO SE MUY BIEN PARA QUE SON LAS DOS SIGUIENTES LINEAS
	provider=new HumanPackageProvider(&context);
	receiver=new HumanPackageReceiver(&context);
	
}

RepastHPCModel::~RepastHPCModel(){
	delete props;
	delete valueLayerSuceptibleMosquitoes;
	delete valueLayerExposedMosquitoes;
	delete valueLayerInfectedMosquitoes;     
	delete valueLayerTemperature;    
	delete valueLayerType;  
	delete provider;
	delete receiver;
}
 void RepastHPCModel::init(){
	initHumans();
	initGridValueLayers();
	initRecords();
} 
//METODO PARA INIZIALIZAR LOS HUMANOS
void RepastHPCModel::initHumans(){
	std::cout <<"incializacion Humanos\n";
	int rank = repast::RepastProcess::instance()->rank();
	std::cout<<"rank: "<<rank<<"\n"; 

	if(rank == 0){
		for (int i=0; i<countOfHumans; i++){
			
			//SE DEFINE EL ID 
			repast::AgentId id(i,rank,0);
			id.currentRank(rank);

			//Se define la edad con ayuda del metodo ageInitializer
			double myRand=repast::Random::instance()->nextDouble();
			int age=ageInitializer(myRand);
			//std::cout<<myRand<<" "<<rank<<"\n";
			
			//se define la homelocation con ayuda del metodo homeLocationInitializer
			std::vector<int>homeLocation=homeLocationInitializer();
			
			//se definen las actividades con ayuda del metodo activitiesInitializer
			std::vector<std::vector<int>> activities=activitiesInitializer(age); 
			
			//se define todo lo relacionado al SEIR status
			string infectionState;
			int timeSinceSuccesfullBite;
			int timeSinceInfection;
			if(i<countOfHumans-countOfInfectedHumans){
				infectionState="suceptible";
				timeSinceSuccesfullBite=-1; //equivalent to Null
				timeSinceInfection=-1; //equivalent to null
			}
			else{
				infectionState="infected";
				timeSinceSuccesfullBite=0;
				timeSinceInfection=0;
			}
			//SE CREA EL HUMANO Y SE METE AL CONTEXT 
			Human* H=new Human(id,infectionState,age,timeSinceSuccesfullBite,timeSinceInfection,homeLocation,activities);
			context.addAgent(H);

			//SE UBICA EL HUMANO EN LA HOME LOCATION
			discreteSpace->moveTo(id,homeLocation);
			//repast::RepastProcess::instance()->synchronizeAgentStatus<Human, HumanPackage,HumanPackageProvider,HumanPackageReceiver>(context, *provider,*receiver,*receiver);



			//mirar si el humano si se ubico donde era
			std::vector<int>currentLocation;
			discreteSpace->getLocation(H->getId(),currentLocation);
			//imprimir humano
			std::cout<<"id: "<<H->getId()<<" age: "<<H->getAge()<<", infecion state: "<<H->getInfectionState()<<", timeSinceSuccesfullBite: "<<H->getTimeSinceSuccesfullBite()<<", timeSinceInfection: "<<H->getTimeSinceInfection()<<", homeLocation: ("<<H->getHomeLocation()[0]<<","<<H->getHomeLocation()[1]<<"), actividad1:("<<H->getActivities()[0][0]<<","<<H->getActivities()[0][1]<<"), actividad2:("<<H->getActivities()[1][0]<<","<<H->getActivities()[1][1]<<"),currentLocation:("<<currentLocation[0]<<";"<<currentLocation[1]<<")\n"; 
		}

	}
	

}

//metodo para la inizializacion de los grid value layers
void RepastHPCModel::initGridValueLayers(){
	//std::cout <<"incializacion patches\n";
	for (int i = 0; i < xdim; i++) {
		for (int j = 0; j < ydim; j++) {
			bool errorFlag1 = false;
			bool errorFlag2 = false;
			bool errorFlag3 = false;
			bool errorFlag4 = false;
			bool errorFlag5 = false;
			std::vector<int> location={i,j};

			//initialize number of suceptible, exposed and infected mosquitoes
			int rand_susc = repast::Random::instance()->createUniIntGenerator(0, 100).next();
			int rand_exp = repast::Random::instance()->createUniIntGenerator(0, 5).next();
			int rand_inf = repast::Random::instance()->createUniIntGenerator(0, 5).next();
			valueLayerSuceptibleMosquitoes-> setValueAt(rand_susc,location,errorFlag1);
			valueLayerExposedMosquitoes-> setValueAt(rand_exp,location,errorFlag2);
			valueLayerInfectedMosquitoes-> setValueAt(rand_inf,location,errorFlag3);
			

			//initialize temperature 
			int maxTempInitialDay=dataTemperatures[0][0];
			int minTempInitialDay=dataTemperatures[1][0];
			int temperature= repast::Random::instance()->createUniIntGenerator(minTempInitialDay,maxTempInitialDay).next();
			valueLayerTemperature-> setValueAt(temperature,location,errorFlag4);
			
			//initialize patch type
			if (i<=xdim/2 && j<=ydim/2) {
				valueLayerType->setValueAt(1,location,errorFlag5) ;//hace referencia a zona residencial
			}
			if (i<=xdim/2 && j>ydim/2) {
				valueLayerType->setValueAt(2,location,errorFlag5);//hace referencia a la zona de estudio
			}
			if (i>xdim/2 && j<=ydim/2) {
				valueLayerType->setValueAt(3,location,errorFlag5);//hace referencia a la zona de trabajo
			}
			if (i>xdim/2 && j>ydim/2) {
				valueLayerType->setValueAt(4,location,errorFlag5);//hace referencia a zona de otras actividades
			}
			//NO ESTOY MUY SEGURA SI ESTOS SYNCHRONIZE SON MUY NECESARIOS
			valueLayerSuceptibleMosquitoes->synchronize();
			valueLayerExposedMosquitoes->synchronize();
			valueLayerInfectedMosquitoes->synchronize();
			valueLayerType->synchronize();
			valueLayerTemperature->synchronize();
			//std::cout<<"Ubicacion:"<<location[0]<<","<<location[1]<<",sucepctibles:"<<valueLayerSuceptibleMosquitoes->getValueAt(location,errorFlag1)<<",expuestos:"<<valueLayerExposedMosquitoes->getValueAt(location,errorFlag1)<<",infectados:"<<valueLayerInfectedMosquitoes->getValueAt(location,errorFlag1)<<",tipo:"<<valueLayerType->getValueAt(location,errorFlag1)<<",temp:"<<valueLayerTemperature->getValueAt(location,errorFlag1)<<"\n";
		}
	}

}

//run all humans methods 
void RepastHPCModel::runAllHumans(){

	//repast::RepastProcess::instance()->synchronizeAgentStatus<Human, HumanPackage,HumanPackageProvider,HumanPackageReceiver>(context, *provider,*receiver,*receiver);

	std::vector<Human*> humans;
	context.selectAgents(countOfHumans, humans);//cojer todos humanos en el contexto y meterlos  en el vector de arriba
	std::vector<Human*>::iterator it=humans.begin(); //crear un iterador que itere sobre este vector y comienze con el primer pointer
	std::cout << "COMIENZA EL MOVIMIENTO HUMANO\n";
	//cojo a todos los humanos, a cada uno le extraigo  una actividad lo muevo hacia esa actividad, actualizo el seir status y esto se repite por la cantidad de actividades que haya
	for(int activityNum=0;activityNum<=1;activityNum++){
		it=humans.begin();
		if(activityNum==0){
			std::cout << "movimiento a other \n";
		}
		else{
			std::cout<<"moviemiento a school/work  \n";
		}
		while(it!=humans.end()){
			
			//cojamos el ID
			repast::AgentId id=(*it)->getId();
			//cojamos el old location
			std::vector<int>oldLocation;
			discreteSpace->getLocation(id,oldLocation);

			//cojamos el nuevo location al que seva a mover el humano
			std::vector<std::vector<int>> activities= (*it)-> getActivities();
			std::vector<int>activityLocation=activities[activityNum];
		

			//el humano se mueve a la ubicacion activity location
			discreteSpace->moveTo(id,activityLocation);

			//repast::RepastProcess::instance()->synchronizeAgentStatus<Human, HumanPackage,HumanPackageProvider,HumanPackageReceiver>(context, *provider,*receiver,*receiver);


			//se actualiza su SEIR Stauts ya que puede que en esta nueva ubicacion el humano se infecte
			(*it)-> actualizeSEIRStatus(&context);
			
			//se recaclula el numero de mosquitos suceptibles, infectados y expuestos en ese patch nuevo donde esta el humano. La temperatura no cambia porque no hay cambio de dia. Aca solo se hacen cmbios a los patches que ocurren por movimiento humano. 

			bool errorFlag1=false;
			//get current number or suceptible,exposed and infected mosquitoes in the patch and also the temperature 
 			int susceptible = valueLayerSuceptibleMosquitoes->getValueAt(activityLocation,errorFlag1);
			int exposed=valueLayerExposedMosquitoes->getValueAt(activityLocation,errorFlag1);
			int infected=valueLayerInfectedMosquitoes->getValueAt(activityLocation,errorFlag1);
			int temperature=valueLayerTemperature->getValueAt(activityLocation,errorFlag1); 

			//recalculate the number of suceptible and exposed mosquitoes 
			SEIModel patch(susceptible,exposed,infected,temperature);
			double h=0.1;patch.recalculateSEI(h);
			
			//actualize the grid value layers

			int newSusceptible=patch.getSuceptibleMosquitoes();
			int newExposed=patch.getExposedMosquitoes();
			int newInfected=patch.getInfectedMosquitoes();
			valueLayerSuceptibleMosquitoes-> setValueAt(newSusceptible,activityLocation,errorFlag1);
			valueLayerExposedMosquitoes-> setValueAt(newExposed,activityLocation,errorFlag1);
			valueLayerInfectedMosquitoes-> setValueAt(newInfected,activityLocation,errorFlag1);
			//print
			std::cout<< "id: "<<(*it)->getId()<<"age: "<<(*it)->getAge()<<", infecion state: "<<(*it)->getInfectionState()<<", timeSinceSuccesfullBite: "<<(*it)->getTimeSinceSuccesfullBite()<<", timeSinceInfection: "<<(*it)->getTimeSinceInfection()<<", previousLocation:"<<oldLocation[0] <<",  "<< oldLocation[1]<<", currentLoc:"<<activityLocation[0]<<" ,"<<activityLocation[1]<<"\n"; 
			it++;
		}
		//NO SE MUY BIEN COMO FUNCIONA LO SIGUIENTE
		/* discreteSpace->balance();
		repast::RepastProcess::instance()->synchronizeAgentStatus<Human,HumanPackage,HumanPackageProvider,HumanPackageReceiver>(context, *provider,*receiver,*receiver);
		repast::RepastProcess::instance()->synchronizeProjectionInfo<Human,HumanPackage,HumanPackageProvider,HumanPackageReceiver>(context, *provider,*receiver,*receiver);
		repast::RepastProcess::instance()->synchronizeAgentStates<HumanPackage,HumanPackageProvider,HumanPackageReceiver>(*provider,*receiver); */
	}
	//cuando los humanos estan ubicados en la actividad de estudioOTrabajo se devuelven a la casa y se actualizan los tiempos de infeccion y de mordidaexitosa
	std::cout<<"movimiento a la casa\n";
	it=humans.begin();
	while(it!=humans.end()){
		repast::AgentId id=(*it)->getId();
		std::vector<int> homeLocation= (*it)-> getHomeLocation();
		discreteSpace->moveTo(id,homeLocation);

		//repast::RepastProcess::instance()->synchronizeAgentStatus<Human, HumanPackage,HumanPackageProvider,HumanPackageReceiver>(context, *provider,*receiver,*receiver);


		(*it)-> actualizeTimes();

		std::vector<int>currentLocation;
		discreteSpace->getLocation(id,currentLocation);
		std::cout<<"id: "<<(*it)->getId()<<"age: "<<(*it)->getAge()<<", infecion state: "<<(*it)->getInfectionState()<<", timeSinceSuccesfullBite: "<<(*it)->getTimeSinceSuccesfullBite()<<", timeSinceInfection: "<<(*it)->getTimeSinceInfection()<<", previousLocation:"<<(*it)->getActivities()[1][0] <<";"<< (*it)->getActivities()[1][1]<<", currentLoc:"<<currentLocation[0]<<";"<<currentLocation[1]<<"\n";
		it++;
	}
	//NO SE MUY BIEN COMO FUNCIONA LO SIGUIENTE
	/* discreteSpace->balance();
	repast::RepastProcess::instance()->synchronizeAgentStatus<Human,HumanPackage,HumanPackageProvider,HumanPackageReceiver>(context, *provider,*receiver,*receiver);
	repast::RepastProcess::instance()->synchronizeProjectionInfo<Human,HumanPackage,HumanPackageProvider,HumanPackageReceiver>(context, *provider,*receiver,*receiver);
	repast::RepastProcess::instance()->synchronizeAgentStates<HumanPackage,HumanPackageProvider,HumanPackageReceiver>(*provider,*receiver); */
}

//actualize gridValueLayers each day (one tick)
void RepastHPCModel::runAllPatches(){
	//std::cout<<"actualizacion patches\n";
	int tick=repast::RepastProcess::instance()->getScheduleRunner().currentTick();
	cout<<"the current tick is:"<<tick<<"\n";
	for (int i = 0; i < xdim; i++) {
		for (int j = 0; j < ydim; j++) {
			bool errorFlag1 = false;bool errorFlag2 = false;bool errorFlag3 = false;bool errorFlag4 = false;
			std::vector<int> location={i,j};
			bool errorFlag=false;
			//actualize number of suceptible, exposed and infected mosquitoes
			int susceptible = valueLayerSuceptibleMosquitoes->getValueAt(location,errorFlag1);
			int exposed=valueLayerExposedMosquitoes->getValueAt(location,errorFlag1);
			int infected=valueLayerInfectedMosquitoes->getValueAt(location,errorFlag1);
			int temperature=valueLayerTemperature->getValueAt(location,errorFlag1);
			
			SEIModel patch(susceptible,exposed,infected,temperature);
			double h=0.1;
			patch.actualizeTemp(tick-1,dataTemperatures);
			patch.recalculateSEI(h);
			int newSusceptible=patch.getSuceptibleMosquitoes();
			int newExposed=patch.getExposedMosquitoes();
			int newInfected=patch.getInfectedMosquitoes();
			int newTemp=patch.getTemp();

			valueLayerSuceptibleMosquitoes-> setValueAt(newSusceptible,location,errorFlag1);
			valueLayerExposedMosquitoes-> setValueAt(newExposed,location,errorFlag1);
			valueLayerInfectedMosquitoes-> setValueAt(newInfected,location,errorFlag1);
			valueLayerTemperature-> setValueAt(newTemp,location,errorFlag1);

			bool errorFlag6=false;
			//Ver como funcionan estos synchronize
			valueLayerSuceptibleMosquitoes->synchronize();
			valueLayerExposedMosquitoes->synchronize();
			valueLayerInfectedMosquitoes->synchronize();
			valueLayerType->synchronize();
			valueLayerTemperature->synchronize();
			//std::cout<<"error:"<<errorFlag6<<" ,ubicacion:"<<location[0]<<","<<location[1]<<",sucepctibles:"<<valueLayerSuceptibleMosquitoes->getValueAt(location,errorFlag6)<<",expuestos:"<<valueLayerExposedMosquitoes->getValueAt(location,errorFlag6)<<",infectados:"<<valueLayerInfectedMosquitoes->getValueAt(location,errorFlag6)<<",tipo:"<<valueLayerType->getValueAt(location,errorFlag6)<<",temp:"<<valueLayerTemperature->getValueAt(location,errorFlag6)<<"\n";
		}
	}

	
	//MIRAR ESTO	
	//repast::RepastProcess::instance()->synchronizeProjectionInfo<Human,HumanPackage,HumanPackageProvider,HumanPackageReceiver>(context, *provider,*receiver,*receiver);
	//repast::RepastProcess::instance()->synchronizeAgentStates<HumanPackage,HumanPackageProvider,HumanPackageReceiver>(*provider,*receiver);
}
//string fileName

void RepastHPCModel::initRecords(){
	int rank = repast::RepastProcess::instance()->rank();
	if(rank == 0){
		writer.initCSVFile();
		/* std::ofstream ofs("test.csv", std::ofstream::trunc);
		ofs << "tick"<< ","<<"TotalHumans"<< ","<<"Susceptible"<<","<<"Exposed"<<","<<"Infected"<<","<<"Recovered"<<"\n";
		ofs.close(); */
	}
}
void RepastHPCModel::recordResults(){
	int rank = repast::RepastProcess::instance()->rank();

	if(rank == 0){
		int* totals = countHumansInState();
		writer.appendRecord(totals);
		/* std::ofstream outfile;
		outfile.open("test.csv", std::ios_base::app); // append instead of overwrite
		outfile << tick << ","<<countOfHumans<< ","<<totals[0]<<","<<totals[1]<<","<<totals[2]<<","<<totals[3]<<"\n"; */
	}
}

//aca es donde se ejecuta toda la simualcion con la ayuda del runner.scheduleEvent
void RepastHPCModel::initSchedule(repast::ScheduleRunner& runner){
	runner.scheduleEvent(1, 1, repast::Schedule::FunctorPtr(new repast::MethodFunctor<RepastHPCModel> (this, &RepastHPCModel::runAllPatches))); //aca lo que hace esto es llamar al metodo runAllPatches cada 1 tick y desde el primer tick. Este metodo recibe (startingTime, cadaCuantoSeReptie...)
	runner.scheduleEvent(1, 1, repast::Schedule::FunctorPtr(new repast::MethodFunctor<RepastHPCModel> (this, &RepastHPCModel::runAllHumans))); //aca lo que hace esto es llamar al metodo runAllHumans cada 1 tick y desde el primer tick. Este metodo recibe (startingTime, cadaCuantoSeReptie...) */
	runner.scheduleEvent(1,1, repast::Schedule::FunctorPtr(new repast::MethodFunctor<RepastHPCModel> (this, &RepastHPCModel::recordResults))); //this event is schedulled to run after the simulation arrives at the stop end
	runner.scheduleStop(stopAt);
}


int* RepastHPCModel::countHumansInState(){
	int *totals = new int [6];

	int current_tick = repast::RepastProcess::instance()->getScheduleRunner().currentTick();;
	int total_humans = countOfHumans;
	int total_susceptible = 0;
	int total_exposed = 0;
	int total_infected= 0;
	int total_recovered = 0;

	std::vector<Human*> humans;
	context.selectAgents(countOfHumans, humans);//cojer todos humanos en el contexto y meterlos  en el vector de arriba
	std::vector<Human*>::iterator it=humans.begin(); //crear un iterador que itere sobre este vector y comienze con el primer pointer
	while(it!=humans.end()){
		//cojamos el ID
		repast::AgentId id=(*it)->getId();
		string human_infection_state = (*it)->getInfectionState();
		if (human_infection_state == "suceptible"){
			total_susceptible = total_susceptible + 1;
		}
		if (human_infection_state == "exposed"){
			total_exposed = total_exposed + 1;
			
		}
		if (human_infection_state == "infected"){
			total_infected= total_infected + 1;
		}	
		if (human_infection_state == "recovered"){
			total_recovered= total_recovered + 1;
		}	
		
		it++;
	}
	totals[0] = current_tick;
	totals[1] = total_humans;
	totals[2] = total_susceptible;
	totals[3] = total_exposed ;
	totals[4] = total_infected;
	totals[5] = total_recovered;
	return totals;
}


//metodo auxiliar para la inizialicion
int RepastHPCModel::ageInitializer(double prob) {
	if(0<=prob && prob<0.0811){ //los numeros de distribuciones se crean asi
		return repast::Random::instance()->createUniIntGenerator(0, 4).next();
	}
	if (0.0811 <= prob && prob < 0.1615) { //tambien se pueden crear en un solo paso 
		return repast::Random::instance()->createUniIntGenerator(5, 9).next();
	}
	if (0.1615 <= prob && prob < 0.2435) {
		return repast::Random::instance()->createUniIntGenerator(10, 14).next();
	}
	 if (0.2435 <= prob && prob < 0.3292) {
		return repast::Random::instance()->createUniIntGenerator(15, 19).next();
	 }
	if (0.3292 <= prob && prob < 0.4249) {
		return repast::Random::instance()->createUniIntGenerator(20, 24).next();
	}
	if (0.4249 <= prob && prob < 0.5196) {
		return repast::Random::instance()->createUniIntGenerator(25, 29).next();
	}
	if (0.5196 <= prob && prob < 0.6025) {
		return repast::Random::instance()->createUniIntGenerator(30, 34).next();
	}
	if (0.6025 <= prob && prob < 0.6801) {
		return repast::Random::instance()->createUniIntGenerator(35, 39).next();
	}
	if (0.6801 <= prob && prob < 0.7506) {
		return repast::Random::instance()->createUniIntGenerator(40,44).next();
	} 
	if (0.7506 <= prob && prob < 0.8097) {
		return repast::Random::instance()->createUniIntGenerator(45, 49).next();
	}
	if (0.8097 <= prob && prob < 0.8626) {
		return repast::Random::instance()->createUniIntGenerator(50, 54).next();
	} 
	if (0.8626 <= prob && prob < 0.9062) {
		return repast::Random::instance()->createUniIntGenerator(55, 59).next();
	} 
	if (0.9062 <= prob && prob < 0.9378) {
		return repast::Random::instance()->createUniIntGenerator(60, 64).next();

	} 
	if (0.9378 <= prob && prob < 0.9601) {
		return repast::Random::instance()->createUniIntGenerator(65, 69).next();
	}
	if (0.9601 <= prob && prob < 0.9768) {
		return repast::Random::instance()->createUniIntGenerator(70, 74).next();
	} 
	if (0.9768 <= prob && prob < 0.9890) {
		return repast::Random::instance()->createUniIntGenerator(75, 79).next();
	}
	if (0.9890 <= prob && prob <= 1) {
		return repast::Random::instance()->createUniIntGenerator(80, 90).next();
	} 
} 
//metodo auxiliar para la inizialicion
std::vector<int> RepastHPCModel::homeLocationInitializer(){
		int xHome=repast::Random::instance()->createUniIntGenerator(0,xdim/2).next();
		int yHome=repast::Random::instance()->createUniIntGenerator(0,ydim/2).next();
		std::vector<int> home={xHome,yHome};
		return home;
}

//metodo auxiliar para la inizialicion
std::vector<std::vector<int>> RepastHPCModel::activitiesInitializer(int age){
	//creacion de las coordenadas de la actividad "otro"
	int xOther=repast::Random::instance()->createUniIntGenerator(xdim/2,xdim).next();
	int yOther=repast::Random::instance()->createUniIntGenerator(ydim/2, ydim).next();
	std::vector<int> other={xOther,yOther};
	
	//creacion de las coordenadas de la actividad estudioOTrabajo dependiendo de la edad
	std::vector<int> studyOrWork; 
	if (age<=24){//la persona estudia
		int xStudyOrWork=repast::Random::instance()->createUniIntGenerator(0, xdim/2).next();
		int yStudyOrWork=repast::Random::instance()->createUniIntGenerator(ydim/2, ydim).next();
		studyOrWork={xStudyOrWork,yStudyOrWork};
	}
	else{ //la persona trabaja
		int xStudyOrWork=repast::Random::instance()->createUniIntGenerator(xdim/2,xdim).next();
		int yStudyOrWork=repast::Random::instance()->createUniIntGenerator(0, ydim/2).next();
		studyOrWork={xStudyOrWork,yStudyOrWork};
	}
	std::vector<std::vector<int>> activitiesCoordinates={other,studyOrWork};
	return activitiesCoordinates;
}



