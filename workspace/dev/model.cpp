#include <stdio.h>
#include <vector>
#include <fstream>
#include <iostream>
#include <memory>

#include <boost/mpi.hpp>
#include <repast_hpc/AgentId.h>
#include <repast_hpc/RepastProcess.h>
#include <repast_hpc/Utilities.h>
#include <repast_hpc/Properties.h>
#include <repast_hpc/initialize_random.h>
#include <repast_hpc/SVDataSetBuilder.h>

#include <geos/io/GeoJSON.h>
#include <geos/io/GeoJSONReader.h>
#include <geos/geom/Geometry.h>

#include "model.h"

DengueModel::DengueModel(std::string propsFile, int argc, char **argv, boost::mpi::communicator *comm)
{
	props = new repast::Properties(propsFile, argc, argv, comm);
	stopAt = repast::strToInt(props->getProperty("stop.at"));
	countOfAgents = repast::strToInt(props->getProperty("count.of.agents"));
	initPolygons();
	initializeRandom(*props, comm);
}

DengueModel::~DengueModel()
{
	delete props;
}

void DengueModel::init()
{
	int rank = repast::RepastProcess::instance()->rank();
}

void DengueModel::initPolygons()
{
	std::string modelGridPath = props->getProperty("model.grid");

	std::ifstream ifs(modelGridPath);
	std::string content((std::istreambuf_iterator<char>(ifs)),
						(std::istreambuf_iterator<char>()));

	geos::io::GeoJSONReader reader;
	geos::io::GeoJSONFeatureCollection fc = reader.readFeatures(content);

	for (const auto &feature : fc.getFeatures())
	{
		auto geometry = feature.getGeometry();

		if (geometry->getGeometryTypeId() == geos::geom::GeometryTypeId::GEOS_POLYGON)
		{
			polygons.push_back(geometry);
		}
	}
}

void DengueModel::initSchedule(repast::ScheduleRunner &runner)
{
	runner.scheduleStop(stopAt);
}
