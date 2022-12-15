/* Demo_03_Model.cpp */

#include <stdio.h>
#include <vector>
#include <string>
#include <boost/mpi.hpp>
#include "repast_hpc/AgentId.h"
#include "repast_hpc/RepastProcess.h"
#include "repast_hpc/Utilities.h"
#include "repast_hpc/Properties.h"
#include "repast_hpc/initialize_random.h"
#include "repast_hpc/SVDataSetBuilder.h"
#include "repast_hpc/Point.h"

#include "Demo_03_Model.h"

RepastHPCDemoAgentPackageProvider::RepastHPCDemoAgentPackageProvider(repast::SharedContext<RepastHPCDemoAgent> *agentPtr) : agents(agentPtr) {}

void RepastHPCDemoAgentPackageProvider::providePackage(RepastHPCDemoAgent *agent, std::vector<RepastHPCDemoAgentPackage> &out)
{
	repast::AgentId id = agent->getId();
	RepastHPCDemoAgentPackage package(id.id(), id.startingRank(), id.agentType(), id.currentRank(), agent->getC(), agent->getTotal());
	out.push_back(package);
}

void RepastHPCDemoAgentPackageProvider::provideContent(repast::AgentRequest req, std::vector<RepastHPCDemoAgentPackage> &out)
{
	std::vector<repast::AgentId> ids = req.requestedAgents();
	for (size_t i = 0; i < ids.size(); i++)
	{
		providePackage(agents->getAgent(ids[i]), out);
	}
}

RepastHPCDemoAgentPackageReceiver::RepastHPCDemoAgentPackageReceiver(repast::SharedContext<RepastHPCDemoAgent> *agentPtr) : agents(agentPtr) {}

RepastHPCDemoAgent *RepastHPCDemoAgentPackageReceiver::createAgent(RepastHPCDemoAgentPackage package)
{
	repast::AgentId id(package.id, package.rank, package.type, package.currentRank);
	return new RepastHPCDemoAgent(id, package.c, package.total);
}

void RepastHPCDemoAgentPackageReceiver::updateAgent(RepastHPCDemoAgentPackage package)
{
	repast::AgentId id(package.id, package.rank, package.type);
	RepastHPCDemoAgent *agent = agents->getAgent(id);
	agent->set(package.currentRank, package.c, package.total);
}

DataSource_AgentTotals::DataSource_AgentTotals(repast::SharedContext<RepastHPCDemoAgent> *c) : context(c) {}

int DataSource_AgentTotals::getData()
{
	int sum = 0;
	repast::SharedContext<RepastHPCDemoAgent>::const_local_iterator iter = context->localBegin();
	repast::SharedContext<RepastHPCDemoAgent>::const_local_iterator iterEnd = context->localEnd();
	while (iter != iterEnd)
	{
		sum += (*iter)->getTotal();
		iter++;
	}
	return sum;
}

DataSource_AgentCTotals::DataSource_AgentCTotals(repast::SharedContext<RepastHPCDemoAgent> *c) : context(c) {}

int DataSource_AgentCTotals::getData()
{
	int sum = 0;
	repast::SharedContext<RepastHPCDemoAgent>::const_local_iterator iter = context->localBegin();
	repast::SharedContext<RepastHPCDemoAgent>::const_local_iterator iterEnd = context->localEnd();
	while (iter != iterEnd)
	{
		sum += (*iter)->getC();
		iter++;
	}
	return sum;
}

RepastHPCDemoModel::RepastHPCDemoModel(std::string propsFile, int argc, char **argv, boost::mpi::communicator *comm) : context(comm)
{
	props = new repast::Properties(propsFile, argc, argv, comm);
	stopAt = repast::strToInt(props->getProperty("stop.at"));
	countOfAgents = repast::strToInt(props->getProperty("count.of.agents"));

	int originX = repast::strToInt(props->getProperty("grid.origin.x"));
	int originY = repast::strToInt(props->getProperty("grid.origin.y"));
	int extentX = repast::strToInt(props->getProperty("grid.extent.x"));
	int extentY = repast::strToInt(props->getProperty("grid.extent.y"));

	initializeRandom(*props, comm);
	if (repast::RepastProcess::instance()->rank() == 0)
		props->writeToSVFile("./output/record.csv");
	provider = new RepastHPCDemoAgentPackageProvider(&context);
	receiver = new RepastHPCDemoAgentPackageReceiver(&context);

	repast::Point<double> origin(originX, originY);
	repast::Point<double> extent(extentX, extentY);

	repast::GridDimensions gd(origin, extent);

	std::vector<int> processDims;
	processDims.push_back(2);
	processDims.push_back(2);

	discreteSpace = new repast::SharedDiscreteSpace<RepastHPCDemoAgent, repast::StrictBorders, repast::SimpleAdder<RepastHPCDemoAgent>>("AgentDiscreteSpace", gd, processDims, 5, comm);

	std::cout << "RANK " << repast::RepastProcess::instance()->rank() << " BOUNDS: " << discreteSpace->dimensions().origin() << " " << discreteSpace->dimensions().extents() << std::endl;

	context.addProjection(discreteSpace);

	// Data collection
	// Create the data set builder
	std::string fileOutputName("./output/agent_total_data.csv");
	repast::SVDataSetBuilder builder(fileOutputName.c_str(), ",", repast::RepastProcess::instance()->getScheduleRunner().schedule());

	// Create the individual data sets to be added to the builder
	DataSource_AgentTotals *agentTotals_DataSource = new DataSource_AgentTotals(&context);
	builder.addDataSource(createSVDataSource("Total", agentTotals_DataSource, std::plus<int>()));

	DataSource_AgentCTotals *agentCTotals_DataSource = new DataSource_AgentCTotals(&context);
	builder.addDataSource(createSVDataSource("C", agentCTotals_DataSource, std::plus<int>()));

	for (int i = originX; i <= originX + extentX; i++)
	{
		for (int j = originY; j <= originY + extentY; j++)
		{
			std::string gridName = "Grid_X" + std::to_string(i) + "_Y" + std::to_string(j);
			DataSource_GridCount *gridCount_DataSource = new DataSource_GridCount(discreteSpace, i, j);
			builder.addDataSource(createSVDataSource(gridName, gridCount_DataSource, std::plus<int>()));
		}
	}

	// Use the builder to create the data set
	agentValues = builder.createDataSet();
}

