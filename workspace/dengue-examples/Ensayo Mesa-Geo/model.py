from turtle import ycor
from unittest.mock import patch
from mesa.datacollection import DataCollector
from mesa import Model
from mesa.time import BaseScheduler
from mesa_geo.geoagent import GeoAgent, AgentCreator
from mesa_geo import GeoSpace
from shapely.geometry import Point
import math
import random


########################## AGENTE HUMANOS ##########################
class PersonAgent(GeoAgent):
    """Person Agent."""

    # Define constants
    naturalEmergenceRate = 0.3
    deathRate = 0.071428571428571
    mosquitoCarryingCapacity = 1000
    mosquitoBiteDemand = 0.5
    maxBitesPerHuman = 19
    probabilityOfTransmissionHToM = 0.333
    probabilityOfTransmissionMToH = 0.333
    # FALTAN DISTRIBUCIONES DE TIEMPOS DE INCUBACION

    # Constructor
    def __init__(
        self,
        unique_id,
        model,
        shape,
        infectionState = None,
        age = None,
        timeSinceSuccesfullBite = None,
        timeSinceInfection = None,
        activities = None,
        homeLocation = None,
    ):
        super().__init__(unique_id, model, shape)
        # Initialization susceptibles humans (all human agents initialize as susceptible)
        infectionState = "susceptible"
        timeSinceSuccesfullBite = None
        timeSinceInfection = None
        age = self.ageSetter(random.random())
        activities = []
        homeLocation = [self.shape.x,self.shape.y]

        # Initialization infected humans (a portion of)
        if self.random.random() < self.model.infectedHumans/self.model.totalHumans:
            infectionState = "infected"
            timeSinceSuccesfullBite = 0
            timeSinceInfection = 0
            age = self.ageSetter(random.random())
            activities = []
            homeLocation = [self.shape.x,self.shape.y]

        self.infectionState = infectionState
        self.age = age
        self.timeSinceSuccesfullBite = timeSinceSuccesfullBite
        self.timeSinceInfection = timeSinceInfection
        self.activities = activities
        self.homeLocation = homeLocation

    def move_point(self, dx, dy):
        """
        Move a point by creating a new one
        :param dx:  Distance to move in x-axis
        :param dy:  Distance to move in y-axis
        """
        return Point(self.shape.x + dx, self.shape.y + dy)

    def step(self):
        """Advance one step."""
        for ind_activity in self.activities:
            activity = ind_activity
            x = activity[0]
            y = activity[1]
            self.shape = self.move_point(x,y)
            patches = self.model.grid.get_intersecting_agents(self)
            patch_actual = [
                patch for patch in patches if patch.temperature == 0
            ]
            print(patch_actual)
            self.actualizeSEIRStatus(patches)
            #patch = self.model.grid.get_intersecting_agents(self)
            # susceptible = self.model.grid.m_susceptible
            # exposed = self.model.grid.m_exposed
            # infected = self.model.grid.m_infected
            # temp = self.model.grid.temperature
            # typeZone = self.model.grid.typeZone

            # Se crea un obj tipo patch
            # patch = MyPatch(susceptible, exposed, infected, temp, typeZone)

            # # se recalcula el SEIR la temperatura no cambia porque estamos en el mismo dia 
            # ActualizedSEIR = patch.step(self.model.step,x,y,self.model.grid)
            # patch.m_susceptible = ActualizedSEIR[0]
            # patch.m_exposed = ActualizedSEIR[1]
            # patch.m_infected = ActualizedSEIR[2]
            # patch.total = ActualizedSEIR[3]
        # self.shape = self.move_point(self.homeLocation.shape.x, self.homeLocation.shape.y)
        # actualizeTimes()

    def __repr__(self):
        return "Person " + str(self.unique_id)

    def actualizeTimes(self):
        if self.timeSinceSuccesfullBite != None:
            timeSinceSuccesfullBite = timeSinceSuccesfullBite + 1

        if self.timeSinceInfection != None:
            timeSinceInfection = timeSinceInfection + 1

    def countHumansInPatch(self):
        HumansinP = self.model.grid.get_intersecting_agents(self)
        contH = 0
        for h in HumansinP:
            contH = contH + 1
        return contH

        # SOLO SE USAN PARA ESCRIBIR EL EXCEL FINAL
        # def countTotalSusceptible():
        #     pass

        # def countTotalExposed():
        #     pass

        # def countTotalInfected():
        #     pass

        # def countTotalRecovered():
        #     pass

    def calculateInfectionProbabilityHuman(self, patch):
        #obtener la posicion del patch en el que me encuentro
        patch = patch[0]
        x = patch.shape.x
        y = patch.shape.y
        #patch = self.model.grid.get_intersecting_agents(self)
        susceptibleMosquitoes = patch.m_susceptible
        exposedMosquitoes = patch.m_exposed
        infectedMosquitoes = patch.m_infected
        #x = patch.shape.x
        #y = patch.shape.y

        #obtengo el numero de mosquitos de cada tipo del patch en el que estoy parada
        # susceptibleMosquitoes = patch(0).m_susceptible
        # exposedMosquitoes = patch.m_exposed
        # infectedMosquitoes = patch.m_infected

        #obtengo el numero de humanos que hay en el patch que estoy parada
        #totalHumans = countHumansInPatch(self)
        totalHumans = self.model.grid.agents_at(patch)

        #contar cuantos modquitos hay en el patch que estoy parada
        totalMosquitoes = susceptibleMosquitoes + exposedMosquitoes + infectedMosquitoes

        successfulBitesPerHuman = 0
        if totalHumans > 0:
            totalSuccesfulBites = (self.mosquitoBiteDemand*totalMosquitoes*maxBitesPerHuman*totalHumans)/(mosquitoBiteDemand*totalMosquitoes+maxBitesPerHuman*totalHumans)
            successfulBitesPerHuman = totalSuccesfulBites/totalHumans
        infectionRateHumans = self.probabilityOfTransmissionMToH*successfulBitesPerHuman*infectedMosquitoes/totalMosquitoes
        humanInfectionProbability = 1-math.exp(-infectionRateHumans)
        return humanInfectionProbability

    def actualizeSEIRStatus(self, patch):
        # si la persona esta en estado suceptible se calcula la probabilidad de pasar a infectado y se determina si pasa a infectado o no
        if self.infectionState == "susceptible":
            probabilityIOfInfectionHuman = self.calculateInfectionProbabilityHuman(patch)
            if random.random() <= probabilityIOfInfectionHuman:
                self.infectionState = "exposed"
                self.timeSinceSuccesfullBite = 0

        # si la persona esta en estado expuesto se determina si pasa a infectado
        if self.infectionState == "exposed":
            acumProb = 0 #mirarla
            if random.random() <= acumProb:
                self.infectionState = "infected"
                self.timeSinceInfection = 0

        # si la persona esta infectada se determina si pasa a recuperado
        if self.infectionState == "infected":
            acumProb = 0 #mirarla
            if random.random() <= acumProb:
                self.infectionState = "recovered"

    def ageSetter(self, prob):
        if 0.0811 <= prob and prob < 0.1615:
            return random.randint(5, 9)
        elif 0.1615 <= prob and prob < 0.2435:
            return random.randint(10, 14)
        elif 0.2435 <= prob and prob < 0.3292:
            return random.randint(15, 19)
        elif 0.3292 <= prob and prob < 0.4249:
            return random.randint(20, 24)
        elif 0.4249 <= prob and prob < 0.5196:
            return random.randint(25, 29)
        elif 0.5196 <= prob and prob < 0.6025:
            return random.randint(30, 34)
        elif 0.6025 <= prob and prob < 0.6801:
            return random.randint(35, 39)
        elif 0.6801 <= prob and prob < 0.7506:
            return random.randint(40, 44)
        elif 0.7506 <= prob and prob < 0.8097:
            return random.randint(45, 49)
        elif 0.8097 <= prob and prob < 0.8626:
            return random.randint(50, 54)
        elif 0.8626 <= prob and prob < 0.9062:
            return random.randint(55, 59)
        elif 0.9062 <= prob and prob < 0.9378:
            return random.randint(60, 64)
        elif 0.9378 <= prob and prob < 0.9601:
            return random.randint(65, 69)
        elif 0.9601 <= prob and prob < 0.9768:
            return random.randint(70, 74)
        elif 0.9768 <= prob and prob < 0.9890:
            return random.randint(75, 79)
        elif 0.9890 <= prob and prob <= 1:
            return random.randint(80, 90)
        else:
            return random.randint(0, 4)



