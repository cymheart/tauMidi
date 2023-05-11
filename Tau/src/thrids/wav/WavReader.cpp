//WavReader.cpp
#define _CRT_SECURE_NO_WARNINGS

#include <cstring> //memset()
# include <cstdlib>
#include <cstdio>

#include "WavReader.hpp"
#include <iostream>
#include <fstream>

using namespace std;

static const uint16_t TWO_POW_7_AS_UINT16 = 128;

static const uint16_t TWO_POW_8_AS_UINT16 = 256;
static const uint32_t TWO_POW_8_AS_UINT32 = 256;

static const float TWO_POW_15_LESS1_AS_FLOAT32 = 32768.0f - 1.0f;
static const double TWO_POW_15_LESS1_AS_FLOAT64 = 32768.0 - 1.0;

static const uint32_t TWO_POW_16_AS_UINT32 = 65536;


static const char* UNINITIALIZED_MSG = "Attempt to call WavReader class method before calling initialize().\n";



WavReader::WavReader() {
	initialized = false;
}



WavReader::~WavReader() {
}



bool WavReader::initialize(const char* readFilePath) {
	//Test for file existence...
	FILE* f = fopen(readFilePath, "r");
	if (!f) {
		fprintf(stderr, "File: %s doesn't exist.\n", readFilePath);
		return false;
	}
	fclose(f);

	//Set member vars

	this->readFilePath = (char*)readFilePath;
	this->readFile = nullptr;

	this->initialized = true; //Set *before* call to readMetadata()
	bool verifies = readMetadata(); //Sets remaining member variables
	this->initialized = verifies; //Update *after* call to readMetadata()

	return verifies;
}



bool WavReader::openFile() {

	if (!initialized) {
		fprintf(stderr, "%s", UNINITIALIZED_MSG);
		return false;
	}

	if (!readFile) {
		readFile = fopen(readFilePath, "rb");
		if (readFile == NULL) {
			fprintf(stderr, "Error: Unable to open input file for reading.\n");
			readFile = nullptr;
			return false;
		}
	}
	else {
		rewind(readFile);
	}

	return true;
}



bool WavReader::closeFile() {

	if (!initialized) {
		fprintf(stderr, "%s", UNINITIALIZED_MSG);
		return false;
	}

	return closeFile(nullptr);
}



bool WavReader::closeFile(const char* errorMessage) {

	if (!initialized) {
		fprintf(stderr, "%s", UNINITIALIZED_MSG);
		return false;
	}

	if (errorMessage) {
		fprintf(stderr, "%s\n", errorMessage);
	}

	if (readFile) {
		fclose(readFile);
		readFile = nullptr;
	}

	return true;
}



bool WavReader::findSubchunk(const char* subchunkId, uint32_t* subchunkSize) {

	if (!initialized) {
		fprintf(stderr, "%s", UNINITIALIZED_MSG);
		return false;
	}

	if (!openFile()) {
		return false;
	}

	//Skip over RIFF header
	if (fseek(readFile, RIFF_HEADER_SIZE, SEEK_CUR)) {
		closeFile("Error: Problem while skipping over RIFF header.\n");
		return false;
	}

	while (true) {

		size_t numToRead = 1;
		size_t numRead = 0;
		uint8_t subchunkHeaderData[SUBCHUNK_HEADER_SIZE];
		numRead = fread(subchunkHeaderData, SUBCHUNK_HEADER_SIZE, 1, readFile);
		if (numRead < numToRead) {
			if (feof(readFile)) {
				fprintf(stderr, "Error: Reached end of file without finding subchunk: %s\n", subchunkId);
				closeFile();
				return false;
			}
			fprintf(stderr, "Error: Problem reading subchunk: %s\n", subchunkId);
			closeFile();
			return false;
		}

		SubchunkHeader* sch = (SubchunkHeader*)subchunkHeaderData;
		bool subchunkFound = !strncmp(sch->subchunkId, subchunkId, 4);
		if (subchunkFound) {
			//Set size to pass back
			*subchunkSize = sch->subchunkSize;
			//Rewind to the beginning of the subchunk, i.e. including the header
			if (fseek(readFile, -(int)SUBCHUNK_HEADER_SIZE, SEEK_CUR)) {
				fprintf(stderr, "Error: Problem advancing to subchunk: %s\n", subchunkId);
				closeFile();
				return false;
			}
			return true;
		}

		//Subchunk not found; advance to next subchunk
		if (fseek(readFile, sch->subchunkSize, SEEK_CUR)) {
			if (feof(readFile)) {
				fprintf(stderr, "Error: End of file reached without finding subchunk: %s\n", subchunkId);
				closeFile();
				return false;
			}
			else {
				closeFile("Error: Problem while advancing to the next subchunk");
				return false;
			}
		}
	}

	return false;
}



