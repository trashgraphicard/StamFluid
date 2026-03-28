#include "config.hpp"
#include "utils.hpp"
#include "fluid_sim.hpp"
#include "brush.hpp"

int main(){
   int dimension[2] = {96, 72};
   int resolution[2] = {1440, 1080};
   FluidSim sim = FluidSim(dimension, resolution);
   sim.run_bad_apple();



   //int dimension[2] = {128, 128};
   //int resolution[2] = {720, 720};
   //FluidSim sim = FluidSim(dimension, resolution);
   //sim.run_brush();
   //return 0;
}