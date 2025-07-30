#ifndef PEAKPICKER_HPP
#define PEAKPICKER_HPP

#include "ap_int.h"
#include "ap_fixed.h"
#include "hls_stream.h"

// Configuration parameters
#define WINDOW_LENGTH 11
#define MIDDLE_INDEX 5  // floor(11/2)
#define MAX_PEAKS 100   // Maximum number of peaks expected

// Data types optimized for zero DSP usage
typedef ap_fixed<24,8> data_t;      // Cross-correlation data type
typedef ap_fixed<24,8> threshold_t; // Threshold data type  
typedef ap_uint<16> location_t;     // Peak location index
typedef ap_uint<8> count_t;         // Peak count

// Streaming peak picker function
void peakPicker(
    hls::stream<data_t>& xcorr_stream,
    hls::stream<threshold_t>& threshold_stream,
    hls::stream<location_t>& locations_stream,
    hls::stream<count_t>& count_stream,
    int length
);

#endif // PEAKPICKER_HPP