bool WavReader::readMetadata() {

	if (!initialized) {
		fprintf(stderr, "%s", UNINITIALIZED_MSG);
		return false;
	}

	if (!openFile()) {
		fprintf(stderr, "Error: Unable to open file to read metadata.\n");
		return false;
	}

	//Read riff header
	uint8_t riffHeaderData[RIFF_HEADER_SIZE];
	size_t numToRead = 1;
	size_t numRead = 0;
	numRead = fread(riffHeaderData, RIFF_HEADER_SIZE, 1, readFile);
	if (numRead < numToRead) {
		closeFile("Error: Problem reading RIFF header.");
		return false;
	}
	RiffHeader* rh = (RiffHeader*)riffHeaderData;
	if (strncmp(rh->chunkId, "RIFF", 4)) {
		closeFile("Error: RIFF header not included at start.");
		return false;
	}

	//Read format subchunk
	uint32_t subchunkSize = 0;
	if (!findSubchunk("fmt ", &subchunkSize)) {
		closeFile("Error: Unable find 'fmt ' subchunk.");
		return false;
	}
	uint8_t formatSubchunkData[FORMAT_SUBCHUNK_SIZE];
	numToRead = 1;
	numRead = 0;
	numRead = fread(formatSubchunkData, FORMAT_SUBCHUNK_SIZE, 1, readFile);
	if (numRead < numToRead) {
		closeFile("Error: Problem reading format subchunk.");
		return false;
	}
	FormatSubchunk* fsc = (FormatSubchunk*)formatSubchunkData;

	//Parse format subchunk
	if (strncmp(fsc->formatSubchunkId, "fmt ", 4)) {
		closeFile("Error: 'fmt ' field not found.");
		return false;
	}

	if (fsc->audioFormat == AUDIO_FORMAT_INT) {
		samplesAreInts = true;
	}
	else if (fsc->audioFormat == AUDIO_FORMAT_FLOAT) {
		samplesAreInts = false;
	}
	else {
		closeFile("Error: Audio format must be WAVE_FORMAT_PCM or WAVE_FORMAT_IEEE_FLOAT.");
		return false;
	}

	numChannels = fsc->numChannels;
	if (!(numChannels == 1 || numChannels == 2)) {
		closeFile("Error: Number of channels must be 1 or 2");
		return false;
	}

	sampleRate = fsc->sampleRate;
	if (sampleRate < 8000) { // Other constraints?
		closeFile("Error: Unsupported sample rate.");
		return false;
	}

	byteDepth = fsc->bitsPerSample / 8;
	if (!((samplesAreInts && (byteDepth == 1 || byteDepth == 2 || byteDepth == 3 || byteDepth == 4)) ||
		(!samplesAreInts && (byteDepth == 4 || byteDepth == 8)))) {
		closeFile("Error: Invalid bits-per-sample value, or invalid combination of bits-per-sample and number of channels.");
		return false;
	}

	subchunkSize = 0;
	if (!findSubchunk("data", &subchunkSize)) {
		closeFile("Error: Data subchunk not found.");
		return false;
	}
	sampleDataSize = subchunkSize;

	if (fsc->blockAlign != numChannels * byteDepth) {
		closeFile("Error: block alignment doesn't match number of channels + bit depth.");
		return false;
	}

	numSamples = sampleDataSize / fsc->blockAlign;

	return true;
}



