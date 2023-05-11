//WavReader.hpp

#ifndef __WAV_READER_HPP__
#define __WAV_READER_HPP__

#include <cstdio> //For FILE
#include <cstdint> //For uint8_t, etc.

#include "WavHeader.hpp"



class WavReader {
    
    public:
    
        WavReader();
        ~ WavReader();
    
        bool initialize(const char* readFilePath);
            
        bool prepareToRead();
    
        bool readData(uint8_t sampleData[], //WAV format bytes
                      uint32_t sampleDataSize); 
    
        bool readDataToInt16s(int16_t int16Samples[], //channels interleaved; length = numInt16Samples * numChannels
                              uint32_t numInt16Samples);
    
        bool finishReading();
    
        //Read int16 sample from an in-memory array of wav-format sample data
        bool readInt16SampleFromArray(const uint8_t sampleData[], //wav-format sample data
                                      uint32_t sampleDataSize,
                                      uint32_t sampleIndex,
                                      int16_t &int16SampleCh1,
                                      int16_t &int16SampleCh2);

        const char* getReadFilePath();
        uint32_t getSampleRate();
        uint32_t getNumSamples();
        uint32_t getNumChannels();
        bool getSamplesAreInts();
        uint32_t getByteDepth();
        uint32_t getSampleDataSize();
        
        
            
    private:
        bool readMetadata();
    
        bool openFile();
        bool closeFile();
        bool closeFile(const char* errorMessage);
    
        bool findSubchunk(const char* subchunkId, uint32_t* subchunkSize);
    
        char* readFilePath;
        FILE* readFile;
        
        //Metadata
        uint32_t sampleRate;
        uint32_t numSamples;
        uint32_t numChannels;
        bool samplesAreInts; //False if samples are 32 or 64-bit floating point values
        uint32_t byteDepth; //Number of significant bytes required to represent a single channel of a sample
        uint32_t sampleDataSize;
        bool initialized;
};



#endif
