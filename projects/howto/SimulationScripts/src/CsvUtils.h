#pragma once

#include <string>
#include <set>
#include <fstream>
#include <vector>
#include <map>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream>
#include <iostream>
#include <biogears/string/manipulation.h>


using namespace biogears;

class CsvUtils {
private:
    std::map<std::string, std::vector<std::string>> data;
    std::map<std::string, std::string> first_row;
    std::vector<std::string> columns;

private:    
    void read_csv(std::string filename, double icustayid){
        // Reads a CSV file into a map of <string, vector<int>> pairs where
        // each pair represents <column name, column values>
        
        // Create an input filestream
        std::ifstream myFile(filename);

        // Make sure the file is open
        if(!myFile.is_open()) throw std::runtime_error("Could not open file");

        // Helper vars
        std::string line, colname;
        std::string val;
        int icustayid_idx;

        // Read the column names
        if(myFile.good())
        {
            // Extract the first line in the file
            std::getline(myFile, line);

            std::vector<std::string> cols = biogears::string_split(line, ",");
            if (std::find(cols.begin(), cols.end(), "icustay_id") == cols.end())
            {
                throw std::runtime_error("Column icustay_id not found");
            }
            icustayid_idx = std::find(cols.begin(), cols.end(), "icustay_id") - cols.begin();
            // Extract each column name
            for(std::string colname : cols){
                // Initialize and add <colname, int vector> pairs to result
                columns.push_back(colname);
                data[colname] = {};
            }
        }
        else
        {
            throw std::runtime_error("File not good");
        }
        int rowNum = 0;
        // Read data, line by line
        while(std::getline(myFile, line))
        {        
            // Keep track of the current column index
            int colIdx = 0;
            std::vector<std::string> vals = biogears::string_split(line, ",");
            if (std::stod(vals.at(icustayid_idx)) == icustayid)
            {
                for(std::string val : vals)
                {
                    if (rowNum == 0)
                    {
                        first_row[columns.at(colIdx)] = val;
                    }
                    // Add the current integer to the 'colIdx' column's values vector
                    data[columns.at(colIdx)].push_back(val);
                    // Increment the column index
                    colIdx++;
                }
                rowNum++;
            }
        }

        // Close file
        myFile.close();
    }

public:

    CsvUtils(std::string mimic_dir, double icustayid)
    {
        read_csv(mimic_dir + "/MIMICtable-1hourly_entire-stay.csv", icustayid);
    }

    std::map<std::string, std::vector<std::string>> get_data() const
    {
        return data;
    }

    std::map<std::string, std::string> get_first_row() const
    {
        return first_row;
    }

    int get_rows() const
    {
        if (columns.size() == 0)
        {
            return 0;
        }
        return data.at(columns.at(0)).size();
    }

    void write_csv(std::string filename) const
    {
        // Make a CSV file with one or more columns of integer values
        // Each column of data is represented by the pair <column name, column data>
        //   as std::pair<std::string, std::vector<int>>
        // The dataset is represented as a vector of these columns
        // Note that all columns should be the same size
        
        // Create an output filestream object
        std::ofstream myFile(filename);
        
        // Send column names to the stream
        for(int j = 0; j < columns.size(); ++j)
        {
            myFile << columns.at(j);
            if(j != columns.size() - 1) myFile << ","; // No comma at end of line
        }
        myFile << "\n";
        
        // Send data to the stream
        for(int i = 0; i < data.at(columns.at(0)).size(); ++i)
        {
            for(int j = 0; j < columns.size(); ++j)
            {
                myFile << data.at(columns.at(j)).at(i);
                if(j != columns.size() - 1) myFile << ","; // No comma at end of line
            }
            myFile << "\n";
        }
        
        // Close the file
        myFile.close();
    }

    static std::vector<std::string> get_column(std::vector<std::pair<std::string, std::vector<std::string>>> data , std::string colname)
    {
        std::vector<std::string> column;
        for (int i = 0; i < data.size(); i++) 
        {
            if (colname == data.at(i).first)
            {
                column = data.at(i).second;
            }
        }
        return column;
    }

    static std::set<double> get_icustayids(const std::string& mimic_file_path)
    {
        std::set<double> icustayids;
        std::ifstream myFile(mimic_file_path);

        // Make sure the file is open
        if(!myFile.is_open()) throw std::runtime_error("Could not open file");

        // Helper vars
        std::string line, colname;
        std::string val;
        int icustayid_idx;

        // Read the column names
        if(myFile.good())
        {
            // Extract the first line in the file
            std::getline(myFile, line);

            std::vector<std::string> cols = biogears::string_split(line, ",");
            if (std::find(cols.begin(), cols.end(), "icustay_id") == cols.end())
            {
                throw std::runtime_error("Column icustay_id not found");
            }
            icustayid_idx = std::find(cols.begin(), cols.end(), "icustay_id") - cols.begin();
        }
        else
        {
            throw std::runtime_error("File not good");
        }
        // Read data, line by line
        while(std::getline(myFile, line))
        {        
            std::vector<std::string> vals = biogears::string_split(line, ",");
            icustayids.insert(std::stod(vals.at(icustayid_idx)));
        }
        myFile.close();
        return icustayids;
    }
};