//Incremental read functions

bool WavReader::prepareToRead() {

	if (!initialized) {
		fprintf(stderr, "%s", UNINITIALIZED_MSG);
		return false;
	}

	//Open file
	if (!openFile()) {
		closeFile("Error: Unable to open file, while preparing to read data.");
		return false;
	}

	//Find data subchunk
	uint32_t subchunkSize = 0;
	if (!findSubchunk("data", &subchunkSize)) {
		closeFile("Error: Unable to find data subchunk, while preparing to read data.");
		return false;
	}

	//Advance past data subchunk header, to the sample data
	if (fseek(readFile, SUBCHUNK_HEADER_SIZE, SEEK_CUR)) {
		closeFile("Error: Unable to advance past data subchunk header.\n");
		return false;
	}

	return true;
}



//Presumes a file opened for binary reading, with file pointer at first byte of sample data
bool WavReader::readData(uint8_t sampleData[],
	uint32_t sampleDataSize) {

	if (!initialized) {
		fprintf(stderr, "%s", UNINITIALIZED_MSG);
		return false;
	}

	if (this->sampleDataSize < sampleDataSize) {
		closeFile("Error: Suppled sampleDataSize larger than available data");
		return false;
	}

	if (sampleDataSize % (byteDepth * numChannels) > 0) {
		closeFile("Error: Suppled sampleDataSize doesn't fall evenly on a sample boundary.");
		return false;
	}

	size_t numToRead = 1;
	size_t numRead = 0;
	numRead = fread((char*)sampleData, 1, sampleDataSize, readFile);
	if (numRead < numToRead) {
		if (feof(readFile)) {
			closeFile("Error: Reached end of file while reading data");
			return false;
		}
		closeFile("Error: Problem reading data");
		return false;
	}

	return true;
}



//Presumes a file opened for binary reading, with file pointer at first byte of sample data
bool WavReader::readDataToInt16s(int16_t int16Samples[], //channels interleaved; length = numInt16Samples * numChannels
	const uint32_t numInt16Samples) {

	if (!initialized) {
		fprintf(stderr, "%s", UNINITIALIZED_MSG);
		return false;
	}

	const uint32_t numBytesToRequest = numInt16Samples * numChannels * byteDepth;
	if (numBytesToRequest > sampleDataSize) {
		closeFile("Error: Suppled numInt16Samples to large for available data");
		return false;
	}

	int16_t sampleCh1 = 0;
	int16_t sampleCh2 = 0;
	const uint32_t numBytes = (numChannels * byteDepth);
	uint8_t* sampleBytes = new uint8_t[numBytes];
	for (uint32_t i = 0; i < numInt16Samples; i++) {
		readData(sampleBytes, numBytes);
		readInt16SampleFromArray(sampleBytes,
			numBytes,
			0, //sampleIndex
			sampleCh1,
			sampleCh2);
		int16Samples[i * numChannels] = sampleCh1;
		if (numChannels == 2) {
			int16Samples[i * numChannels + 1] = sampleCh2;
		}
	}

	delete[] sampleBytes;

	return true;
}



bool WavReader::finishReading() {

	if (!initialized) {
		fprintf(stderr, "%s", UNINITIALIZED_MSG);
		return false;
	}

	if (readFile) {
		fclose(readFile);
		readFile = nullptr;
	}

	return true;
}



