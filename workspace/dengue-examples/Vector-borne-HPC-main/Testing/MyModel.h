
#ifndef MyModel
#define MyModel
#include <fstream>
#include <boost/mpi.hpp>
#include "repast_hpc/Schedule.h"
#include "repast_hpc/Properties.h"
//the model class is the "arena" in which the population of agents are stored. The way that this is achieved is providing the model calass with a Repast Shared context object
#include "repast_hpc/SharedContext.h"
#include "repast_hpc/SharedDiscreteSpace.h"
#include "repast_hpc/GridComponents.h"
#include "repast_hpc/ValueLayerND.h"
#include "repast_hpc/AgentRequest.h"

#include "MyHuman.h"
/* Agent Package Provider (NO ESTOY MUY SEGURA DE PARA QUE SIRVE) */
class HumanPackageProvider {
	private:
    	repast::SharedContext<Human>* agents;
	
	public:
    	HumanPackageProvider(repast::SharedContext<Human>* agentPtr);
    	void providePackage(Human * agent, std::vector<HumanPackage>& out);
    	void provideContent(repast::AgentRequest req, std::vector<HumanPackage>& out);
	
};

/* Agent Package Receiver (NO ESTOY MUY SEGURA DE PARA QUE SIRVE) */
class HumanPackageReceiver {
	private:
		repast::SharedContext<Human>* agents;
		
	public:	
		HumanPackageReceiver(repast::SharedContext<Human>* agentPtr);
		Human * createAgent(HumanPackage package);
		void updateAgent(HumanPackage package);
	
};


class RepastHPCModel{
	//the following atributes will come from the model.props file
	int stopAt;
	int countOfHumans;
	int countOfInfectedHumans;
	int xdim;
	int ydim;

	repast::Properties* props;

	repast::SharedContext<Human> context; //this indicates that the model  needs a context 
	repast::SharedDiscreteSpace<Human, repast::StrictBorders, repast::SimpleAdder<Human> >* discreteSpace; //the model is in a discrete space
	
	//value layers que contiene suceptible,exposed, infected,temp and type of patch
	repast::ValueLayerND<double>* valueLayerSuceptibleMosquitoes;
	repast::ValueLayerND<double>* valueLayerExposedMosquitoes;
	repast::ValueLayerND<double>* valueLayerInfectedMosquitoes;     
	repast::ValueLayerND<double>* valueLayerTemperature;    
	repast::ValueLayerND<int>* valueLayerType;    
	
	//(NO ESTOY MUY SEGURA DE PARA QUE ES)
	HumanPackageProvider* provider;
	HumanPackageReceiver* receiver;
	
public:
	RepastHPCModel(std::string propsFile, int argc, char** argv, boost::mpi::communicator* comm);//constructor
	~RepastHPCModel();
	void init();
	void initHumans(); //se inizializan los agentes humanos
	void initGridValueLayers();//se inizializan los valores de los grid value layer

	void runAllHumans();//se corren los metodos de los humanos
	void runAllPatches();//se corren los metodos de los patches 

	void initSchedule(repast::ScheduleRunner& runner); //obligatorio

	// metodos auxiliares para la inizializacion
	int ageInitializer(double prob);
	std::vector<int> homeLocationInitializer();
	std::vector<std::vector<int>> activitiesInitializer(int age);
};

#endif
