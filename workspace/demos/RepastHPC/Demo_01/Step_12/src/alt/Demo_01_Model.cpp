/* Demo_01_Model.cpp */

#include <stdio.h>
#include <vector>
#include <boost/mpi.hpp>
#include "repast_hpc/AgentId.h"
#include "repast_hpc/RepastProcess.h"
#include "repast_hpc/Utilities.h"
#include "repast_hpc/Properties.h"
#include "repast_hpc/initialize_random.h"

#include "Demo_01_Model.h"


RepastHPCDemoAgentPackageProvider::RepastHPCDemoAgentPackageProvider(repast::SharedContext<RepastHPCDemoAgent>* agentPtr): agents(agentPtr){ }

void RepastHPCDemoAgentPackageProvider::providePackage(RepastHPCDemoAgent * agent, std::vector<RepastHPCDemoAgentPackage>& out){
    repast::AgentId id = agent->getId();
    RepastHPCDemoAgentPackage package(id.id(), id.startingRank(), id.agentType(), id.currentRank(), agent->getC(), agent->getTotal());
    out.push_back(package);
}

void RepastHPCDemoAgentPackageProvider::provideContent(repast::AgentRequest req, std::vector<RepastHPCDemoAgentPackage>& out){
    std::vector<repast::AgentId> ids = req.requestedAgents();
    for(size_t i = 0; i < ids.size(); i++){
        providePackage(agents->getAgent(ids[i]), out);
    }
}


RepastHPCDemoAgentPackageReceiver::RepastHPCDemoAgentPackageReceiver(repast::SharedContext<RepastHPCDemoAgent>* agentPtr): agents(agentPtr){}

RepastHPCDemoAgent * RepastHPCDemoAgentPackageReceiver::createAgent(RepastHPCDemoAgentPackage package){
    repast::AgentId id(package.id, package.rank, package.type);
    return new RepastHPCDemoAgent(id, package.c, package.total);
}

void RepastHPCDemoAgentPackageReceiver::updateAgent(RepastHPCDemoAgentPackage package){
    repast::AgentId id(package.id, package.rank, package.type);
    RepastHPCDemoAgent * agent = agents->getAgent(id);
    agent->set(package.currentRank, package.c, package.total);
}


RepastHPCDemoModel::RepastHPCDemoModel(std::string propsFile, int argc, char** argv, boost::mpi::communicator* comm): context(comm){
	props = new repast::Properties(propsFile, argc, argv, comm);
	stopAt = repast::strToInt(props->getProperty("stop.at"));
	countOfAgents = repast::strToInt(props->getProperty("count.of.agents"));
	initializeRandom(*props, comm);
	if(repast::RepastProcess::instance()->rank() == 0) props->writeToSVFile("./output/record.csv");
	provider = new RepastHPCDemoAgentPackageProvider(&context);
	receiver = new RepastHPCDemoAgentPackageReceiver(&context);
}

RepastHPCDemoModel::~RepastHPCDemoModel(){
	delete props;
	delete provider;
	delete receiver;

}

void RepastHPCDemoModel::init(){
	int rank = repast::RepastProcess::instance()->rank();
	for(int i = 0; i < countOfAgents; i++){
		repast::AgentId id(i, rank, 0);
		id.currentRank(rank);
		RepastHPCDemoAgent* agent = new RepastHPCDemoAgent(id);
		context.addAgent(agent);
	}
}

void RepastHPCDemoModel::requestAgents(){
	int rank = repast::RepastProcess::instance()->rank();
	int worldSize= repast::RepastProcess::instance()->worldSize();
	repast::AgentRequest req(rank);
	for(int i = 0; i < worldSize; i++){                     // For each process
		if(i != rank){                                      // ... except this one
			std::vector<RepastHPCDemoAgent*> agents;        
			context.selectAgents(5, agents);                // Choose 5 local agents randomly
			for(size_t j = 0; j < agents.size(); j++){
				repast::AgentId local = agents[j]->getId();          // Transform each local agent's id into a matching non-local one
				repast::AgentId other(local.id(), i, 0);
				other.currentRank(i);
				req.addRequest(other);                      // Add it to the agent request
			}
		}
	}
	repast::RepastProcess::instance()->requestAgents<RepastHPCDemoAgent, RepastHPCDemoAgentPackage, RepastHPCDemoAgentPackageProvider, RepastHPCDemoAgentPackageReceiver>(context, req, *provider, *receiver, *receiver);

}

