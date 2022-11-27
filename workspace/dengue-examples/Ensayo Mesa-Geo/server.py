from mesa_geo.visualization.ModularVisualization import ModularServer
from mesa.visualization.modules import ChartModule, TextElement
#from mesa.visualization.UserParam import UserSettableParameter
from model import InfectedModel, PersonAgent, NeighbourhoodAgent
from mesa_geo.visualization.MapModule import MapModule


class InfectedText(TextElement):
    """
    Display a text count of how many steps have been taken
    """
    def __init__(self):
        pass

    def render(self, model):
        return "Steps: " + str(model.steps)


model_params = {
   "simulationTime": 364,
#    "totalHumans": 1000,
#    "infectedHumans": 100,
   "totalHumans": 10,
   "infectedHumans": 4,
}


def infected_draw(agent):
    """
    Portrayal Method for canvas
    """
    portrayal = dict()
    if isinstance(agent, PersonAgent):
        portrayal["radius"] = "2"
        if agent.infectionState == "susceptible":
            portrayal["color"] = "Green"
        elif agent.infectionState == "exposed":
            portrayal["color"] = "Yellow"
        elif agent.infectionState == "infected":
            portrayal["color"] = "Red"
        elif agent.infectionState == "recovered":
            portrayal["color"] = "Blue"
    if isinstance(agent, NeighbourhoodAgent):
        portrayal["color"] = "Grey"
    return portrayal


infected_text = InfectedText()
map_element = MapModule(infected_draw, InfectedModel.MAP_COORDS, 10, 500, 500)
infected_chart = ChartModule(
    [
        {"Label": "susceptible", "Color": "Green"},
        {"Label": "exposed", "Color": "Yellow"},
        {"Label": "infected", "Color": "Red"},
        {"Label": "recovered", "Color": "Blue"},
    ]
)
server = ModularServer(
    InfectedModel,
    [map_element, infected_text, infected_chart],
    "Hybrid Agent Based Model in the city of Bello",
    model_params,
)
server.launch()