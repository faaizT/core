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
#include <stdio.h>
#include <time.h>
#include <biogears/cdm/patient/actions/SEConsumeNutrients.h>
#include <biogears/cdm/system/physiology/SEGastrointestinalSystem.h>
#include <biogears/cdm/system/physiology/SECardiovascularSystem.h>
#include <biogears/cdm/system/equipment/Anesthesia/SEAnesthesiaMachine.h>
#include <biogears/cdm/system/equipment/Anesthesia/actions/SEAnesthesiaMachineConfiguration.h>
#include <biogears/cdm/system/physiology/SEEnergySystem.h>
#include <biogears/cdm/patient/SENutrition.h>
#include <biogears/cdm/patient/SEPatient.h>
#include <biogears/cdm/properties/SEScalarTypes.h>
#include <biogears/cdm/engine/PhysiologyEngineTrack.h>
#include <biogears/cdm/patient/actions/SEExercise.h>
#include <biogears/cdm/patient/actions/SEPainStimulus.h>
#include <biogears/cdm/patient/actions/SESubstanceBolus.h>
#include <biogears/cdm/patient/actions/SEAsthmaAttack.h>
#include <biogears/cdm/patient/actions/SEAirwayObstruction.h>
#include <biogears/cdm/patient/actions/SEApnea.h>
#include <biogears/cdm/patient/actions/SEAcuteStress.h>
#include <biogears/cdm/patient/actions/SEUrinate.h>
#include <biogears/cdm/compartment/SECompartmentManager.h>
#include <biogears/engine/BioGearsPhysiologyEngine.h>
#include <biogears/string/manipulation.h>

#include <biogears/cdm/patient/actions/SEInfection.h>
#include <biogears/cdm/patient/actions/SESubstanceCompoundInfusion.h>
#include <biogears/cdm/patient/actions/SESubstanceInfusion.h>
#include <biogears/cdm/substance/SESubstance.h>
#include <biogears/cdm/substance/SESubstanceCompound.h>
#include <biogears/cdm/substance/SESubstanceManager.h>
#include <biogears/cdm/patient/actions/SEOverride.h>
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
    std::map<std::string, std::vector<std::string>> mimic_data;
    const std::string currentDateTime() {
        time_t     now = time(0);
        struct tm  tstruct;
        char       buf[80];
        tstruct = *localtime(&now);
        strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);
        return buf;
    }

