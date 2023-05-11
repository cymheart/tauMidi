//WavWriter.cpp
#define _CRT_SECURE_NO_WARNINGS

#include <cstring> //memset()
#include <stdio.h>

#include "WavWriter.hpp"


static const char* UNINITIALIZED_MSG = "Attempt to call WavWriter class method before calling initialize().\n";

static const uint64_t MAX_UINT32 = 4294967295;



WavWriter::WavWriter() {
	initialized = false;
}



WavWriter::~WavWriter() {
}



bool WavWriter::initialize(const char* writeFilePath,
	uint32_t sampleRate,
	uint32_t numChannels,
	bool samplesAreInts, //False if samples are 32 or 64-bit floating point values
	uint32_t byteDepth) {

	//Validate writeFilePath
	FILE* f = fopen(writeFilePath, "w+b");
	if (!f) {
		fprintf(stderr, "Error: Problem testing write-file path (during open).\n");
		return false;
	}
	fclose(f);
	f = nullptr;
	if (remove(writeFilePath) != 0) {
		perror("Error: Problem testing write-file path (during removal)");
		return false;
	}

	//Validate number of channels
	if (!(numChannels == 1 || numChannels == 2)) {
		fprintf(stderr, "Error: Number of channels must be 1 or 2");
		return false;
	}

	//Validate sample rate
	if (sampleRate < 8000) { // Other constraints?
		fprintf(stderr, "Error: Unsupported sample rate.");
		return false;
	}

	//Validate byte depth + int/float combination
	if (!((samplesAreInts && (byteDepth == 1 || byteDepth == 2 || byteDepth == 3 || byteDepth == 4)) ||
		(!samplesAreInts && (byteDepth == 4 || byteDepth == 8)))) {
		fprintf(stderr, "Error: Invalid bits-per-sample value, or invalid combination of bits-per-sample and number of channels.");
		return false;
	}

	//Set member variables
	this->writeFilePath = writeFilePath;
	this->writeFile = nullptr;
	this->sampleRate = sampleRate;
	this->numChannels = numChannels;
	this->samplesAreInts = samplesAreInts;
	this->byteDepth = byteDepth;
	this->initialized = true;
	this->numSamplesWritten = 0;

	return true;
}



bool WavWriter::openFile() {

	if (!initialized) {
		fprintf(stderr, "%s", UNINITIALIZED_MSG);
		return false;
	}

	if (!writeFile) {
		writeFile = fopen(writeFilePath, "w+b");
		if (writeFile == NULL) {
			fprintf(stderr, "Error: Unable to open output file for writing.\n");
			writeFile = nullptr;
			return false;
		}
	}
	else {
		rewind(writeFile);
	}

	return true;
}



bool WavWriter::closeFile() {

	if (!initialized) {
		fprintf(stderr, "%s", UNINITIALIZED_MSG);
		return false;
	}

	return closeFile(nullptr);
}



bool WavWriter::closeFile(const char* errorMessage) {

	if (!initialized) {
		fprintf(stderr, "%s", UNINITIALIZED_MSG);
		return false;
	}

	if (errorMessage) {
		fprintf(stderr, "%s\n", errorMessage);
	}

	if (writeFile) {
		fclose(writeFile);
		writeFile = nullptr;
	}

	return true;
}



