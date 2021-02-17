#pragma once
#include <map>
#include <CsvUtils.h>
#include <biogears/cdm/patient/SEPatient.h>
#include <biogears/cdm/properties/SEScalarTypes.h>
#include <biogears/engine/BioGearsPhysiologyEngine.h>

using namespace biogears;

class PatientUtils {
public:
    static void create_patient(SEPatient* patient, const CsvUtils* csvUtils, double icustayid)
    {
        patient->SetName(std::to_string(icustayid));
        //Patient Gender is the only thing that is absolutely required to be set.
        //All value not explicitly set based or standard values or calculations.
        //If you do something out of bounds or set something you're not allowed to, it will alert you with a warning/error.
        std::map<std::string, std::string> first_row = csvUtils->get_first_row();
        CDM::enumSex gender;
        if (std::stod(first_row["gender"]) == 0)
        {
            gender = CDM::enumSex::Male;
        } 
        else
        {
            gender = CDM::enumSex::Female;
        }
        patient->SetGender(gender);
        double age = std::stod(first_row["age"]);
        double weight = std::stod(first_row["Weight_kg"]);
        double hr = std::stod(first_row["HR"]);
        double sysBP = std::stod(first_row["SysBP"]);
        double diaBP = std::stod(first_row["DiaBP"]);
        double rr = std::stod(first_row["RR"]);

        patient->GetAge().SetValue(age, TimeUnit::day);
        patient->GetWeight().SetValue(weight, MassUnit::kg);
        patient->GetHeartRateBaseline().SetValue(hr, FrequencyUnit::Per_min);
        patient->GetSystolicArterialPressureBaseline().SetValue(sysBP, PressureUnit::mmHg);
        patient->GetDiastolicArterialPressureBaseline().SetValue(diaBP, PressureUnit::mmHg);
        patient->GetRespirationRateBaseline().SetValue(rr, FrequencyUnit::Per_min);
    }
};