#include <iostream>
#include <typeinfo>
#include <stdio.h>
#include <stdexcept> // std::runtime_error
#include <SepsisSimulation.h>

void simulate_mimic(const std::string& mimicdir, double icustayid, const std::string& exportdir, bool override_readings=true)
{
    SepsisSimulation sepsisSimulation(mimicdir, icustayid, exportdir);
    double time_step = 0;
    std::map<std::string, std::vector<std::string>> data = sepsisSimulation.get_mimic_data();
    if (override_readings) {
      sepsisSimulation.override_readings(0);
    }
    sepsisSimulation.sepsis_infection(std::stod(data["SOFA"].at(0)));
    double preadm_input = std::stod(data["input_total"].at(0)) - std::stod(data["input_1hourly"].at(0));
    sepsisSimulation.administer_iv(preadm_input, true);
    for (int i = 0; i < std::ceil(preadm_input/750); i++) {
        sepsisSimulation.consume_nutrients(time_step);
        sepsisSimulation.advance_time(3600, TimeUnit::s);
        time_step += 3600;
    }
    double prev_time_step = std::stod(data["charttime"].at(0));
    int rowNum = data["charttime"].size();
    for(int i = 0; i < rowNum; i++) {
        if (override_readings) {
          sepsisSimulation.override_readings(i);
        }
        double time_step_jump = std::stod(data["charttime"].at(i)) - prev_time_step;
        int t = 0;
        while (t < ((int)time_step_jump/3600)) {
            sepsisSimulation.consume_nutrients(time_step);
            sepsisSimulation.advance_time(3600, TimeUnit::s);
            time_step += 3600;
            t += 1;
        }        
        sepsisSimulation.advance_time(time_step_jump - t*3600, TimeUnit::s);
        double o2_frac = std::stod(data["FiO2_1"].at(i));
        sepsisSimulation.action_o2_mask(o2_frac);
        double iv_input = std::stod(data["input_1hourly"].at(i));
        sepsisSimulation.administer_iv(iv_input);
        double vp_rate = std::stod(data["median_dose_vaso"].at(i));
        sepsisSimulation.administer_vasopressors(vp_rate);
        prev_time_step = std::stod(data["charttime"].at(i));
    }
}

int main( int argc, char* argv[] )
{
  if (argc <= 4) {
    simulate_mimic(argv[1], std::stod(argv[2]), argv[3]);
  } else {
    std::stringstream ss(argv[4]);
    bool override;
    if(!(ss >> std::boolalpha >> override)) {
      throw std::runtime_error("Invalid last argument. Must be true or false");
    }
    simulate_mimic(argv[1], std::stod(argv[2]), argv[3], override);
  }
  return 0;
}