########################## AGENTE PATCHES ##########################
class NeighbourhoodAgent(GeoAgent):
    """Neighbourhood agent. Changes color according to number of infected inside it."""

    def __init__(
        self,
        unique_id,
        model,
        shape,
        m_susceptible = None,
        m_exposed = None,
        m_infected = None,
        total = None,
        temperature = None,
        typeZone = None
    ):
        super().__init__(unique_id, model, shape)
        # Initialization patches
        m_susceptible = random.randint(0,100)
        m_exposed = random.randint(0,5)
        m_infected = random.randint(0,5)
        total = m_susceptible + m_exposed + m_infected
        temperature = random.randint(25,30)
        num_rand_aux = random.randint(1,4)
        if num_rand_aux == 1:
            typeZone = "residential"
        elif num_rand_aux == 2:
            typeZone = "study"
        elif num_rand_aux == 3:
            typeZone = "work"
        elif num_rand_aux == 4:
            typeZone = "leisure"

        self.m_susceptible = m_susceptible
        self.m_exposed = m_exposed
        self.m_infected = m_infected
        self.total = total
        self.temperature = temperature
        self.typeZone = typeZone

    def step(self):
        """Advance agent one step."""
        self.temperature = random.randint(25,30)

    def __repr__(self):
        return "Neighborhood " + str(self.unique_id)


