//WavHeader.hpp

#ifndef __WAV_HEADER_HPP__
#define __WAV_HEADER_HPP__

#include <cstdint>
#include <limits.h> //CHAR_BIT

//See:
// http://soundfile.sapp.org/doc/WaveFormat/
// http://www-mmsp.ece.mcgill.ca/Documents/AudioFormats/WAVE/WAVE.html
// https://sites.google.com/site/musicgapi/technical-documents/wav-file-format#fact
// https://www.recordingblogs.com/wiki/wave-file-format


//Verify we have float32 and float64
static char static_assert_float32[1 - (2 * ((sizeof(float) * CHAR_BIT) != 32))]; //To ensure float is 32 bits
static char static_assert_float64[1 - (2 * ((sizeof(double) * CHAR_BIT) != 64))]; //To ensure double is 64 bits


typedef struct {
	char        chunkId[4];             //"RIFF"
	uint32_t    fileSizeLess8;
	char        formatName[4];          //"WAV"
} RiffHeader;
const uint32_t RIFF_HEADER_SIZE = sizeof(RiffHeader);



typedef struct {
	char        subchunkId[4];
	uint32_t    subchunkSize;           //Number of bytes in subchunk, following this field
} SubchunkHeader;
const uint32_t SUBCHUNK_HEADER_SIZE = sizeof(SubchunkHeader);  //Total size of subchunk



typedef struct {
	char        formatSubchunkId[4];    //"fmt "
	uint32_t    formatSubchunkSize;     //Number of bytes following this field
	uint16_t    audioFormat;            //1 for PCM, 3 for Float
	uint16_t    numChannels;            //1 or 2
	uint32_t    sampleRate;             //8000, 44100, etc.
	uint32_t    byteRate;               // == sampleRate * numChannels * (bitsPerSample / 8)
	uint16_t    blockAlign;             //Bytes per sample, including all channels
	uint16_t    bitsPerSample;          //Bits per sample, for one channel
} FormatSubchunk;
const uint32_t FORMAT_SUBCHUNK_SIZE = sizeof(FormatSubchunk);  //Total size of format subchunk



const uint8_t AUDIO_FORMAT_FLOAT = 3;
const uint8_t AUDIO_FORMAT_INT = 1;



//Supposedly required for IEEE floating-point PCM format; see:
// http://www-mmsp.ece.mcgill.ca/Documents/AudioFormats/WAVE/WAVE.html
typedef struct {
	char        factSubchunkId[4];  //"fact"
	uint32_t    factSubchunkSize;  //Number of bytes following this field
	uint32_t    numSamplesPerChannel;
} FactSubchunk;
const uint32_t FACT_SUBCHUNK_SIZE = sizeof(FactSubchunk); //Total size of fact subchunk



#endif //__WAV_HEADER_HPP__
