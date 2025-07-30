#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <cmath>
#include "peakPicker.hpp"
#include "hls_stream.h"

int main() {
    // Input streams
    hls::stream<data_t> xcorr_stream("xcorr_stream");
    hls::stream<threshold_t> threshold_stream("threshold_stream");
    
    // Output streams  
    hls::stream<location_t> locations_stream("locations_stream");
    hls::stream<count_t> count_stream("count_stream");
    
    // Read input cross-correlation data
    std::ifstream xcorr_file("pssCorrMagSq_3_in.txt");
    if (!xcorr_file.is_open()) {
        std::cerr << "Error: Cannot open pssCorrMagSq_3_in.txt" << std::endl;
        return -1;
    }
    
    std::vector<double> xcorr_data;
    double value;
    while (xcorr_file >> value) {
        xcorr_data.push_back(value);
    }
    xcorr_file.close();
    
    // Read threshold data
    std::ifstream threshold_file("threshold_in.txt");
    if (!threshold_file.is_open()) {
        std::cerr << "Error: Cannot open threshold_in.txt" << std::endl;
        return -1;
    }
    
    std::vector<double> threshold_data;
    while (threshold_file >> value) {
        threshold_data.push_back(value);
    }
    threshold_file.close();
    
    // Verify data lengths match
    if (xcorr_data.size() != threshold_data.size()) {
        std::cerr << "Error: Data size mismatch - xcorr: " << xcorr_data.size() 
                  << ", threshold: " << threshold_data.size() << std::endl;
        return -1;
    }
    
    int length = xcorr_data.size();
    std::cout << "Processing " << length << " samples" << std::endl;
    
    // Fill input streams
    for (int i = 0; i < length; i++) {
        xcorr_stream.write(data_t(xcorr_data[i]));
        threshold_stream.write(threshold_t(threshold_data[i]));
    }
    
    // Call the peak picker function
    std::cout << "Running peakPicker..." << std::endl;
    peakPicker(xcorr_stream, threshold_stream, locations_stream, count_stream, length);
    
    // Read results
    count_t peak_count = count_stream.read();
    std::cout << "Number of peaks found: " << peak_count << std::endl;
    
    // Read peak locations
    std::vector<int> peak_locations;
    for (int i = 0; i < peak_count; i++) {
        if (!locations_stream.empty()) {
            location_t loc = locations_stream.read();
            peak_locations.push_back(int(loc));
            std::cout << "Peak " << (i+1) << " at location: " << loc << std::endl;
        }
    }
    
    // Write output to file for comparison
    std::ofstream output_file("peakLocs_out.txt");
    if (output_file.is_open()) {
        for (size_t i = 0; i < peak_locations.size(); i++) {
            output_file << peak_locations[i] << std::endl;
        }
        output_file.close();
        std::cout << "Results written to peakLocs_out.txt" << std::endl;
    }
    
    // Read reference data for comparison
    std::ifstream ref_file("locations_3_ref.txt");
    if (ref_file.is_open()) {
        std::vector<int> ref_locations;
        int ref_loc;
        while (ref_file >> ref_loc) {
            ref_locations.push_back(ref_loc);
        }
        ref_file.close();
        
        // Compare results
        bool match = true;
        if (peak_locations.size() != ref_locations.size()) {
            match = false;
            std::cout << "ERROR: Number of peaks mismatch - found: " << peak_locations.size() 
                      << ", expected: " << ref_locations.size() << std::endl;
        } else {
            for (size_t i = 0; i < peak_locations.size(); i++) {
                if (peak_locations[i] != ref_locations[i]) {
                    match = false;
                    std::cout << "ERROR: Peak " << (i+1) << " mismatch - found: " 
                              << peak_locations[i] << ", expected: " << ref_locations[i] << std::endl;
                }
            }
        }
        
        if (match) {
            std::cout << "SUCCESS: All peaks match reference!" << std::endl;
            return 0;
        } else {
            std::cout << "FAILURE: Peak detection results do not match reference" << std::endl;
            return 1;
        }
    } else {
        std::cout << "Warning: Cannot open reference file for comparison" << std::endl;
        return 0;
    }
}