public:
	SepsisSimulation(std::string mimicdir, double icustayid, std::string exportdir)
	{
		std::string time_now = currentDateTime();
		CsvUtils csvUtils(mimicdir, icustayid);
		mimic_data = csvUtils.get_data();
		bg = CreateBioGearsEngine("SimulateMIMIC_" + std::to_string((int) icustayid) + "_" + time_now + ".log");
		bg->GetEngineTrack()->GetDataRequestManager().CreatePhysiologyDataRequest().Set("HeartRate", FrequencyUnit::Per_min);
		bg->GetEngineTrack()->GetDataRequestManager().CreatePhysiologyDataRequest().Set("MeanArterialPressure", PressureUnit::mmHg);
		bg->GetEngineTrack()->GetDataRequestManager().CreatePhysiologyDataRequest().Set("SystolicArterialPressure", PressureUnit::mmHg);
		bg->GetEngineTrack()->GetDataRequestManager().CreatePhysiologyDataRequest().Set("DiastolicArterialPressure", PressureUnit::mmHg);
		bg->GetEngineTrack()->GetDataRequestManager().CreatePhysiologyDataRequest().Set("CardiacOutput", VolumePerTimeUnit::L_Per_min);
		bg->GetEngineTrack()->GetDataRequestManager().CreatePhysiologyDataRequest().Set("HemoglobinContent", MassUnit::g);
		bg->GetEngineTrack()->GetDataRequestManager().CreatePhysiologyDataRequest().Set("CentralVenousPressure", PressureUnit::mmHg);
		bg->GetEngineTrack()->GetDataRequestManager().CreatePhysiologyDataRequest().Set("Hematocrit");
		bg->GetEngineTrack()->GetDataRequestManager().CreatePhysiologyDataRequest().Set("ArterialBloodPH");
		bg->GetEngineTrack()->GetDataRequestManager().CreatePhysiologyDataRequest().Set("UrinationRate", VolumePerTimeUnit::mL_Per_hr);
		bg->GetEngineTrack()->GetDataRequestManager().CreatePhysiologyDataRequest().Set("WhiteBloodCellCount", AmountPerVolumeUnit::ct_Per_uL);
		bg->GetEngineTrack()->GetDataRequestManager().CreatePhysiologyDataRequest().Set("UrineProductionRate", VolumePerTimeUnit::mL_Per_min);
		bg->GetEngineTrack()->GetDataRequestManager().CreatePhysiologyDataRequest().Set("RespirationRate", FrequencyUnit::Per_min);
		bg->GetEngineTrack()->GetDataRequestManager().CreatePhysiologyDataRequest().Set("OxygenSaturation");
		bg->GetEngineTrack()->GetDataRequestManager().CreatePhysiologyDataRequest().Set("CarbonDioxideSaturation");
		bg->GetEngineTrack()->GetDataRequestManager().CreatePhysiologyDataRequest().Set("CoreTemperature", TemperatureUnit::C);
		bg->GetEngineTrack()->GetDataRequestManager().CreatePhysiologyDataRequest().Set("SkinTemperature", TemperatureUnit::C);
		bg->GetEngineTrack()->GetDataRequestManager().CreateSubstanceDataRequest().Set(*bg->GetSubstanceManager().GetSubstance("Bicarbonate"), "BloodConcentration", MassPerVolumeUnit::mg_Per_dL);
		bg->GetEngineTrack()->GetDataRequestManager().CreateSubstanceDataRequest().Set(*bg->GetSubstanceManager().GetSubstance("Creatinine"), "BloodConcentration", MassPerVolumeUnit::mg_Per_dL);
		bg->GetEngineTrack()->GetDataRequestManager().CreateSubstanceDataRequest().Set(*bg->GetSubstanceManager().GetSubstance("Lactate"), "BloodConcentration", MassPerVolumeUnit::mg_Per_dL);
		bg->GetEngineTrack()->GetDataRequestManager().CreateSubstanceDataRequest().Set(*bg->GetSubstanceManager().GetSubstance("Piperacillin"), "BloodConcentration", MassPerVolumeUnit::mg_Per_dL);
		bg->GetEngineTrack()->GetDataRequestManager().CreateSubstanceDataRequest().Set(*bg->GetSubstanceManager().GetSubstance("Tazobactam"), "BloodConcentration", MassPerVolumeUnit::mg_Per_dL);
		bg->GetEngineTrack()->GetDataRequestManager().CreateSubstanceDataRequest().Set(*bg->GetSubstanceManager().GetSubstance("Glucose"), "BloodConcentration", MassPerVolumeUnit::mg_Per_dL);
		bg->GetEngineTrack()->GetDataRequestManager().CreateSubstanceDataRequest().Set(*bg->GetSubstanceManager().GetSubstance("Sodium"), "BloodConcentration", AmountPerVolumeUnit::mmol_Per_L);
		bg->GetEngineTrack()->GetDataRequestManager().CreateSubstanceDataRequest().Set(*bg->GetSubstanceManager().GetSubstance("Potassium"), "BloodConcentration", AmountPerVolumeUnit::mmol_Per_L);
		bg->GetEngineTrack()->GetDataRequestManager().CreateSubstanceDataRequest().Set(*bg->GetSubstanceManager().GetSubstance("Chloride"), "BloodConcentration", AmountPerVolumeUnit::mmol_Per_L);
		bg->GetEngineTrack()->GetDataRequestManager().CreateSubstanceDataRequest().Set(*bg->GetSubstanceManager().GetSubstance("Calcium"), "BloodConcentration", AmountPerVolumeUnit::mmol_Per_L);
		bg->GetEngineTrack()->GetDataRequestManager().CreateSubstanceDataRequest().Set(*bg->GetSubstanceManager().GetSubstance("Magnesium"), "BloodConcentration", MassPerVolumeUnit::mg_Per_dL);
		bg->GetEngineTrack()->GetDataRequestManager().CreateSubstanceDataRequest().Set(*bg->GetSubstanceManager().GetSubstance("Albumin"), "BloodConcentration", MassPerVolumeUnit::g_Per_dL);
		bg->GetEngineTrack()->GetDataRequestManager().SetResultsFilename(exportdir + "/SimulateMIMIC_" + std::to_string((int) icustayid) + "_" + time_now + ".csv");
		bg->GetEngineTrack()->GetDataRequestManager().SetSamplesPerSecond(1. / (5. * 60.));
		SEPatient pt(bg->GetLogger(), true);
		PatientUtils::create_patient(&pt, &csvUtils, icustayid);
		
		if (!bg->InitializeEngine(pt)) {
			bg->GetLogger()->Error("Could not load state, check the error");
			return;
		}
		bg->SaveState("./states/" + std::to_string((int) icustayid) + "@0s.xml");
	}

	void advance_time(int time, TimeUnit unit)
	{
		std::lock_guard<std::mutex> l(mutex);
		bg->GetLogger()->Info("Starting to advance time...............................");
		bg->AdvanceModelTime(time, unit);
		bg->GetLogger()->Info("Advanced time by " + std::to_string(time) + unit.GetString());
	}

	void override_readings(int index)
	{
		std::lock_guard<std::mutex> l(mutex);
		SEOverride override;
		override.SetOverrideState(CDM::enumOnOff::On);
		override.SetOverrideConformance(CDM::enumOnOff::On);
		double arterial_ph = std::stod(mimic_data["Arterial_pH"].at(index));
		override.GetArterialPHOverride().SetValue(arterial_ph);
		double wbc_count = std::stod(mimic_data["WBC_count"].at(index))*1000;
		override.GetWBCCountOverride().SetValue(wbc_count, AmountPerVolumeUnit::ct_Per_uL);
		double total_bili = std::stod(mimic_data["Total_bili"].at(index));
		override.GetTotalBilirubinOverride().SetValue(total_bili, MassPerVolumeUnit::mg_Per_dL);
		double glucose_conc = std::stod(mimic_data["Glucose"].at(index));
		override.GetGlucoseConcentrationOverride().SetValue(glucose_conc, MassPerVolumeUnit::mg_Per_dL);
		double lactate_conc = std::stod(mimic_data["Arterial_lactate"].at(index));
		override.GetLactateConcentrationOverride().SetValue(lactate_conc * 89.07, MassPerVolumeUnit::ug_Per_mL);
		double sodium_conc = std::stod(mimic_data["Sodium"].at(index));
		override.GetSodiumConcentrationOverride().SetValue(sodium_conc * 22.9898, MassPerVolumeUnit::ug_Per_mL);
		double potassium_conc = std::stod(mimic_data["Potassium"].at(index));
		override.GetPotassiumConcentrationOverride().SetValue(potassium_conc * 39.10, MassPerVolumeUnit::ug_Per_mL);
		double calcium_conc = std::stod(mimic_data["Ionised_Ca"].at(index));
		override.GetCalciumConcentrationOverride().SetValue(calcium_conc * 40.08, MassPerVolumeUnit::ug_Per_mL);
		double core_temp = std::stod(mimic_data["Temp_C"].at(index));
		override.GetCoreTemperatureOverride().SetValue(core_temp, TemperatureUnit::C);
		double oxygen_saturation = std::stod(mimic_data["paO2"].at(index));
		override.GetO2SaturationOverride().SetValue(oxygen_saturation/100);
		double co2_saturation = std::stod(mimic_data["paCO2"].at(index));
		override.GetCO2SaturationOverride().SetValue(co2_saturation/1000);
		if (override.IsValid()) {
				bg->GetLogger()->Info("Overriding Readings to Copy MIMIC.........................");
				bg->ProcessAction(override);
		}
	}

	void generic_exercise(int time_s, double intensity)
	{
		std::lock_guard<std::mutex> l(mutex);
		SEExercise::SEGeneric ge;
		ge.Intensity.SetValue(intensity);
		SEExercise exG{ ge };
		bg->ProcessAction(exG);
		advance_time(time_s, TimeUnit::s);
		ge.Intensity.SetValue(0.0);
		SEExercise exGStop{ ge };
		bg->ProcessAction(exGStop);
	}

	void urinate()
	{
		std::lock_guard<std::mutex> l(mutex);
		auto relief = biogears::SEUrinate();
  	if (relief.IsValid()) {
			bg->GetLogger()->Info("Patient urinates");
			bg->ProcessAction(relief);
		}
	}

	void pain_stimulus(double severity=0.5)
	{
		std::lock_guard<std::mutex> l(mutex);
		SEPainStimulus PainStimulus; //pain object
		std::string location; //location of pain stimulus, examples "Arm", "Leg"

		//set up the configuration of the pain stimulus
		if (((float) rand()/RAND_MAX) < 0.5) {
			location = "Arm";
		} else {
			location = "Leg";
		}		
		PainStimulus.SetLocation(location);
		PainStimulus.GetSeverity().SetValue(severity);
		bg->ProcessAction(PainStimulus);
	}

	void sepsis_infection(double sofa)
	{
		std::lock_guard<std::mutex> l(mutex);
		double sofa_frac = sofa/24;
		CDM::enumInfectionSeverity::value severity;
		if (sofa_frac < 1.0/3) {
				severity = CDM::enumInfectionSeverity::Mild;
		} else if (sofa_frac < 2.0/3) {
				severity = CDM::enumInfectionSeverity::Moderate;
		} else {
				severity = CDM::enumInfectionSeverity::Severe;
		}
		SEInfection infection {};
		infection.SetSeverity(severity);
		SEScalarMassPerVolume infection_mic;
		infection_mic.SetValue(8.0, MassPerVolumeUnit::g_Per_L);
		infection.GetMinimumInhibitoryConcentration().Set(infection_mic);
		infection.SetLocation("Gut");
		bg->GetLogger()->Info("Initiating infection with severity: " + std::to_string(sofa_frac));
		bg->ProcessAction(infection);
	}

	void asthma_attack(int time_s, double severity=0.5)
	{
		std::lock_guard<std::mutex> l(mutex);
		bg->GetLogger()->Info("Asthma attack...........................");
		SEAsthmaAttack asthmaAttack;
		asthmaAttack.GetSeverity().SetValue(severity);
		bg->ProcessAction(asthmaAttack);
		advance_time(time_s, TimeUnit::s);
		asthmaAttack.GetSeverity().SetValue(0.0);
		bg->ProcessAction(asthmaAttack);
	}

	void airway_obstruction(int time_s, double severity=0.5)
	{
		std::lock_guard<std::mutex> l(mutex);
		SEAirwayObstruction obstruction;
		obstruction.GetSeverity().SetValue(severity);
		bg->ProcessAction(obstruction);
		bg->GetLogger()->Info("Giving the patient an airway obstruction.");
		advance_time(time_s, TimeUnit::s);
		obstruction.GetSeverity().SetValue(0.0);
		bg->ProcessAction(obstruction);
  	bg->GetLogger()->Info("Removing the airway obstruction.");
	}

	void apnea(double severity=0.5)
	{
		std::lock_guard<std::mutex> l(mutex);
		bg->GetLogger()->Info("Patient has apnea.");
		SEApnea apnea;
	  apnea.GetSeverity().SetValue(severity);
  	bg->ProcessAction(apnea);
	}

	void acute_stress(double severity=0.5)
	{
		std::lock_guard<std::mutex> l(mutex);
		bg->GetLogger()->Info("Patient has acute stress.");
		SEAcuteStress acuteStress;
	  acuteStress.GetSeverity().SetValue(severity);
  	bg->ProcessAction(acuteStress);

	}

	void administer_vasopressors(double rate) {
		std::lock_guard<std::mutex> l(mutex);
		double weight = bg->GetPatient().GetWeight(MassUnit::kg);
		SESubstance* norepinephrine = bg->GetSubstanceManager().GetSubstance("Norepinephrine");
		SESubstanceInfusion m_pressor {*norepinephrine};
		m_pressor.GetRate().SetValue(rate*weight, VolumePerTimeUnit::mL_Per_min);
		m_pressor.GetConcentration().SetValue(1.0, MassPerVolumeUnit::ug_Per_mL);
		bg->GetLogger()->Info("Starting to administer vasopressors...........................");
		bg->ProcessAction(m_pressor);
		bg->GetLogger()->Info("Administered vasopressors......................................");
	}

	void administer_iv(double dose, bool preadm=false)
	{
		std::lock_guard<std::mutex> l(mutex);
		SESubstanceCompound* saline = bg->GetSubstanceManager().GetCompound("Saline");
		SESubstanceCompoundInfusion m_saline {*saline};
		m_saline.GetBagVolume().SetValue(dose, VolumeUnit::mL);
		if (preadm) {
				m_saline.GetRate().SetValue(750.0/60, VolumePerTimeUnit::mL_Per_min);
		} else {
				m_saline.GetRate().SetValue(dose/60, VolumePerTimeUnit::mL_Per_min);
		}
		bg->GetLogger()->Info("Starting to administer IV fluids.....................");
		bg->ProcessAction(m_saline);
		bg->GetLogger()->Info("Administered IV fluids................................");
	}

	void administer_morphine(double dose_ml=10.5, double conetration=1000)
	{
		std::lock_guard<std::mutex> l(mutex);
		const SESubstance* morphine = bg->GetSubstanceManager().GetSubstance("Morphine");
		// Create a substance bolus action to administer the substance
		SESubstanceBolus bolus(*morphine);
		bolus.GetConcentration().SetValue(1000, MassPerVolumeUnit::ug_Per_mL);
		bolus.GetDose().SetValue(10.5, VolumeUnit::mL);
		bolus.SetAdminRoute(CDM::enumBolusAdministration::Intravenous);
		bg->GetLogger()->Info("Giving the patient Morphine.............................");
	  bg->ProcessAction(bolus);
	}

	void consume_nutrients(double time_step)
	{
		std::lock_guard<std::mutex> l(mutex);
		SEConsumeNutrients nutrients;
		if (((int) time_step) % 4*3600) {
				bg->GetLogger()->Info("Patient eats food.....................");
				nutrients.GetNutrition().GetCarbohydrate().SetValue(130, MassUnit::g);
				nutrients.GetNutrition().GetFat().SetValue(27, MassUnit::g);
				nutrients.GetNutrition().GetProtein().SetValue(20, MassUnit::g);
				nutrients.GetNutrition().GetCalcium().SetValue(5000, MassUnit::mg);
				nutrients.GetNutrition().GetSodium().SetValue(0.75, MassUnit::g);
				nutrients.GetNutrition().GetWater().SetValue(300, VolumeUnit::mL);            
		} else {
				bg->GetLogger()->Info("Patient drinks water.....................");
				nutrients.GetNutrition().GetWater().SetValue(150, VolumeUnit::mL);
		}
		bg->ProcessAction(nutrients);
	}

	void action_o2_mask(double o2_fraction)
	{
		std::lock_guard<std::mutex> l(mutex);
		auto AMConfig = biogears::SEAnesthesiaMachineConfiguration(bg->GetSubstanceManager());
		auto& config = AMConfig.GetConfiguration();
		config.GetOxygenFraction().SetValue(o2_fraction);
		if (AMConfig.IsValid()) {
				bg->GetLogger()->Info("Using oxygen mask.....................");
				bg->ProcessAction(AMConfig);
		} else {
				bg->GetLogger()->Info("Skipping oxygen mask. Invalid configuration.");
		}
	}

	std::map<std::string, std::vector<std::string>> get_mimic_data() const
	{
		return mimic_data;
	}

};