########################## SIMULATION BUILDER ##########################
class InfectedModel(Model):
    """Model class for a simplistic infection model."""

    # Geographical parameters for desired map
    MAP_COORDS = [6.333, -75.55]  # Bello
    geojson_regions = "bello_grid.geojson"
    unique_id = "id"

    def __init__(self, simulationTime, totalHumans, infectedHumans):

        self.schedule = BaseScheduler(self)
        self.grid = GeoSpace()
        self.steps = 0
        self.counts = None
        self.reset_counts()

        # SIR model parameters
        self.simulationTime = simulationTime
        self.totalHumans = totalHumans
        self.infectedHumans = infectedHumans
        #self.counts["susceptible"] = totalHumans-infectedHumans
        #self.counts["infected"] = infectedHumans

        self.running = True
        self.datacollector = DataCollector(
            {
                "susceptible": get_susceptible_count,
                "exposed": get_exposed_count,
                "infected": get_infected_count,
                "recovered": get_recovered_count,
            }
        )

        # Set up the Neighbourhood patches for every region in file (add to schedule later)
        AC = AgentCreator(NeighbourhoodAgent, {"model": self})
        neighbourhood_agents = AC.from_file(self.geojson_regions, unique_id=self.unique_id)
        self.grid.add_agents(neighbourhood_agents)
        # From all the neighbourhood agents, create a list with only the residential agents for initialization humans in home location
        neighbourhood_residential_agents = []
        neighbourhood_work_agents = []
        neighbourhood_study_agents = []
        neighbourhood_leisure_agents = []
        for i in range(len(neighbourhood_agents)):
            if neighbourhood_agents[i].typeZone == "residential":
                neighbourhood_residential_agents.append(neighbourhood_agents[i])
            elif neighbourhood_agents[i].typeZone == "work":
                neighbourhood_work_agents.append(neighbourhood_agents[i])
            elif neighbourhood_agents[i].typeZone == "study":
                neighbourhood_study_agents.append(neighbourhood_agents[i])
            elif neighbourhood_agents[i].typeZone == "leisure":
                neighbourhood_leisure_agents.append(neighbourhood_agents[i])

        # Generate PersonAgent population
        ac_population = AgentCreator(PersonAgent, {"model": self})
        # Generate random location in the residential patches, add agent to grid and scheduler
        cont_susc=0
        cont_inf=0
        for i in range(totalHumans):
            # Se le asigna la home location a cada humano
            home_neighbourhood = self.random.randint(0, len(neighbourhood_residential_agents) - 1)  # Region where agent starts
            center_x, center_y = neighbourhood_residential_agents[home_neighbourhood].shape.centroid.coords.xy
            this_bounds = neighbourhood_residential_agents[home_neighbourhood].shape.bounds
            spread_x = int(this_bounds[2] - this_bounds[0])  # Heuristic for agent spread in region
            spread_y = int(this_bounds[3] - this_bounds[1])
            this_x = center_x[0] + self.random.randint(0, spread_x) - spread_x / 2
            this_y = center_y[0] + self.random.randint(0, spread_y) - spread_y / 2
            this_person = ac_population.create_agent(Point(this_x, this_y), "P" + str(i))

            # Se le asigna la actividad de estudio a humanos menores a 24 años
            if this_person.age <= 24:
                study_neighbourhood = self.random.randint(0, len(neighbourhood_study_agents) - 1)
                center_x, center_y = neighbourhood_study_agents[study_neighbourhood].shape.centroid.coords.xy
                this_bounds = neighbourhood_study_agents[study_neighbourhood].shape.bounds
                spread_x = int(this_bounds[2] - this_bounds[0])  # Heuristic for agent spread in region
                spread_y = int(this_bounds[3] - this_bounds[1])
                this_x = center_x[0] + self.random.randint(0, spread_x) - spread_x / 2
                this_y = center_y[0] + self.random.randint(0, spread_y) - spread_y / 2
                this_person.activities.append((this_x,this_y))

            # Se le asigna la actividad de trabajo a humanos mayores a 24 años
            elif this_person.age > 24:
                work_neighbourhood = self.random.randint(0, len(neighbourhood_work_agents) - 1)
                center_x, center_y = neighbourhood_work_agents[work_neighbourhood].shape.centroid.coords.xy
                this_bounds = neighbourhood_work_agents[work_neighbourhood].shape.bounds
                spread_x = int(this_bounds[2] - this_bounds[0])  # Heuristic for agent spread in region
                spread_y = int(this_bounds[3] - this_bounds[1])
                this_x = center_x[0] + self.random.randint(0, spread_x) - spread_x / 2
                this_y = center_y[0] + self.random.randint(0, spread_y) - spread_y / 2
                this_person.activities.append((this_x,this_y))

            # Se le asigna la actividad de ocio a todos los humanos
            leisure_neighbourhood = self.random.randint(0, len(neighbourhood_leisure_agents) - 1)  # Region where agent starts
            center_x, center_y = neighbourhood_leisure_agents[leisure_neighbourhood].shape.centroid.coords.xy
            this_bounds = neighbourhood_leisure_agents[leisure_neighbourhood].shape.bounds
            spread_x = int(this_bounds[2] - this_bounds[0])  # Heuristic for agent spread in region
            spread_y = int(this_bounds[3] - this_bounds[1])
            this_x = center_x[0] + self.random.randint(0, spread_x) - spread_x / 2
            this_y = center_y[0] + self.random.randint(0, spread_y) - spread_y / 2
            this_person.activities.append((this_x,this_y))

            # Se cuentan la cantidad de susceptibles e infectados al inicializar
            if this_person.infectionState == "susceptible":
                cont_susc = cont_susc + 1
            elif this_person.infectionState == "infected":
                cont_inf = cont_inf + 1

            # Se agrega el humano creado a el grid y al schedule
            self.grid.add_agents(this_person)
            self.schedule.add(this_person)

        self.counts["susceptible"] = cont_susc
        self.counts["infected"] = cont_inf

        # Add the neighbourhood agents to schedule AFTER person agents,
        # to allow them to update their color by using BaseScheduler
        for agent in neighbourhood_agents:
            self.schedule.add(agent)

        self.datacollector.collect(self)

    def reset_counts(self):
        self.counts = {
            "susceptible": 0,
            "exposed": 0,
            "infected": 0,
            "recovered": 0,
        }

    def step(self):
        """Run one step of the model."""
        self.steps += 1
        #self.reset_counts()
        self.schedule.step()
        #self.grid._recreate_rtree()  # Recalculate spatial tree, because agents are moving

        self.datacollector.collect(self) # Calcula la cantidad de susc exp inf y recup

        # Run until simulation time is reached
        if self.steps == self.simulationTime:
            self.running = False

