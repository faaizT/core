#include <iostream>
#include <typeinfo>
#include <algorithm>
#include <set>
#include <thread>
#include <stdio.h>
#include <stdexcept> // std::runtime_error
#include <biogears/threading/thread_pool.h>
#include <SepsisSimulation.h>

void print_help()
{

  std::cout << "Usage howto-simulationscripts\n"
               "[mimicdir (string)] [icustayid (input \"all\" if simulating for all icustayids)] [exportdir (string)] [until_first_action (boolean)] [<optional> override_readings (boolean)] [<optional> thread_count (int)]\n\n";

  std::cout << "Example: \n";
  std::cout << "../outputs/Release/bin/howto-simulationscripts /Users/faaiz/mimicdir 3042 /Users/faaiz/exportdir true false 4\n";
  std::cout << std::endl;
  exit(0);
}

void simulate_mimic(const std::string& mimicdir, double icustayid, const std::string& exportdir, bool override_readings=false)
{
    SepsisSimulation sepsisSimulation(mimicdir, icustayid, exportdir);
    double time_step = 0;
    std::map<std::string, std::vector<std::string>> data = sepsisSimulation.get_mimic_data();
    if (override_readings) {
      sepsisSimulation.override_readings(0);
    }
    sepsisSimulation.sepsis_infection(std::stod(data["SOFA"].at(0)));
    // double preadm_input = std::stod(data["input_total"].at(0)) - std::stod(data["input_1hourly"].at(0));
    // sepsisSimulation.administer_iv(preadm_input, true);
    // for (int i = 0; i < std::ceil(preadm_input/750); i++) {
    //     sepsisSimulation.consume_nutrients(time_step);
    //     sepsisSimulation.advance_time(3600, TimeUnit::s);
    //     time_step += 3600;
    // }
    double prev_time_step = std::stod(data["charttime"].at(0));
    int rowNum = data["charttime"].size();
    for(int i = 0; i < rowNum; i++) {
        double time_step_jump = std::stod(data["charttime"].at(i)) - prev_time_step;
        int t = 0;
        while (t < ((int)time_step_jump/3600)) {
            sepsisSimulation.consume_nutrients(time_step);
            sepsisSimulation.advance_time(3600, TimeUnit::s);
            time_step += 3600;
            t += 1;
        }        
        sepsisSimulation.advance_time(time_step_jump - t*3600, TimeUnit::s);
        if (override_readings) {
          sepsisSimulation.override_readings(i);
        }
        double o2_frac = std::stod(data["FiO2_1"].at(i));
        sepsisSimulation.action_o2_mask(o2_frac);
        double iv_input = std::stod(data["input_1hourly"].at(i));
        sepsisSimulation.administer_iv(iv_input);
        double vp_rate = std::stod(data["median_dose_vaso"].at(i));
        sepsisSimulation.administer_vasopressors(vp_rate);
        prev_time_step = std::stod(data["charttime"].at(i));
    }
    sepsisSimulation.advance_time(10, TimeUnit::s);
}

