#include <boost/mpi.hpp>
#include <repast_hpc/RepastProcess.h>

#include "model.h"

int main(int argc, char **argv)
{
	std::string configFile = argv[1];
	std::string propsFile = argv[2];

	boost::mpi::environment env(argc, argv);
	boost::mpi::communicator world;

	repast::RepastProcess::init(configFile);

	DengueModel *model = new DengueModel(propsFile, argc, argv, &world);
	repast::ScheduleRunner &runner = repast::RepastProcess::instance()->getScheduleRunner();

	model->init();
	model->initSchedule(runner);

	runner.run();

	delete model;

	repast::RepastProcess::instance()->done();
}