# Functions needed for datacollector
def get_susceptible_count(model):
    return model.counts["susceptible"]


def get_exposed_count(model):
    return model.counts["exposed"]


def get_infected_count(model):
    return model.counts["infected"]


def get_recovered_count(model):
    return model.counts["recovered"]



########################## MY PATCH ##########################
class MyPatch(GeoAgent):

    # Define constants
    naturalEmergenceRate = 0.3
    deathRate = 0.071428571428571
    mosquitoCarryingCapacity = 1000
    mosquitoBiteDemand = 0.5
    maxBitesPerHuman = 19
    probabilityOfTransmissionHToM = 0.333
    probabilityOfTransmissionMToH = 0.333

    # Constructor
    def __init__(self, suceptibleMosquitoes: float,
                 exposedMosquitoes: float,
                 infectedMosquitoes: float,
                 temperaturePatch: int,
                 patchType: int):
        self.suceptibleMosquitoes = suceptibleMosquitoes
        self.exposedMosquitoes = exposedMosquitoes
        self.infectedMosquitoes = infectedMosquitoes
        self.temperaturePatch = temperaturePatch
        self.patchType = patchType

    def step(self, x, y):
        self.recalculateSEIR(x, y)
        susc = self.suceptibleMosquitoes
        exp = self.exposedMosquitoes
        inf = self.infectedMosquitoes
        total = susc + exp + inf
        resp = [susc, exp, inf, total]
        return resp

    def recalculateSEIR(self, x, y):
        timeStep=0.1
        self.solveRK4(timeStep, x, y)

    def solveRK4(self, h, x, y):

        s0 = self.suceptibleMosquitoes
        e0 = self.exposedMosquitoes
        i0 = self.infectedMosquitoes

        birthRate = self.calculateBirthRate()
        infectionRate = self.calculateInfectionRate(x,y)

        niter = 1/h
        i=1
        while(i<=niter):
            k1i = h*self.infected_function(s0,e0,i0)
            k1e = h*self.exposed_function(s0,e0,i0,infectionRate)
            k1s = h*self.suceptible_function(s0,e0,i0,birthRate,infectionRate)

            k2i = h*self.infected_function(s0+k1s/2,e0+k1e/2,i0+k1i/2)
            k2e = h*self.exposed_function(s0+k1s/2,e0+k1e/2,i0+k1i/2,infectionRate)
            k2s = h*self.suceptible_function(s0+k1s/2,e0+k1e/2,i0+k1i/2,birthRate,infectionRate)

            k3i = h*self.infected_function(s0+k2s/2,e0+k2e/2,i0+k2i/2)
            k3e = h*self.exposed_function(s0+k2s/2,e0+k2e/2,i0+k2i/2,infectionRate)
            k3s = h*self.suceptible_function(s0+k2s/2,e0+k2e/2,i0+k2i/2,birthRate,infectionRate)

            k4i = h*self.infected_function(s0+k3s,e0+k3e,i0+k3i)
            k4e = h*self.exposed_function(s0+k3s,e0+k3e,i0+k3i,infectionRate)
            k4s = h*self.suceptible_function(s0+k3s,e0+k3e,i0+k3i,birthRate,infectionRate)

            i0 = i0+(k1i+2*k2i+2*k3i+k4i)/6
            self.infectedMosquitoes = i0

            e0 = e0+(k1e+2*k2e+2*k3e+k4e)/6
            self.exposedMosquitoes = e0

            s0 = s0+(k1s+2*k2s+2*k3s+k4s)/6
            self.suceptibleMosquitoes = s0

            birthRate = self.calculateBirthRate()
            infectionRate = self.calculateInfectionRate(x,y)
            i=i+1

    def suceptible_function(self, suceptible, exposed, infected, birthRate, infectionRate):
        s1 = birthRate-infectionRate*suceptible-self.deathRate*suceptible
        return s1

    def exposed_function(self, suceptible, exposed, infected, infectionRate): 
        exposedToinfectedRate = self.calculateExposedToinfectedRate()
        e1 = infectionRate*suceptible-exposedToinfectedRate*exposed-self.deathRate*exposed
        return e1

    def infected_function(self, suceptible, exposed, infected):
        exposedToinfectedRate = self.calculateExposedToinfectedRate()
        i1 = exposedToinfectedRate*exposed-self.deathRate*infected
        return i1

    def calculateBirthRate(self):
        totalMosquitoes = self.suceptibleMosquitoes+self.infectedMosquitoes+self.exposedMosquitoes
        mosquitoPopulationGrowthRate = self.naturalEmergenceRate-self.deathRate
        birthRate = totalMosquitoes*(self.naturalEmergenceRate-mosquitoPopulationGrowthRate*totalMosquitoes/self.mosquitoCarryingCapacity)
        return birthRate

    def calculateInfectionRate(self, x, y):
        totalHumans = self.calculateTotalHumansInPatch(x, y)
        humansInfected = self.calculateInfectedHumansInPatch(x, y)
        totalMosquitoes = self.suceptibleMosquitoes+self.infectedMosquitoes+self.exposedMosquitoes

        totalSuccesfulBites = (self.mosquitoBiteDemand*totalMosquitoes*self.maxBitesPerHuman*totalHumans)/(self.mosquitoBiteDemand*totalMosquitoes+self.maxBitesPerHuman*totalHumans)
        successfulBitesPerMosquito = totalSuccesfulBites/totalMosquitoes
        infectionRateMosquitoes = 0
        if totalHumans>0:
            infectionRateMosquitoes = successfulBitesPerMosquito*self.probabilityOfTransmissionHToM*(humansInfected/totalHumans)
        return infectionRateMosquitoes

    def calculateExposedToinfectedRate(self):
        if self.temperaturePatch<12:
            exposedToInfectedRate = 0
        else:
            patchIncubationPeriod = 4+math.exp(5.15-0.123*self.temperaturePatch)
            exposedToInfectedRate = 1/patchIncubationPeriod
        return exposedToInfectedRate

    # se cuenta la cantidad de humanos en el patch
    def calculateTotalHumansInPatch(self):
        return len(self.model.grid.get_intersecting_agents(self))

    # se cuenta la cantidad de humanos infectados en el patch
    def calculateInfectedHumansInPatch(self):
        neighbors = self.model.grid.get_intersecting_agents(self)
        infected_neighbors = [
            neighbor for neighbor in neighbors if neighbor.infectionState == "infected"
        ]
        return len(infected_neighbors)

    def getPatchTemperature(self):
        temp = random.randint(25,30)
        return temp