void RepastHPCDemoModel::doSomething(){
	int whichRank = 3;
	if(repast::RepastProcess::instance()->rank() == whichRank) std::cout << " TICK " << repast::RepastProcess::instance()->getScheduleRunner().currentTick() << std::endl;
	if(repast::RepastProcess::instance()->rank() == whichRank){
	    std::cout << " BEFORE PLAY: " << std::endl;
    	for(int i = 0; i < 10; i++){
			repast::AgentId toDisplay(i, whichRank, 0);
			toDisplay.currentRank(whichRank);
			RepastHPCDemoAgent* agent = context.getAgent(toDisplay);
			std::cout << agent->getId() << " " << agent->getC() << " " << agent->getTotal() << std::endl;
		}
	
		for(int r = 0; r < 4; r++){
			if(r != whichRank){
				for(int i = 0; i < 10; i++){
					repast::AgentId toDisplay(i, r, 0);
					toDisplay.currentRank(r);
					RepastHPCDemoAgent* agent = context.getAgent(toDisplay);
					if(agent != 0) std::cout << agent->getId() << " " << agent->getC() << " " << agent->getTotal() << std::endl;
				}
			}
		}
	}
	
	std::vector<RepastHPCDemoAgent*> agents;
	context.selectAgents(repast::SharedContext<RepastHPCDemoAgent>::LOCAL, countOfAgents, agents);
	std::vector<RepastHPCDemoAgent*>::iterator it = agents.begin();
	while(it != agents.end()){
		(*it)->play(&context);
		it++;
    }

	
	if(repast::RepastProcess::instance()->rank() == whichRank){
	    std::cout << " AFTER PLAY, BEFORE SYNC: " << std::endl;
    	for(int i = 0; i < 10; i++){
			repast::AgentId toDisplay(i, whichRank, 0);
			toDisplay.currentRank(whichRank);
			RepastHPCDemoAgent* agent = context.getAgent(toDisplay);
			std::cout << agent->getId() << " " << agent->getC() << " " << agent->getTotal() << std::endl;
		}
		
		for(int r = 0; r < 4; r++){
			if(r != whichRank){
				for(int i = 0; i < 10; i++){
					repast::AgentId toDisplay(i, r, 0);
					toDisplay.currentRank(r);
					RepastHPCDemoAgent* agent = context.getAgent(toDisplay);
					if(agent != 0) std::cout << agent->getId() << " " << agent->getC() << " " << agent->getTotal() << std::endl;
				}
			}
		}
	}
	
	
	repast::RepastProcess::instance()->synchronizeAgentStates<RepastHPCDemoAgentPackage, RepastHPCDemoAgentPackageProvider, RepastHPCDemoAgentPackageReceiver>(*provider, *receiver);

	
	if(repast::RepastProcess::instance()->rank() == whichRank){
	    std::cout << " AFTER SYNC: " << std::endl;
    	for(int i = 0; i < 10; i++){
			repast::AgentId toDisplay(i, whichRank, 0);
			toDisplay.currentRank(whichRank);
			RepastHPCDemoAgent* agent = context.getAgent(toDisplay);
			std::cout << agent->getId() << " " << agent->getC() << " " << agent->getTotal() << std::endl;
		}
		
		for(int r = 0; r < 4; r++){
			if(r != whichRank){
				for(int i = 0; i < 10; i++){
					repast::AgentId toDisplay(i, r, 0);
					toDisplay.currentRank(r);
					RepastHPCDemoAgent* agent = context.getAgent(toDisplay);
					if(agent != 0) std::cout << agent->getId() << " " << agent->getC() << " " << agent->getTotal() << std::endl;
				}
			}
		}
	}
}

void RepastHPCDemoModel::initSchedule(repast::ScheduleRunner& runner){
	runner.scheduleEvent(1, repast::Schedule::FunctorPtr(new repast::MethodFunctor<RepastHPCDemoModel> (this, &RepastHPCDemoModel::requestAgents)));
	runner.scheduleEvent(2, 1, repast::Schedule::FunctorPtr(new repast::MethodFunctor<RepastHPCDemoModel> (this, &RepastHPCDemoModel::doSomething)));
	runner.scheduleEndEvent(repast::Schedule::FunctorPtr(new repast::MethodFunctor<RepastHPCDemoModel> (this, &RepastHPCDemoModel::recordResults)));
	runner.scheduleStop(stopAt);
}

void RepastHPCDemoModel::recordResults(){
	if(repast::RepastProcess::instance()->rank() == 0){
		props->putProperty("Result","Passed");
		std::vector<std::string> keyOrder;
		keyOrder.push_back("RunNumber");
		keyOrder.push_back("stop.at");
		keyOrder.push_back("Result");
		props->writeToSVFile("./output/results.csv", keyOrder);
    }
}


