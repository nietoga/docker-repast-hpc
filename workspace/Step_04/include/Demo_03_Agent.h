/* Demo_03_Agent.h */

#ifndef DEMO_03_AGENT
#define DEMO_03_AGENT

#include "repast_hpc/AgentId.h"
#include "repast_hpc/TDataSource.h"
#include "repast_hpc/SharedContext.h"
#include "repast_hpc/SharedDiscreteSpace.h"

class RepastHPCDemoAgent
{

private:
    repast::AgentId id;
    int maxSiblings;

public:
    RepastHPCDemoAgent(repast::AgentId id, int maxSiblings);

    ~RepastHPCDemoAgent();

    virtual repast::AgentId &getId() { return id; }

    virtual const repast::AgentId &getId() const { return id; }

    int getMaxSiblings() { return maxSiblings; }

    void setMaxSiblings(int maxSiblings) { this->maxSiblings = maxSiblings; }

    void move(repast::SharedDiscreteSpace<RepastHPCDemoAgent, repast::StrictBorders, repast::SimpleAdder<RepastHPCDemoAgent>> *space);
};

struct RepastHPCDemoAgentPackage
{

public:
    int id;
    int rank;
    int type;
    int currentRank;
    int maxSiblings;

    RepastHPCDemoAgentPackage();
    RepastHPCDemoAgentPackage(int id, int rank, int type, int currentRank, int maxSiblings);

    template <class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar &id;
        ar &rank;
        ar &type;
        ar &currentRank;
        ar &maxSiblings;
    }
};

/**
 * For some reason it doesn't work on Model.h
 * Adding #include "repast_hpc/Moore2DGridQuery.h" to Model.cpp breaks it.
 * error: 'Grid' does not name a type
 */
class DataSource_GridCount : public repast::TDataSource<int>
{
private:
    repast::SharedDiscreteSpace<RepastHPCDemoAgent, repast::StrictBorders, repast::SimpleAdder<RepastHPCDemoAgent>> *discreteSpace;
    int i;
    int j;

public:
    DataSource_GridCount(repast::SharedDiscreteSpace<RepastHPCDemoAgent, repast::StrictBorders, repast::SimpleAdder<RepastHPCDemoAgent>> *discreteSpace, int i, int j);
    int getData();
};

#endif