RepastHPCDemoModel::~RepastHPCDemoModel()
{
	delete props;
	delete provider;
	delete receiver;

	delete agentValues;
}

void RepastHPCDemoModel::init()
{
	int n = repast::RepastProcess::instance()->worldSize();
	int rank = repast::RepastProcess::instance()->rank();

	int sliceSize = countOfAgents / n;

	if (rank == 0)
	{
		sliceSize += countOfAgents % n;
	}

	for (int i = 0; i < sliceSize; i++)
	{
		repast::AgentId id(rank * sliceSize + i, rank, 0);
		id.currentRank(rank);

		RepastHPCDemoAgent *agent = new RepastHPCDemoAgent(id);
		context.addAgent(agent);

		double offsetX = repast::Random::instance()->nextDouble() * discreteSpace->dimensions().extents().getX();
		double offsetY = repast::Random::instance()->nextDouble() * discreteSpace->dimensions().extents().getY();

		int x = (int)(discreteSpace->dimensions().origin().getX() + offsetX);
		int y = (int)(discreteSpace->dimensions().origin().getY() + offsetY);
		repast::Point<int> initialLocation(x, y);
		discreteSpace->moveTo(id, initialLocation);
	}
}

void RepastHPCDemoModel::doSomething()
{
	std::vector<RepastHPCDemoAgent *> agents;
	context.selectAgents(repast::SharedContext<RepastHPCDemoAgent>::LOCAL, countOfAgents, agents);
	std::vector<RepastHPCDemoAgent *>::iterator it = agents.begin();

	while (it != agents.end())
	{
		(*it)->play(&context, discreteSpace);
		it++;
	}

	it = agents.begin();
	while (it != agents.end())
	{
		(*it)->move(discreteSpace);
		it++;
	}

	discreteSpace->balance();
	repast::RepastProcess::instance()->synchronizeAgentStatus<RepastHPCDemoAgent, RepastHPCDemoAgentPackage, RepastHPCDemoAgentPackageProvider, RepastHPCDemoAgentPackageReceiver>(context, *provider, *receiver, *receiver);
	repast::RepastProcess::instance()->synchronizeProjectionInfo<RepastHPCDemoAgent, RepastHPCDemoAgentPackage, RepastHPCDemoAgentPackageProvider, RepastHPCDemoAgentPackageReceiver>(context, *provider, *receiver, *receiver);
	repast::RepastProcess::instance()->synchronizeAgentStates<RepastHPCDemoAgentPackage, RepastHPCDemoAgentPackageProvider, RepastHPCDemoAgentPackageReceiver>(*provider, *receiver);
}

void RepastHPCDemoModel::initSchedule(repast::ScheduleRunner &runner)
{
	runner.scheduleEvent(1, 1, repast::Schedule::FunctorPtr(new repast::MethodFunctor<RepastHPCDemoModel>(this, &RepastHPCDemoModel::doSomething)));
	runner.scheduleEndEvent(repast::Schedule::FunctorPtr(new repast::MethodFunctor<RepastHPCDemoModel>(this, &RepastHPCDemoModel::recordResults)));
	runner.scheduleStop(stopAt);

	// Data collection
	runner.scheduleEvent(0.5, 1, repast::Schedule::FunctorPtr(new repast::MethodFunctor<repast::DataSet>(agentValues, &repast::DataSet::record)));
	runner.scheduleEndEvent(repast::Schedule::FunctorPtr(new repast::MethodFunctor<repast::DataSet>(agentValues, &repast::DataSet::write)));
}

void RepastHPCDemoModel::recordResults()
{
	if (repast::RepastProcess::instance()->rank() == 0)
	{
		props->putProperty("Result", "Passed");
		std::vector<std::string> keyOrder;
		keyOrder.push_back("stop.at");
		keyOrder.push_back("Result");
		props->writeToSVFile("./output/results.csv", keyOrder);
	}
}
