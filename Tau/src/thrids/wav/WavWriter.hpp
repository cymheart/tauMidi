//WavWriter.hpp

#ifndef __WAV_WRITER_HPP__
#define __WAV_WRITER_HPP__


#include <cstdio> //For FILE
#include <cstdint> //For uint8_t, etc.

#include "WavHeader.hpp"



class WavWriter {
    
    public:
 
        WavWriter(); //Number of bytes required to represent the value of a single channel of a sample
        ~WavWriter();
    
        bool initialize(const char* writeFilePath,
                        uint32_t sampleRate,
                        uint32_t numChannels,
                        bool samplesAreInts, //False if samples are 32 or 64-bit floating point values
                        uint32_t byteDepth); //Number of bytes required to represent the value of a single channel of a sample
        
        bool startWriting();

        bool writeData(const uint8_t sampleData[], //WAV format bytes
                       uint32_t sampleDataSize);
    
        bool writeDataFromInt16s(const int16_t int16Samples[], //channels interleaved; length = numInt16Samples * numChannels
                                 uint32_t numInt16Samples);
        
        bool finishWriting(); //Verify, update header's data size field, close file
    
        //Write sample to in-memory wav data array
        bool writeInt16SampleToArray(int16_t int16SampleCh1,
                                     int16_t int16SampleCh2,
                                     uint32_t sampleIndex,
                                     const uint8_t sampleData[], //Wav format bytes; samples interleaved if multiple channels
                                     uint32_t sampleDataSize);
    
        const char* getWriteFilePath();
        uint32_t getSampleRate();
        uint32_t getNumChannels();
        bool getSamplesAreInts();
        uint32_t getByteDepth();
        uint32_t getNumSamplesWritten();
        uint32_t getSampleDataWrittenSize();
             
    
    private:
        bool openFile();
        bool closeFile();
        bool closeFile(const char* errorMessage);
        bool findSubchunk(const char* subchunkId);
    
        const char* writeFilePath;
        FILE* writeFile;
        
        uint32_t sampleRate;
        uint32_t numChannels;
        bool samplesAreInts; //False if samples are floating-point values, 32 or 64-bit
        uint32_t byteDepth; //Number of significant bytes required a single channel of a sample
        bool initialized;
        uint32_t numSamplesWritten;
};



#endif