bool WavWriter::findSubchunk(const char* subchunkId) {

	if (!initialized) {
		fprintf(stderr, "%s", UNINITIALIZED_MSG);
		return false;
	}

	if (!openFile()) {
		return false;
	}

	//Skip over RIFF header
	if (fseek(writeFile, RIFF_HEADER_SIZE, SEEK_CUR)) {
		closeFile("Error: Problem while skipping over RIFF header.\n");
		return false;
	}

	while (true) {

		size_t numToRead = 1;
		size_t numRead = 0;
		uint8_t subchunkHeaderData[SUBCHUNK_HEADER_SIZE];
		numRead = fread(subchunkHeaderData, SUBCHUNK_HEADER_SIZE, 1, writeFile);
		if (numRead < numToRead) {
			if (feof(writeFile)) {
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
			//Rewind to the beginning of the subchunk, i.e. including the header
			if (fseek(writeFile, -(int)SUBCHUNK_HEADER_SIZE, SEEK_CUR)) {
				fprintf(stderr, "Error: Problem advancing to subchunk: %s\n", subchunkId);
				closeFile();
				return false;
			}
			return true;
		}

		//Subchunk not found; advance to next subchunk
		if (fseek(writeFile, sch->subchunkSize, SEEK_CUR)) {
			if (feof(writeFile)) {
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



//Write functions



bool WavWriter::startWriting() {

	if (!initialized) {
		fprintf(stderr, "%s", UNINITIALIZED_MSG);
		return false;
	}

	if (!openFile()) {
		return false;
	}

	//Write RIFF header
	uint8_t riffHeaderData[RIFF_HEADER_SIZE];
	RiffHeader* rh = (RiffHeader*)riffHeaderData;
	rh->chunkId[0] = 'R';
	rh->chunkId[1] = 'I';
	rh->chunkId[2] = 'F';
	rh->chunkId[3] = 'F';
	rh->fileSizeLess8 = 0; //Unknown at outset; filled upon completion
	rh->formatName[0] = 'W';
	rh->formatName[1] = 'A';
	rh->formatName[2] = 'V';
	rh->formatName[3] = 'E';
	size_t numToWrite = 1;
	size_t numWritten = 0;
	numWritten = fwrite(riffHeaderData, RIFF_HEADER_SIZE, 1, writeFile);
	if (numWritten < numToWrite) {
		closeFile("Error: Problem writing RIFF header.");
		return false;
	}

	//Write format chunk
	uint8_t formatSubchunkData[FORMAT_SUBCHUNK_SIZE];
	FormatSubchunk* fsc = (FormatSubchunk*)formatSubchunkData;
	fsc->formatSubchunkId[0] = 'f';
	fsc->formatSubchunkId[1] = 'm';
	fsc->formatSubchunkId[2] = 't';
	fsc->formatSubchunkId[3] = ' ';
	fsc->formatSubchunkSize = 16;
	fsc->audioFormat = (samplesAreInts) ? AUDIO_FORMAT_INT : AUDIO_FORMAT_FLOAT;
	fsc->numChannels = numChannels;
	fsc->sampleRate = sampleRate;
	fsc->byteRate = sampleRate * numChannels * byteDepth;
	fsc->blockAlign = numChannels * byteDepth;
	fsc->bitsPerSample = byteDepth * 8;
	numToWrite = 1;
	numWritten = 0;
	numWritten = fwrite(formatSubchunkData, FORMAT_SUBCHUNK_SIZE, 1, writeFile);
	if (numWritten < numToWrite) {
		closeFile("Error: Problem writing format subchunk.");
		return false;
	}

	//"fact" subchunk; supposedly required for floating-point representation
	//See: http://www-mmsp.ece.mcgill.ca/Documents/AudioFormats/WAVE/WAVE.html
	if (!samplesAreInts) {
		//Write fact chunk
		uint8_t factSubchunkData[FACT_SUBCHUNK_SIZE];
		FactSubchunk* factsc = (FactSubchunk*)factSubchunkData;
		factsc->factSubchunkId[0] = 'f';
		factsc->factSubchunkId[1] = 'a';
		factsc->factSubchunkId[2] = 'c';
		factsc->factSubchunkId[3] = 't';
		factsc->factSubchunkSize = 4;
		factsc->numSamplesPerChannel = 0; //Unknown at outset; filled upon completion
		numToWrite = 1;
		numWritten = 0;
		numWritten = fwrite(factSubchunkData, FACT_SUBCHUNK_SIZE, 1, writeFile);
		if (numWritten < numToWrite) {
			closeFile("Error: Problem writing fact subchunk.");
			return false;
		}
	}

	uint8_t dataSubchunkHeader[SUBCHUNK_HEADER_SIZE];
	SubchunkHeader* dsh = (SubchunkHeader*)dataSubchunkHeader;
	dsh->subchunkId[0] = 'd';
	dsh->subchunkId[1] = 'a';
	dsh->subchunkId[2] = 't';
	dsh->subchunkId[3] = 'a';
	dsh->subchunkSize = 0; //Unknown at outset; filled upon completion
	numToWrite = 1;
	numWritten = 0;
	numWritten = fwrite(dataSubchunkHeader, SUBCHUNK_HEADER_SIZE, 1, writeFile);
	if (numWritten < numToWrite) {
		closeFile("Error: Problem writing data subchunk header.");
		return false;
	}

	return true;
}



bool WavWriter::writeData(const uint8_t sampleData[], //WAV format bytes
	uint32_t sampleDataSize) {

	if (!initialized) {
		fprintf(stderr, "%s", UNINITIALIZED_MSG);
		return false;
	}

	uint32_t sampleBlockSize = byteDepth * numChannels;
	if (sampleDataSize % sampleBlockSize) {
		fprintf(stderr, "Error: Sample data size doesn't divide evenly by sample block size.\n");
		return false;
	}

	//Requires that:
	// 1) File is open for writing
	// 2) Header has already been written
	// 3) File pointer is at the right location for writing data

	size_t numBytesWritten = 0;
	numBytesWritten = fwrite(sampleData, 1, sampleDataSize, writeFile);

	uint64_t newNumSamplesWritten = (uint64_t)numSamplesWritten +
		(uint64_t)(numBytesWritten / (byteDepth * numChannels));
	if (newNumSamplesWritten > MAX_UINT32) {
		closeFile("Error: Problem writing sample data - overflow.\n");
		return false;
	}
	numSamplesWritten = (uint32_t)newNumSamplesWritten;

	if (numBytesWritten < sampleDataSize) {
		closeFile("Error: Problem writing sample data.\n");
		return false;
	}

	return true;
}



bool WavWriter::writeDataFromInt16s(const int16_t int16Samples[], //channels interleaved; length = numInt16Samples * numChannels
	uint32_t numInt16Samples) {

	if (!initialized) {
		fprintf(stderr, "%s", UNINITIALIZED_MSG);
		return false;
	}


	const uint32_t bufferSize = (numChannels * byteDepth);
	uint8_t* buffer = new uint8_t[bufferSize];
	for (uint32_t i = 0; i < numInt16Samples; i++) {

		int16_t int16SampleCh1 = int16Samples[i * numChannels];
		int16_t int16SampleCh2 = (numChannels == 2) ? int16Samples[i * numChannels + 1] : 0;

		writeInt16SampleToArray(int16SampleCh1,
			int16SampleCh2, //Irrelevant if only one channel
			0, //sampleIndex
			buffer,
			bufferSize);

		bool ok = writeData(buffer, bufferSize);  // Updates numSamplesWritten
		if (!ok) {
			delete[] buffer;
			fprintf(stderr, "Error: Problem while writing data.\n");
			return false;
		}
	}

	delete[] buffer;
	return true;
}



bool WavWriter::finishWriting() {

	if (!initialized) {
		fprintf(stderr, "%s", UNINITIALIZED_MSG);
		return false;
	}

	//Need to update:
	// 1. file length in "RIFF" chunk
	// 2. Subchunk length in data subchunk
	//based on the number of samples actually written.

	if (!openFile()) {
		return false;
	}

	//Advance past "RIFF" chunk ID field
	int numOffsetBytes = 4;
	if (fseek(writeFile, numOffsetBytes, SEEK_CUR) != 0) {
		closeFile("Error advancing to file size.");
		return false;
	}

	//Update RIFF chunk's fileSizeLess8 field
	uint32_t fileSizeLess8 = 4 + //End of riff header
		FORMAT_SUBCHUNK_SIZE + //Format subchunk
		((samplesAreInts) ? 0 : FACT_SUBCHUNK_SIZE) + //Fact subchunk
		(8 + (numSamplesWritten * numChannels * byteDepth)); //Data subchunk - with numSamples actually written
	uint8_t* bytes = (uint8_t*)(&fileSizeLess8);
	size_t numBytesWritten = 0;
	uint32_t numBytesToWrite = sizeof(uint32_t);
	numBytesWritten = fwrite(bytes, 1, numBytesToWrite, writeFile);
	if (numBytesWritten < numBytesToWrite) {
		closeFile("Error: Unable to update riff chunk file length.");
		return false;
	}

	//If floating-point samples...
	if (!samplesAreInts) {

		//Advance to fact subchunk
		if (!findSubchunk("fact")) {
			closeFile("Error: Fact subchunk not found.");
			return false;
		}

		//Update fact chunk - specifically numSamplesPerChannel
		uint8_t factSubchunkData[FACT_SUBCHUNK_SIZE];
		FactSubchunk* factsc = (FactSubchunk*)factSubchunkData;
		factsc->factSubchunkId[0] = 'f';
		factsc->factSubchunkId[1] = 'a';
		factsc->factSubchunkId[2] = 'c';
		factsc->factSubchunkId[3] = 't';
		factsc->factSubchunkSize = 4;
		factsc->numSamplesPerChannel = numSamplesWritten;
		numBytesToWrite = 1;
		numBytesWritten = 0;
		numBytesWritten = fwrite(factSubchunkData, FACT_SUBCHUNK_SIZE, 1, writeFile);
		if (numBytesWritten < numBytesToWrite) {
			closeFile("Error: Problem writing fact subchunk.");
			return false;
		}
	}

	//Advance to data subchunk
	if (!findSubchunk("data")) {
		closeFile("Error: Data subchunk not found.");
		return false;
	}

	//Update data subchunk - specifically to update subchunkSize
	uint8_t dataSubchunkHeader[SUBCHUNK_HEADER_SIZE];
	SubchunkHeader* dsh = (SubchunkHeader*)dataSubchunkHeader;
	dsh->subchunkId[0] = 'd';
	dsh->subchunkId[1] = 'a';
	dsh->subchunkId[2] = 't';
	dsh->subchunkId[3] = 'a';
	dsh->subchunkSize = (numSamplesWritten * numChannels * byteDepth);
	numBytesToWrite = SUBCHUNK_HEADER_SIZE;
	numBytesWritten = fwrite(dataSubchunkHeader, 1, numBytesToWrite, writeFile);
	if (numBytesWritten < numBytesToWrite) {
		perror("Error updating data subchunk header");
		closeFile("Error: Problem updating data subchunk header.");
		return false;
	}

	return (closeFile());
}



bool WavWriter::writeInt16SampleToArray(int16_t int16SampleCh1,
	int16_t int16SampleCh2,
	uint32_t sampleIndex,
	const uint8_t sampleData[], //Wav format bytes; samples interleaved if multiple channels
	uint32_t sampleDataSize) {

	if (!initialized) {
		fprintf(stderr, "%s", UNINITIALIZED_MSG);
		return false;
	}

	uint8_t* srcBytes = new uint8_t[sizeof(int16_t) * numChannels];
	((int16_t*)srcBytes)[0] = int16SampleCh1;
	if (numChannels == 2) {
		((int16_t*)srcBytes)[1] = int16SampleCh2;
	}

	uint8_t* destBytes = (uint8_t*)(sampleData + (sampleIndex * numChannels * byteDepth));
	memcpy(destBytes, srcBytes, (sizeof(int16_t) * numChannels));

	delete[] srcBytes;

	return true;
}



//Accessors



const char* WavWriter::getWriteFilePath() {

	if (!initialized) {
		fprintf(stderr, "%s", UNINITIALIZED_MSG);
		return nullptr;
	}

	return writeFilePath;
}



uint32_t WavWriter::getSampleRate() {

	if (!initialized) {
		fprintf(stderr, "%s", UNINITIALIZED_MSG);
		return false;
	}

	return sampleRate;
}



uint32_t WavWriter::getNumChannels() {

	if (!initialized) {
		fprintf(stderr, "%s", UNINITIALIZED_MSG);
		return false;
	}

	return numChannels;
}



bool WavWriter::getSamplesAreInts() {

	if (!initialized) {
		fprintf(stderr, "%s", UNINITIALIZED_MSG);
		return false;
	}

	return samplesAreInts;
}



uint32_t WavWriter::getByteDepth() {

	if (!initialized) {
		fprintf(stderr, "%s", UNINITIALIZED_MSG);
		return false;
	}

	return byteDepth;
}



uint32_t WavWriter::getNumSamplesWritten() {

	if (!initialized) {
		fprintf(stderr, "%s", UNINITIALIZED_MSG);
		return false;
	}

	return numSamplesWritten;
}



uint32_t WavWriter::getSampleDataWrittenSize() {

	if (!initialized) {
		fprintf(stderr, "%s", UNINITIALIZED_MSG);
		return false;
	}

	return numSamplesWritten * byteDepth * numChannels;
}



