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



// Include the various types you will be using in your code
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



using namespace biogears;


double RewardFromAntibiotics(const std::string& patient_state, double volume, double rate, const std::string& severity)
{
	return 0;
/***
	// Create the engine and load the patient
	std::unique_ptr<PhysiologyEngine> bg = CreateBioGearsEngine("RewardFromAntibiotics.log");
  	bg->GetLogger()->Info("RewardFromAntibiotics");
	SEPatient patient(bg->GetLogger());
	patient.SetName("HowToCreateAPatient");
	//Patient Gender is the only thing that is absolutely required to be set.
	//All value not explicitly set based or standard values or calculations.
	//If you do something out of bounds or set something you're not allowed to, it will alert you with a warning/error.
	patient.SetGender(CDM::enumSex::Male);
	patient.GetAge().SetValue(55, TimeUnit::yr);
	patient.GetWeight().SetValue(170, MassUnit::lb);
	patient.GetHeight().SetValue(71, LengthUnit::in);
	patient.GetBodyFatFraction().SetValue(0.21);
	patient.GetDiastolicArterialPressureBaseline().SetValue(65, PressureUnit::mmHg);
	patient.GetHeartRateBaseline().SetValue(72, FrequencyUnit::Per_min);
	patient.GetRespirationRateBaseline().SetValue(16, FrequencyUnit::Per_min);
	patient.GetSystolicArterialPressureBaseline().SetValue(114, PressureUnit::mmHg);
	if (!bg->InitializeEngine(patient)) {
    	bg->GetLogger()->Error("Could not load state, check the error");
    	return -1000.0;
  	}
	// Create data requests for each value that should be written to the output log as the engine is executing
	// Physiology System Names are defined on the System Objects
	// defined in the Physiology.xsd file
  	bg->GetEngineTrack()->GetDataRequestManager().CreatePhysiologyDataRequest().Set("HeartRate", FrequencyUnit::Per_min);
	bg->GetEngineTrack()->GetDataRequestManager().CreatePhysiologyDataRequest().Set("BloodVolume", VolumeUnit::mL);
	bg->GetEngineTrack()->GetDataRequestManager().CreatePhysiologyDataRequest().Set("CardiacOutput", VolumePerTimeUnit::mL_Per_min);
	bg->GetEngineTrack()->GetDataRequestManager().CreatePhysiologyDataRequest().Set("MeanArterialPressure", PressureUnit::mmHg);
	bg->GetEngineTrack()->GetDataRequestManager().CreatePhysiologyDataRequest().Set("SystolicArterialPressure", PressureUnit::mmHg);
	bg->GetEngineTrack()->GetDataRequestManager().CreatePhysiologyDataRequest().Set("DiastolicArterialPressure", PressureUnit::mmHg);
	bg->GetEngineTrack()->GetDataRequestManager().CreatePhysiologyDataRequest().Set("StomachContents-Water",VolumeUnit::mL);
		
	bg->GetEngineTrack()->GetDataRequestManager().SetResultsFilename("HowToSimulationScripts_" + severity + "_2.csv");

	bg->GetLogger()->Info("READINGS BEFORE ANTIBIOTIC IS ADMINISTERED");
	bg->GetLogger()->Info(std::stringstream() <<"Cardiac Output : " << bg->GetCardiovascularSystem()->GetCardiacOutput(VolumePerTimeUnit::mL_Per_min) << VolumePerTimeUnit::mL_Per_min);
	bg->GetLogger()->Info(std::stringstream() <<"Blood Volume : " << bg->GetCardiovascularSystem()->GetBloodVolume(VolumeUnit::mL) << VolumeUnit::mL);
	bg->GetLogger()->Info(std::stringstream() <<"Mean Arterial Pressure : " << bg->GetCardiovascularSystem()->GetMeanArterialPressure(PressureUnit::mmHg) << PressureUnit::mmHg);
	bg->GetLogger()->Info(std::stringstream() <<"Systolic Pressure : " << bg->GetCardiovascularSystem()->GetSystolicArterialPressure(PressureUnit::mmHg) << PressureUnit::mmHg);
	bg->GetLogger()->Info(std::stringstream() <<"Diastolic Pressure : " << bg->GetCardiovascularSystem()->GetDiastolicArterialPressure(PressureUnit::mmHg) << PressureUnit::mmHg);
	bg->GetLogger()->Info(std::stringstream() <<"Heart Rate : " << bg->GetCardiovascularSystem()->GetHeartRate(FrequencyUnit::Per_min) << "bpm");
	bg->GetLogger()->Info(std::stringstream() <<"Stomach Water : " << bg->GetGastrointestinalSystem()->GetStomachContents()->GetWater(VolumeUnit::mL) << VolumeUnit::mL);

	// Advance some time to get some resting data
	bg->AdvanceModelTime(50, TimeUnit::s);
	CDM::enumInfectionSeverity::value infection_severity;

	if (severity == "mild") 
	{
		infection_severity = CDM::enumInfectionSeverity::value::Mild;
	} 
	else if (severity == "moderate") 
	{
		infection_severity = CDM::enumInfectionSeverity::value::Moderate;
	}
	else if (severity == "severe") 
	{
		infection_severity = CDM::enumInfectionSeverity::value::Severe;
	}

	if (severity != "none")
	{
		SEInfection infection {};
		infection.SetSeverity(infection_severity);
		SEScalarMassPerVolume infection_mic;
		infection_mic.SetValue(8.0, MassPerVolumeUnit::g_Per_L);
		infection.GetMinimumInhibitoryConcentration().Set(infection_mic);
		infection.SetLocation("Gut");
		bg->ProcessAction(infection); 
	}
	bg->AdvanceModelTime(50, TimeUnit::s);
	SESubstanceCompound* antibiotic = bg->GetSubstanceManager().GetCompound("PiperacillinTazobactam");
	biogears::SESubstanceCompoundInfusion* m_antibiotic = new SESubstanceCompoundInfusion(*antibiotic);
	m_antibiotic->GetBagVolume().SetValue(volume, VolumeUnit::mL);
	m_antibiotic->GetRate().SetValue(rate, VolumePerTimeUnit::mL_Per_min);
	bg->ProcessAction(*m_antibiotic);
	bg->AdvanceModelTime(1, TimeUnit::hr);
	bg->GetLogger()->Info("READINGS AFTER ANTIBIOTICS ARE ADMINISTERED");
	bg->GetLogger()->Info(std::stringstream() <<"Cardiac Output : " << bg->GetCardiovascularSystem()->GetCardiacOutput(VolumePerTimeUnit::mL_Per_min) << VolumePerTimeUnit::mL_Per_min);
	bg->GetLogger()->Info(std::stringstream() <<"Blood Volume : " << bg->GetCardiovascularSystem()->GetBloodVolume(VolumeUnit::mL) << VolumeUnit::mL);
	bg->GetLogger()->Info(std::stringstream() <<"Mean Arterial Pressure : " << bg->GetCardiovascularSystem()->GetMeanArterialPressure(PressureUnit::mmHg) << PressureUnit::mmHg);
	bg->GetLogger()->Info(std::stringstream() <<"Systolic Pressure : " << bg->GetCardiovascularSystem()->GetSystolicArterialPressure(PressureUnit::mmHg) << PressureUnit::mmHg);
	bg->GetLogger()->Info(std::stringstream() <<"Diastolic Pressure : " << bg->GetCardiovascularSystem()->GetDiastolicArterialPressure(PressureUnit::mmHg) << PressureUnit::mmHg);
	bg->GetLogger()->Info(std::stringstream() <<"Heart Rate : " << bg->GetCardiovascularSystem()->GetHeartRate(FrequencyUnit::Per_min) << "bpm");
	bg->GetLogger()->Info(std::stringstream() <<"Stomach Water : " << bg->GetGastrointestinalSystem()->GetStomachContents()->GetWater(VolumeUnit::mL) << VolumeUnit::mL);
	double reward = (bg->GetCardiovascularSystem()->GetHeartRate(FrequencyUnit::Per_min) > 55) && (bg->GetCardiovascularSystem()->GetHeartRate(FrequencyUnit::Per_min) < 100) ? 1.0 : -1.0 ;

	bg->GetLogger()->Info("Reward of administering the antibiotic: " + std::to_string(reward));
	bg->GetLogger()->Info("Finished");
	return reward;
*/

}