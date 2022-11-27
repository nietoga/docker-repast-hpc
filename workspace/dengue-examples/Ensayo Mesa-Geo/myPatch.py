import math
import random

class MyPatch():
    
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
        
    def recalculateSEIR(x, y):
        timeStep=0.1
        solveRK4(timeStep, x, y)
        
    def solveRK4(h, x, y):

        s0 = self.suceptibleMosquitoes
        e0 = self.exposedMosquitoes
        i0 = self.infectedMosquitoes
        
        birthRate = calculateBirthRate()
        infectionRate = calculateInfectionRate(x,y)
        
        niter = 1/h        
        i=1
        while(i<=niter):
            k1i = h*infected_function(s0,e0,i0)
            k1e = h*exposed_function(s0,e0,i0,infectionRate)
            k1s = h*suceptible_function(s0,e0,i0,birthRate,infectionRate)
            
            k2i = h*infected_function(s0+k1s/2,e0+k1e/2,i0+k1i/2)
            k2e = h*exposed_function(s0+k1s/2,e0+k1e/2,i0+k1i/2,infectionRate)
            k2s = h*suceptible_function(s0+k1s/2,e0+k1e/2,i0+k1i/2,birthRate,infectionRate)
            
            k3i = h*infected_function(s0+k2s/2,e0+k2e/2,i0+k2i/2)
            k3e = h*exposed_function(s0+k2s/2,e0+k2e/2,i0+k2i/2,infectionRate)
            k3s = h*suceptible_function(s0+k2s/2,e0+k2e/2,i0+k2i/2,birthRate,infectionRate)
            
            k4i = h*infected_function(s0+k3s,e0+k3e,i0+k3i)
            k4e = h*exposed_function(s0+k3s,e0+k3e,i0+k3i,infectionRate)
            k4s = h*suceptible_function(s0+k3s,e0+k3e,i0+k3i,birthRate,infectionRate)
            
            i0 = i0+(k1i+2*k2i+2*k3i+k4i)/6
            self.infectedMosquitoes = i0
            
            e0 = e0+(k1e+2*k2e+2*k3e+k4e)/6
            self.exposedMosquitoes = e0

            s0 = s0+(k1s+2*k2s+2*k3s+k4s)/6
            self.suceptibleMosquitoes = s0
            
            birthRate = calculateBirthRate()
            infectionRate = calculateInfectionRate(x,y)
            i=i+1
            
    def suceptible_function(suceptible, exposed, infected, birthRate, infectionRate):
        s1 = birthRate-infectionRate*suceptible-deathRate*suceptible
        return s1
    
    def exposed_function(suceptible, exposed, infected, infectionRate): 
        exposedToinfectedRate = calculateExposedToinfectedRate()
        e1 = infectionRate*suceptible-exposedToinfectedRate*exposed-deathRate*exposed
        return e1
    
    def infected_function(suceptible, exposed, infected):
        exposedToinfectedRate = calculateExposedToinfectedRate()
        i1 = exposedToinfectedRate*exposed-deathRate*infected
        return i1
    
    def calculateBirthRate():
        totalMosquitoes = self.suceptibleMosquitoes+self.infectedMosquitoes+self.exposedMosquitoes
        mosquitoPopulationGrowthRate = naturalEmergenceRate-deathRate
        birthRate = totalMosquitoes*(naturalEmergenceRate-mosquitoPopulationGrowthRate*totalMosquitoes/mosquitoCarryingCapacity)
        return birthRate

    def calculateInfectionRate(x, y):
        totalHumans = calculateTotalHumansInPatch(x, y)
        humansInfected = calculateInfectedHumansInPatch(x, y)
        totalMosquitoes = self.suceptibleMosquitoes+self.infectedMosquitoes+self.exposedMosquitoes
        
        totalSuccesfulBites = (mosquitoBiteDemand*totalMosquitoes*maxBitesPerHuman*totalHumans)/(mosquitoBiteDemand*totalMosquitoes+maxBitesPerHuman*totalHumans)
        successfulBitesPerMosquito = totalSuccesfulBites/totalMosquitoes
        infectionRateMosquitoes = 0
        if totalHumans>0:
            infectionRateMosquitoes = successfulBitesPerMosquito*probabilityOfTransmissionHToM*(humansInfected/totalHumans)
        return infectionRateMosquitoes
    
    def calculateExposedToinfectedRate():
        if self.temperaturePatch<12:
            exposedToInfectedRate = 0
        else:
            patchIncubationPeriod = 4+math.exp(5.15-0.123*self.temperaturePatch)
            exposedToInfectedRate = 1/patchIncubationPeriod
        return exposedToinfectedRate

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
