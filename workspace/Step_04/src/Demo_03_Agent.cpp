/* Demo_03_Agent.cpp */

#include "Demo_03_Agent.h"
#include "repast_hpc/Moore2DGridQuery.h"
#include "repast_hpc/Point.h"

RepastHPCDemoAgent::RepastHPCDemoAgent(repast::AgentId id, int maxSiblings) : id(id), maxSiblings(maxSiblings) {}

RepastHPCDemoAgent::~RepastHPCDemoAgent() {}

void RepastHPCDemoAgent::move(repast::SharedDiscreteSpace<RepastHPCDemoAgent, repast::StrictBorders, repast::SimpleAdder<RepastHPCDemoAgent>> *space)
{
    std::vector<int> agentLoc;
    space->getLocation(id, agentLoc);

    std::vector<RepastHPCDemoAgent *> agents;
    repast::Moore2DGridQuery<RepastHPCDemoAgent> moore2DQuery(space);
    moore2DQuery.query(agentLoc, 0, true, agents);

    if (agents.size() <= maxSiblings)
    {
        return;
    }

    std::vector<std::vector<int>> options = {{-1, 0}, {0, -1}, {1, 0}, {0, 1}};

    std::vector<int> agentNewLoc;

    do
    {
        agentNewLoc.clear();
        int pos = (int)(repast::Random::instance()->nextDouble() * 4);
        std::vector<int> option = options[pos];
        agentNewLoc.push_back(agentLoc[0] + option[0]);
        agentNewLoc.push_back(agentLoc[1] + option[1]);
    } while (!space->bounds().contains(agentNewLoc));

    // std::cout << "Moving " << id << " from "
    //           << agentLoc[0] << "," << agentLoc[1] << " to "
    //           << agentNewLoc[0] << "," << agentNewLoc[1] << std::endl;
    space->moveTo(id, agentNewLoc);
}

RepastHPCDemoAgentPackage::RepastHPCDemoAgentPackage() {}

RepastHPCDemoAgentPackage::RepastHPCDemoAgentPackage(int id, int rank, int type, int currentRank, int maxSiblings)
    : id(id), rank(rank), type(type), currentRank(currentRank), maxSiblings(maxSiblings) {}

/**
 * For some reason it doesn't work on Model.h
 * Adding #include "repast_hpc/Moore2DGridQuery.h" to Model.cpp breaks it.
 * error: 'Grid' does not name a type
 */
DataSource_GridCount::DataSource_GridCount(repast::SharedDiscreteSpace<RepastHPCDemoAgent, repast::StrictBorders, repast::SimpleAdder<RepastHPCDemoAgent>> *discreteSpace, int i, int j)
    : discreteSpace(discreteSpace), i(i), j(j) {}

int DataSource_GridCount::getData()
{
    repast::Point<int> center(i, j);

    if (!discreteSpace->dimensions().contains(center))
    {
        return 0;
    }

    std::vector<RepastHPCDemoAgent *> agents;
    repast::Moore2DGridQuery<RepastHPCDemoAgent> moore2DQuery(discreteSpace);
    moore2DQuery.query(center, 0, true, agents);

    return agents.size();
}
