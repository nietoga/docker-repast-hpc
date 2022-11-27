 #include <vector>
 class SEIModel{
    private: 
        //variables de estado de los patches (diferentes en cada patch)
        double suceptibleMosquitoes;
        double exposedMosquitoes;
        double infectedMosquitoes;
        double temperature;
    
    public:
        //Constant variables (they are the same for each patch)
        const double naturalEmergenceRate=0.3;
        const double deathRate=0.071428571428571;
        const  double mosquitoCarryingCapacity=1000;
        const double mosquitoBiteDemand=0.5;
        const double maxBitesPerHuman=19;
        const double probabilityOfTransmissionHToM=0.333;
        //otras variables globales 
        const double probabilityOfTransmissionMToH=0.333;

        //METHODS
        //constructor

        SEIModel(double suceptibleM, double exposedM, double infectedM, double temp); 
       
        //getters
        double getSuceptibleMosquitoes();
        double getExposedMosquitoes();
        double getInfectedMosquitoes();
        double getTemp();
        
        //setters
        void setSuceptibleMosquitoes(double suceptibleM);
        void setExposedMosquitoes(double exposedM);
        void setInfectedMosquitoes(double infectedM);
        void setTemp(double temp);
        
        //most important methods
        void recalculateSEI(double timeStep);
        void actualizeTemp(int tick,std::vector<std::vector<int>> dataTemperatures);
        
        //auxiliary methods
        double suceptible_function(double suceptible,double exposed,double infected,double birthRate, double infectionRate);
        double exposed_function(double suceptible,double exposed,double infected,double infectionRate);
        double infected_function(double suceptible,double exposed,double infected);
        double calculateBirthRate();
        double calculateInfectionRate();
        double calculateExposedToinfectedRate();
        int calculateTotalHumansInPatch();
        int calculateInfectedHumansInPatch();
};