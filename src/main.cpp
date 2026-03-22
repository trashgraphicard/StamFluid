#include "config.hpp"
#include "utils.hpp"
#include "field.hpp"
#include "fluid_sim.hpp"

int main(){
   int dimension[2] = {96, 72};
   int resolution[2] = {1440, 1080};
   FluidSim sim = FluidSim(dimension, resolution);
   sim.run();
   return 0;
}