void simulate_mimic_until_first_action(const std::string& mimicdir, double icustayid, const std::string& exportdir, bool override_readings=true)
{
  SepsisSimulation sepsisSimulation(mimicdir, icustayid, exportdir);
  double time_step = 0;
  std::map<std::string, std::vector<std::string>> data = sepsisSimulation.get_mimic_data();
  std::vector<std::string> input_total = data["input_total"];
  std::vector<std::string> input_1hourly = data["input_1hourly"];
  std::vector<std::string> median_dose_vaso = data["median_dose_vaso"];
  int last_index = std::min({distance( begin(input_total), find_if( begin(input_total), end(input_total), [](auto x) { return std::stod(x) > 0; })),
  distance( begin(input_1hourly), find_if( begin(input_1hourly), end(input_1hourly), [](auto x) { return std::stod(x) > 0; })),
  distance( begin(median_dose_vaso), find_if( begin(median_dose_vaso), end(median_dose_vaso), [](auto x) { return std::stod(x) > 0; }))});
  if (last_index>0) {
    if (override_readings) {
        sepsisSimulation.override_readings(0);
    }
    sepsisSimulation.sepsis_infection(std::stod(data["SOFA"].at(0)));
    double prev_time_step = std::stod(data["charttime"].at(0));
    for(int i = 0; i < std::min(last_index+1, (int) data["charttime"].size()); i++) {
        double time_step_jump = std::stod(data["charttime"].at(i)) - prev_time_step;
        int t = 0;
        while (t < ((int)time_step_jump/3600)) {
            sepsisSimulation.consume_nutrients(time_step);
            sepsisSimulation.advance_time(1800, TimeUnit::s);
            time_step += 1800;
            if (((float) rand()/RAND_MAX) < 1./3.) {
              sepsisSimulation.urinate();
            }
            if (((int) time_step % 7*1800) == 0 && ((float) rand()/RAND_MAX) < 0.4) {
              sepsisSimulation.generic_exercise(60*15, std::min({(double) rand()/RAND_MAX, 0.4}));
              sepsisSimulation.consume_nutrients(time_step);
              sepsisSimulation.advance_time(60*15, TimeUnit::s);
            } else if (((float) rand()/RAND_MAX) < 0.4) {
              sepsisSimulation.acute_stress();
              sepsisSimulation.advance_time(60*30, TimeUnit::s);
            } else if (((float) rand()/RAND_MAX) < 0.3) {
              sepsisSimulation.pain_stimulus();
              sepsisSimulation.advance_time(60*5, TimeUnit::s);
              sepsisSimulation.administer_morphine();
              sepsisSimulation.advance_time(60*25, TimeUnit::s);
            } else if (((float) rand()/RAND_MAX) < 0.2) {
              sepsisSimulation.asthma_attack(60);
              sepsisSimulation.advance_time(60*29, TimeUnit::s);
            } else if (((float) rand()/RAND_MAX) < 0.2) {
              sepsisSimulation.airway_obstruction(60);
              sepsisSimulation.advance_time(60*29, TimeUnit::s);
            } else if (((float) rand()/RAND_MAX) < 0.1) {
              sepsisSimulation.apnea(0.3);
              sepsisSimulation.advance_time(60*30, TimeUnit::s);
            } else {
              sepsisSimulation.advance_time(60*30, TimeUnit::s);
            }
            time_step += 1800;
            t += 1;
        }        
        sepsisSimulation.advance_time(time_step_jump - t*3600, TimeUnit::s);
        if (override_readings) {
          sepsisSimulation.override_readings(i);
        }
        double o2_frac = std::stod(data["FiO2_1"].at(i));
        sepsisSimulation.action_o2_mask(o2_frac);
        prev_time_step = std::stod(data["charttime"].at(i));
    }
    sepsisSimulation.advance_time(600, TimeUnit::s);  
  }
}

void run_simulation_threads(const std::string& mimicdir, std::set<double> icustayids, const std::string& exportdir, bool until_first_action, bool override_readings=true, unsigned int thread_count=8)
{
  biogears::ThreadPool pool{ thread_count };
  auto channel = pool.get_source();
  std::set<double>::iterator itr;
  if (until_first_action) {
    for (itr = icustayids.begin(); itr != icustayids.end(); itr++) {
      channel->insert([=]() mutable { return simulate_mimic_until_first_action(mimicdir, *itr, exportdir, override_readings); });
    }
  } else {
    for (itr = icustayids.begin(); itr != icustayids.end(); itr++) {
      channel->insert([=]() mutable { return simulate_mimic(mimicdir, *itr, exportdir, override_readings); });
    }
  }
  pool.start();
  while (!pool.stop_if_empty()) {
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(16ms);
  }
  pool.join();
  exit(1);
}

int main( int argc, char* argv[] )
{
  // std::set<double> icustayids;
  // if (argc < 5) {
  //   print_help();
  // }
  // if ((std::string) argv[2] == "all") {
  //   icustayids = CsvUtils::get_icustayids((std::string) argv[1] + "/MIMICtable-1hourly_entire-stay.csv");
  // } else {
  //   icustayids = CsvUtils::get_icustayids((std::string) argv[1]);
  //   int icustayid = *next(icustayids.begin(), std::stoi(argv[2]));
  // }
  // std::stringstream ss(argv[4]);
  // bool until_first_action;
  // if(!(ss >> std::boolalpha >> until_first_action)) {
  //   print_help();
  // }
  // if (argc >= 6) {
  //   std::stringstream ss(argv[5]);
  //   bool override;
  //   if(!(ss >> std::boolalpha >> override)) {
  //     print_help();
  //   }
  //   if (argc >= 7) {
  //     run_simulation_threads(argv[1], icustayids, argv[3], until_first_action, override, std::stoi(argv[6]));
  //   } else {
  //     run_simulation_threads(argv[1], icustayids, argv[3], until_first_action, override);
  //   }
  // } else {
  //   run_simulation_threads(argv[1], icustayids, argv[3], until_first_action);
  // }
  std::set<double> icustayids;
  icustayids = CsvUtils::get_icustayids((std::string) argv[1]);
  int icustayid = *std::next(icustayids.begin(), std::stoi(argv[2]));
  simulate_mimic(argv[1], icustayid, argv[3], false);
  return 0;
}