//Read sample from in-memory wav data array
bool WavReader::readInt16SampleFromArray(const uint8_t sampleData[],
	uint32_t sampleDataSize,
	uint32_t sampleIndex,
	int16_t& int16SampleCh1,
	int16_t& int16SampleCh2) {

	if (!initialized) {
		fprintf(stderr, "%s", UNINITIALIZED_MSG);
		return false;
	}

	//Verify in bounds
	uint32_t sampleBlockSize = numChannels * byteDepth;
	if (sampleDataSize < ((sampleIndex + 1) * sampleBlockSize)) {
		int16SampleCh1 = 0;
		int16SampleCh2 = 0;
		return false;
	}

	switch (sampleBlockSize) {

	case 1: { //int8 mono
		//NOTE:
		//"There are some inconsistencies in the WAV format:
		//for example, 8-bit data is unsigned while 16-bit data is signed"
		//https://en.wikipedia.org/wiki/WAV
		int16SampleCh1 = ((int16_t)sampleData[sampleIndex] - TWO_POW_7_AS_UINT16) * TWO_POW_8_AS_UINT16;
		int16SampleCh2 = 0;

		break;
	}

	case 2: { //int8 stereo or int16 mono
		if (numChannels == 2) { //8-bit stereo
			//NOTE:
			//"There are some inconsistencies in the WAV format:
			//for example, 8-bit data is unsigned while 16-bit data is signed"
			//https://en.wikipedia.org/wiki/WAV
			uint64_t sampleIndexX2 = sampleIndex * 2;
			int16SampleCh1 = ((int16_t)sampleData[sampleIndexX2] - TWO_POW_7_AS_UINT16) * TWO_POW_8_AS_UINT16;
			int16SampleCh2 = ((int16_t)sampleData[sampleIndexX2 + 1] - TWO_POW_7_AS_UINT16) * TWO_POW_8_AS_UINT16;
		}
		else { //int16 mono
			int16SampleCh1 = ((int16_t*)sampleData)[sampleIndex];
			int16SampleCh2 = 0;
		}

		break;
	}

	case 3: { //int24 mono
		uint64_t sampleIndexX3 = sampleIndex * 3;
		uint32_t ch1 = sampleData[sampleIndexX3 + 2];
		ch1 <<= 8;
		ch1 |= sampleData[sampleIndexX3 + 1];
		ch1 <<= 8;
		ch1 |= sampleData[sampleIndexX3];
		if (0x800000 & ch1) { //If negative...
			ch1 |= 0xFF000000; //Sign extension
		}
		int16SampleCh1 = ((int32_t)ch1) / TWO_POW_8_AS_UINT32;
		int16SampleCh2 = 0;

		break;
	}

	case 4: { //int16 stereo, int32 mono, float32 mono
		if (!samplesAreInts) { //float32 mono
			float ch1float = ((float*)sampleData)[sampleIndex] * TWO_POW_15_LESS1_AS_FLOAT32; //For floats, full scale is 1.0; for int16, 2^15 - 1.
			int16SampleCh1 = ((int16_t)ch1float);
			int16SampleCh2 = 0;
		}
		else if (numChannels == 1) { //int32 mono
			int16SampleCh1 = (((int32_t*)sampleData)[sampleIndex]) / TWO_POW_16_AS_UINT32;
			int16SampleCh2 = 0;
		}
		else { //int16 stereo
			uint64_t sampleIndexX2 = sampleIndex * 2;
			int16SampleCh1 = ((int16_t*)sampleData)[sampleIndexX2];
			int16SampleCh2 = ((int16_t*)sampleData)[sampleIndexX2 + 1];
		}

		break;
	}

	case 6: { //int24 stereo

		uint64_t sampleIndexX6 = sampleIndex * 6;

		uint32_t ch1 = sampleData[sampleIndexX6 + 2];
		ch1 <<= 8;
		ch1 |= sampleData[sampleIndexX6 + 1];
		ch1 <<= 8;
		ch1 |= sampleData[sampleIndexX6];
		if (0x800000 & ch1) { //If negative...
			ch1 |= 0xFF000000; //Sign extension
		}
		int16SampleCh1 = ch1 / TWO_POW_8_AS_UINT32;

		uint32_t ch2 = sampleData[sampleIndexX6 + 5];
		ch2 <<= 8;
		ch2 |= sampleData[sampleIndexX6 + 4];
		ch2 <<= 8;
		ch2 |= sampleData[sampleIndexX6 + 3];
		if (0x800000 & ch2) { //If negative...
			ch2 |= 0xFF000000; //Sign extension
		}
		int16SampleCh2 = ch2 / TWO_POW_8_AS_UINT32;

		break;
	}

	case 8: { //float64 mono, float32 stereo, int32 stereo
		if (!samplesAreInts) { //floating point
			if (numChannels == 1) { //float64 mono
				double ch1float = ((double*)sampleData)[sampleIndex] * TWO_POW_15_LESS1_AS_FLOAT64; //For floats, full scale is 1.0; for int16, 2^15 - 1.
				int16SampleCh1 = ((int16_t)ch1float);
				int16SampleCh2 = 0;
			}
			else { // (numChannels == 2) - float32 stereo
				uint64_t sampleIndexX2 = sampleIndex * 2;
				float ch1float = ((float*)sampleData)[sampleIndexX2] * TWO_POW_15_LESS1_AS_FLOAT32; //For floats, full scale is 1.0; for int16, 2^15 - 1.
				int16SampleCh1 = ((int16_t)ch1float);
				float ch2float = ((float*)sampleData)[sampleIndexX2 + 1] * TWO_POW_15_LESS1_AS_FLOAT32; //For floats, full scale is 1.0; for int16, 2^15 - 1.
				int16SampleCh2 = ((int16_t)ch2float);
			}
		}
		else { //int32 stereo
			uint64_t sampleIndexX2 = sampleIndex * 2;
			int16SampleCh1 = (((int32_t*)sampleData)[sampleIndexX2]) / TWO_POW_16_AS_UINT32;
			int16SampleCh2 = (((int32_t*)sampleData)[sampleIndexX2 + 1]) / TWO_POW_16_AS_UINT32;
		}

		break;
	}

	case 16: { //float64 stereo

		uint64_t sampleIndexX2 = sampleIndex * 2;

		double ch1float = ((double*)sampleData)[sampleIndexX2] * TWO_POW_15_LESS1_AS_FLOAT64; //For floats, full scale is 1.0; for int16, 2^15 - 1.
		int16SampleCh1 = ((int16_t)ch1float);

		double ch2float = ((double*)sampleData)[sampleIndexX2 + 1] * TWO_POW_15_LESS1_AS_FLOAT64; //For floats, full scale is 1.0; for int16, 2^15 - 1.
		int16SampleCh2 = ((int16_t)ch2float);

		break;
	}

	default: { //Error case

		int16SampleCh1 = 0;
		int16SampleCh2 = 0;

		break;
	}
	}

	return true;
}



