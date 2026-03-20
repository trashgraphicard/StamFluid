#include "config.h"
#include "utils.h"
#include "field.h"
#include "fluid_sim.h"

int main(){
   int dimension[2] = {72, 54};
   int resolution[2] = {1440, 1080};
   FluidSim sim = FluidSim(dimension, resolution);
   sim.run();
}