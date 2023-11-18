#pragma once
#include <cstdint>
#include <guiddef.h>

constexpr size_t RIFF_CHUNK_SIZE = sizeof(uint8_t) * 8 + sizeof(uint32_t);

struct WAVHeader {
    // RIFF descriptor
    uint8_t        riff[4];            // RIFF Header bytes 0-3
    uint32_t       chunk_size;         // WAV Chunk Size  bytes 4-7
    uint8_t        wave[4];            // WAVE Header bytes 8-11

    // fmt sub-chunk
    uint8_t        fmt[4];             // FMT header bytes 12-15
    uint32_t       sub_chunk_1_size;   // Size of the fmt chunk bytes 16-19                     
    uint16_t       audio_format;       // Audio format 1=PCM,6=mulaw,7=alaw, 257=IBM Mu-Law, 258=IBM A-Law, 259=ADPCM bytes 20-21
    uint16_t       num_channels;       // Number of channels 1=Mono 2=Stereo   bytes 22-23           
    uint32_t       sample_rate;        // Sampling Frequency in Hz  bytes 24-27                
    uint32_t       byte_rate;          // bytes per second    bytes 28-31
    uint16_t       block_align;        // 2=16-bit mono, 4=16-bit stereo    bytes 32-33
    uint16_t       bits_per_sample;    // Number of bits per sample   bytes 34-35
    uint16_t       extra_data_size;    // 36-38 if present
    
    // Extra information if present
    uint16_t       valid_bits_per_sample;
    uint32_t       dw_channel_mask;
    GUID           guid;

    struct WAVFactChunk {
        uint8_t fact[4];
        uint32_t fact_ck_size;
        uint32_t total_samples; // per channel (total_samples * samples_rate = length of file in seconds)
    } fact_chunk;

    // Data sub-chunk
    uint8_t        sub_chunk_2_id[4];  // "data"  string   
    uint32_t       sub_chunk_2_size;   // Sample data length IN BYTES
};

