#pragma once

#include <boost/mpi.hpp>
#include <repast_hpc/Schedule.h>
#include <repast_hpc/Properties.h>
#include <repast_hpc/SharedContext.h>
#include <repast_hpc/AgentRequest.h>
#include <repast_hpc/TDataSource.h>
#include <repast_hpc/SVDataSet.h>

class DengueModel
{
	int stopAt;
	int countOfAgents;
	repast::Properties *props;

public:
	DengueModel(std::string propsFile, int argc, char **argv, boost::mpi::communicator *comm);
	~DengueModel();
	void init();
	void initSchedule(repast::ScheduleRunner &runner);
};
