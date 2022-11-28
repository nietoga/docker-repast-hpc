#pragma once

#include <vector>

#include <boost/mpi.hpp>
#include <repast_hpc/Schedule.h>
#include <repast_hpc/Properties.h>
#include <repast_hpc/SharedContext.h>
#include <repast_hpc/AgentRequest.h>
#include <repast_hpc/TDataSource.h>
#include <repast_hpc/SVDataSet.h>

#include <geos/geom/Geometry.h>

class DengueModel
{
	int stopAt;
	int countOfAgents;
	repast::Properties *props;
	std::vector<geos::geom::Geometry const *> polygons;

	void initPolygons();

public:
	DengueModel(std::string propsFile, int argc, char **argv, boost::mpi::communicator *comm);
	~DengueModel();
	void init();
	void initSchedule(repast::ScheduleRunner &runner);
};