//Accessors



const char* WavReader::getReadFilePath() {

	if (!initialized) {
		fprintf(stderr, "%s", UNINITIALIZED_MSG);
		return nullptr;
	}

	return readFilePath;
}



uint32_t WavReader::getSampleRate() {

	if (!initialized) {
		fprintf(stderr, "%s", UNINITIALIZED_MSG);
		return false;
	}

	return sampleRate;
}



uint32_t WavReader::getNumSamples() {

	if (!initialized) {
		fprintf(stderr, "%s", UNINITIALIZED_MSG);
		return false;
	}

	return numSamples;
}



uint32_t WavReader::getNumChannels() {

	if (!initialized) {
		fprintf(stderr, "%s", UNINITIALIZED_MSG);
		return false;
	}

	return numChannels;
}



bool WavReader::getSamplesAreInts() {

	if (!initialized) {
		fprintf(stderr, "%s", UNINITIALIZED_MSG);
		return false;
	}

	return samplesAreInts;
}



uint32_t WavReader::getByteDepth() {

	if (!initialized) {
		fprintf(stderr, "%s", UNINITIALIZED_MSG);
		return false;
	}

	return byteDepth;
}



uint32_t WavReader::getSampleDataSize() {

	if (!initialized) {
		fprintf(stderr, "%s", UNINITIALIZED_MSG);
		return false;
	}

	return sampleDataSize;
}


