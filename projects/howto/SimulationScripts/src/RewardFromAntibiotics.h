/**************************************************************************************
Copyright 2015 Applied Research Associates, Inc.
Licensed under the Apache License, Version 2.0 (the "License"); you may not use
this file except in compliance with the License. You may obtain a copy of the License
at:
http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software distributed under
the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied. See the License for the
specific language governing permissions and limitations under the License.
**************************************************************************************/

#pragma once

#include <string>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream>
#include <CsvUtils.h>
#include <PatientUtils.h>
#include <biogears/cdm/patient/actions/SEConsumeNutrients.h>
#include <biogears/cdm/system/physiology/SEGastrointestinalSystem.h>
#include <biogears/cdm/system/physiology/SECardiovascularSystem.h>
#include <biogears/cdm/system/physiology/SEEnergySystem.h>
#include <biogears/cdm/patient/SENutrition.h>
#include <biogears/cdm/patient/SEPatient.h>
#include <biogears/cdm/properties/SEScalarTypes.h>
#include <biogears/cdm/engine/PhysiologyEngineTrack.h>
#include <biogears/cdm/compartment/SECompartmentManager.h>
#include <biogears/engine/BioGearsPhysiologyEngine.h>
#include <biogears/string/manipulation.h>

#include <biogears/cdm/patient/actions/SEInfection.h>
#include <biogears/cdm/patient/actions/SESubstanceCompoundInfusion.h>
#include <biogears/cdm/patient/actions/SESubstanceInfusion.h>
#include <biogears/cdm/substance/SESubstance.h>
#include <biogears/cdm/substance/SESubstanceCompound.h>
#include <biogears/cdm/substance/SESubstanceManager.h>
#include <map>
#include <mutex>
#include <thread>
#include <future>

namespace biogears {
class SEInfection;
class SEInflammatoryResponse;
class SESubstance;
class SESubstanceInfusion;
class SESubstanceCompoundInfusion;
}

class SepsisSimulation {
private:
	std::mutex mutex;
	std::unique_ptr<biogears::PhysiologyEngine> bg;
	std::thread m_sepsisThread;
public:
	SepsisSimulation() : m_sepsisThread()
	{
        double icustayid = 4201.0;
        std::string mimicdir = "/Users/faaiz/exportdir";
		bg = CreateBioGearsEngine("SimulateMIMIC.log");
		bg->GetEngineTrack()->GetDataRequestManager().CreatePhysiologyDataRequest().Set("HeartRate", FrequencyUnit::Per_min);
		bg->GetEngineTrack()->GetDataRequestManager().CreatePhysiologyDataRequest().Set("BloodVolume", VolumeUnit::mL);
		bg->GetEngineTrack()->GetDataRequestManager().CreatePhysiologyDataRequest().Set("CardiacOutput", VolumePerTimeUnit::mL_Per_min);
		bg->GetEngineTrack()->GetDataRequestManager().CreatePhysiologyDataRequest().Set("MeanArterialPressure", PressureUnit::mmHg);
		bg->GetEngineTrack()->GetDataRequestManager().CreatePhysiologyDataRequest().Set("SystolicArterialPressure", PressureUnit::mmHg);
		bg->GetEngineTrack()->GetDataRequestManager().CreatePhysiologyDataRequest().Set("DiastolicArterialPressure", PressureUnit::mmHg);
		bg->GetEngineTrack()->GetDataRequestManager().CreatePhysiologyDataRequest().Set("StomachContents-Water",VolumeUnit::mL);
			
		bg->GetEngineTrack()->GetDataRequestManager().SetResultsFilename("SimulateMIMIC_" + std::to_string(icustayid) + ".csv");
		// m_sepsisThread = std::thread(&SepsisSimulation::advance_time,this);
        SEPatient pt(bg->GetLogger(), true);
		CsvUtils csvUtils(mimicdir, icustayid);
		
		PatientUtils::create_patient(&pt, &csvUtils, icustayid);
		
		if (!bg->InitializeEngine(pt)) {
			bg->GetLogger()->Error("Could not load state, check the error");
			return;
		}
	}

	void advance_time(int time, TimeUnit unit)
	{
        std::lock_guard<std::mutex> l(mutex);
		// mutex.lock();
		bg->AdvanceModelTime(time, unit);
		// mutex.unlock();
        // return true;
	}

	void administer_vasopressors(double rate, double weight) {
        std::lock_guard<std::mutex> l(mutex);
		SESubstance* norepinephrine = bg->GetSubstanceManager().GetSubstance("Norepinephrine");
		SESubstanceInfusion m_pressor {*norepinephrine};
		m_pressor.GetRate().SetValue(rate*weight, VolumePerTimeUnit::mL_Per_min);
		m_pressor.GetConcentration().SetValue(1.0, MassPerVolumeUnit::ug_Per_mL);
		// mutex.lock();
        // std::future<bool> res = std::async(std::launch::async, [m_pressor, this](){
        //     return bg->ProcessAction(m_pressor);
        // });
		bg->ProcessAction(m_pressor);
        // res.wait();
		// mutex.unlock();
        // return true;
	}

};

void simulate_mimic(const std::string& mimicdir, double icustayid, const std::string& exportdir)
{
    SepsisSimulation sepsisSimulation;
    double time_step = 0;
    CsvUtils csvUtils(mimicdir, icustayid);
    std::map<std::string, std::vector<std::string>> data = csvUtils.get_data();
    double preadm_input = std::stod(data["input_total"].at(0)) - std::stod(data["input_1hourly"].at(0));
    // std::thread vasopressor_thread = std::thread(administer_vasopressors, bg, 0.45, 45.0);
    sepsisSimulation.administer_vasopressors(0.45, 45.0);
    // administer_iv
    double prev_time_step = std::stod(data["charttime"].at(0));
    int rowNum = csvUtils.get_rows();
    for(int i = 0; i < rowNum; i++) {
        double time_step_jump = std::stod(data["charttime"].at(i)) - prev_time_step;
        int t = 0;
        while (t < ((int)time_step_jump/3600)) {
            // consume nutrients
            // std::thread _advance_time;
            sepsisSimulation.advance_time(3600, TimeUnit::s);
            // _advance_time = std::thread(advance_time, bg, 3600, TimeUnit::s);
            time_step += 3600;
            t += 1;
        }
        sepsisSimulation.advance_time(time_step_jump - t*3600, TimeUnit::s);
        // administer vp
        // administer iv
        prev_time_step = std::stod(data["charttime"].at(i));
    }
}

double RewardFromAntibiotics(const std::string& patient_state, double volume, double rate, const std::string& severity);