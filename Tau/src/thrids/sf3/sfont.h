//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2010 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#ifndef SFONT_H
#define SFONT_H

#include <cstdint>
#include<vector>
#include <string>
#include "scutils/Utils.h"
#include "scutils/ByteStream.h"
#include "scutils/LinkedList.h"

using namespace scutils;
using namespace std;

namespace SfTools {

//---------------------------------------------------------
//   sfVersionTag
//---------------------------------------------------------

struct sfVersionTag {
    int major;
    int minor;
};

enum Modulator {
};
enum Generator {
    Gen_StartAddrOfs, Gen_EndAddrOfs, Gen_StartLoopAddrOfs,
    Gen_EndLoopAddrOfs, Gen_StartAddrCoarseOfs, Gen_ModLFO2Pitch,
    Gen_VibLFO2Pitch, Gen_ModEnv2Pitch, Gen_FilterFc, Gen_FilterQ,
    Gen_ModLFO2FilterFc, Gen_ModEnv2FilterFc, Gen_EndAddrCoarseOfs,
    Gen_ModLFO2Vol, Gen_Unused1, Gen_ChorusSend, Gen_ReverbSend, Gen_Pan,
    Gen_Unused2, Gen_Unused3, Gen_Unused4,
    Gen_ModLFODelay, Gen_ModLFOFreq, Gen_VibLFODelay, Gen_VibLFOFreq,
    Gen_ModEnvDelay, Gen_ModEnvAttack, Gen_ModEnvHold, Gen_ModEnvDecay,
    Gen_ModEnvSustain, Gen_ModEnvRelease, Gen_Key2ModEnvHold,
    Gen_Key2ModEnvDecay, Gen_VolEnvDelay, Gen_VolEnvAttack,
    Gen_VolEnvHold, Gen_VolEnvDecay, Gen_VolEnvSustain, Gen_VolEnvRelease,
    Gen_Key2VolEnvHold, Gen_Key2VolEnvDecay, Gen_Instrument,
    Gen_Reserved1, Gen_KeyRange, Gen_VelRange,
    Gen_StartLoopAddrCoarseOfs, Gen_Keynum, Gen_Velocity,
    Gen_Attenuation, Gen_Reserved2, Gen_EndLoopAddrCoarseOfs,
    Gen_CoarseTune, Gen_FineTune, Gen_SampleId, Gen_SampleModes,
    Gen_Reserved3, Gen_ScaleTune, Gen_ExclusiveClass, Gen_OverrideRootKey,
    Gen_Dummy
};

enum Transform { Linear };

//---------------------------------------------------------
//   ModulatorList
//---------------------------------------------------------

struct ModulatorList {
    Modulator src;
    Generator dst;
    int amount;
    Modulator amtSrc;
    Transform transform;
};

//---------------------------------------------------------
//   GeneratorList
//---------------------------------------------------------

union GeneratorAmount {
    short sword;
    uint16_t uword;
    struct {
        uint8_t lo, hi;
    };
};

struct GeneratorList {
    Generator gen;
    GeneratorAmount amount;
};

//---------------------------------------------------------
//   Zone
//---------------------------------------------------------

struct Zone {
    LinkedList<GeneratorList*> generators;
    LinkedList<ModulatorList*> modulators;
    int instrumentIndex;
};

//---------------------------------------------------------
//   Preset
//---------------------------------------------------------

struct Preset {
    char* name;
    int preset;
    int bank;
    int presetBagNdx; // used only for read
    int library;
    int genre;
    int morphology;
    LinkedList<Zone*> zones;

    Preset():name(0), preset(0), bank(0), presetBagNdx(0), library(0), genre(0), morphology(0) {}
};

//---------------------------------------------------------
//   Instrument
//---------------------------------------------------------

struct Instrument {
    char* name;
    int index;        // used only for read
    LinkedList<Zone*> zones;

    Instrument();
    ~Instrument();
};

//---------------------------------------------------------
//   Sample
//---------------------------------------------------------

struct Sample {
    char* name;
    uint32_t start;
    uint32_t end;
    uint32_t loopstart;
    uint32_t loopend;
    uint32_t samplerate;

    int origpitch;
    int pitchadj;
    int sampleLink;
    int sampletype;

    Sample();
    ~Sample();
};

//---------------------------------------------------------
//   SoundFont
//---------------------------------------------------------

class SoundFont {
public:
    string path;
    sfVersionTag version;
    char* engine;
    char* name;
    char* date;
    char* comment;
    char* tools;
    char* creator;
    char* product;
    char* copyright;
    char* irom;
    sfVersionTag iver;

    int samplePos;
    int sampleLen;
    size_t fileLen;

    LinkedList<Preset*> presets;
    LinkedList<Instrument*> instruments;

    LinkedList<Zone*> pZones;
    LinkedList<Zone*> iZones;
    LinkedList<Sample*> samples;

private:

    ByteStream* _file;
    FILE* f;

    bool isBigEndianSys = false;

    bool _compress;
    double _oggQuality;
    double _oggAmp;
    int64_t _oggSerial;

    unsigned readDword();
    int readWord();
    int readShort();
    int readByte();
    int readChar();
    int readFourcc(const char*);
    int readFourcc(char*);
    void readSignature(const char* signature);
    void readSignature(char* signature);
    void skip(int);
    void readSection(const char* fourcc, int len);
    void readVersion(sfVersionTag * v);
    char* readString(int);
    void readPhdr(int);
    void readBag(int, LinkedList<Zone*>*);
    void readMod(int, LinkedList<Zone*>*);
    void readGen(int, LinkedList<Zone*>*);
    void readInst(int);
    void readShdr(int);

    void writeDword(int);
    void writeWord(unsigned short int);
    void writeByte(unsigned char);
    void writeChar(char);
    void writeShort(short);
    void write(const char* p, int n);
    void writeSample(const Sample*);
    void writeStringSection(const char* fourcc, char* s);
    void writePreset(int zoneIdx, const Preset*);
    void writeModulator(const ModulatorList*);
    void writeGenerator(const GeneratorList*);
    void writeInstrument(int zoneIdx, const Instrument*);

    void writeIfil();
    void writeIver();
    void writeSmpl();
    void writePhdr();
    void writeBag(const char* fourcc, LinkedList<Zone*>*);
    void writeMod(const char* fourcc, LinkedList<Zone*>*);
    void writeGen(const char* fourcc, LinkedList<Zone*>*);
    void writeInst();
    void writeShdr();

    int writeCompressedSample(Sample*);
    int writeUncompressedSample(Sample* s);
    bool writeCSample(Sample*, int);
    int writeUncompressedSample(Sample* s, ByteStream& f);

    bool write();

public:
    SoundFont(const string&);
    ~SoundFont();
    bool read();
    bool compress(ByteStream* f, double oggQuality, double oggAmp, int64_t oggSerial = rand());
    bool uncompress(ByteStream* f);
    short* createUncompressSmpl();
    bool writeCode(LinkedList<int>);
    bool writeCode();
    void dumpPresets();

    // Extra option
    bool smallSf;

};
}
#endif

