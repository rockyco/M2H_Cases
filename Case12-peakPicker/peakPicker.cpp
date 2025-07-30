#include "peakPicker.hpp"

void peakPicker(
    hls::stream<data_t>& xcorr_stream,
    hls::stream<threshold_t>& threshold_stream,
    hls::stream<location_t>& locations_stream,
    hls::stream<count_t>& count_stream,
    int length
) {
    #pragma HLS PIPELINE II=1
    
    // Circular buffer for sliding window (11 samples)
    data_t window_buffer[WINDOW_LENGTH];
    #pragma HLS ARRAY_PARTITION variable=window_buffer complete
    
    // Threshold buffer to align with window
    threshold_t threshold_buffer[WINDOW_LENGTH];
    #pragma HLS ARRAY_PARTITION variable=threshold_buffer complete
    
    // Peak count
    count_t peak_count = 0;
    
    // Initialize buffers
    INIT_LOOP: for(int i = 0; i < WINDOW_LENGTH; i++) {
        #pragma HLS UNROLL
        window_buffer[i] = 0;
        threshold_buffer[i] = 0;
    }
    
    // Main processing loop - process all input samples
    MAIN_LOOP: for(int index = 0; index < length; index++) {
        #pragma HLS PIPELINE II=1
        
        // Read new sample and threshold
        data_t new_sample = xcorr_stream.read();
        threshold_t new_threshold = threshold_stream.read();
        
        // Shift circular buffer (optimized for zero DSP)
        SHIFT_LOOP: for(int i = WINDOW_LENGTH-1; i > 0; i--) {
            #pragma HLS UNROLL
            window_buffer[i] = window_buffer[i-1];
            threshold_buffer[i] = threshold_buffer[i-1];
        }
        window_buffer[0] = new_sample;
        threshold_buffer[0] = new_threshold;
        
        // Check if we have enough samples for peak detection
        if(index >= WINDOW_LENGTH - 1) {
            location_t candidate_location = index - MIDDLE_INDEX + 1; // Convert to 1-based indexing like MATLAB
            
            // Get middle sample and its threshold
            data_t middle_sample = window_buffer[MIDDLE_INDEX];
            threshold_t middle_threshold = threshold_buffer[MIDDLE_INDEX];
            
            // Check if middle sample exceeds threshold (zero DSP comparison)
            bool threshold_check = (middle_sample >= middle_threshold);
            
            if(threshold_check) {
                // Check if middle sample is maximum in window (zero DSP comparisons)
                bool is_peak = true;
                
                PEAK_CHECK_LOOP: for(int i = 0; i < WINDOW_LENGTH; i++) {
                    #pragma HLS UNROLL
                    if(i != MIDDLE_INDEX) {
                        if(window_buffer[i] > middle_sample) {
                            is_peak = false;
                        }
                    }
                }
                
                // Output peak location if found
                if(is_peak) {
                    locations_stream.write(candidate_location);
                    peak_count++;
                }
            }
        }
    }
    
    // Output final peak count
    count_stream.write(peak_count);
}