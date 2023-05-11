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

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <math.h>
#include <vorbis/vorbisenc.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>
#include "sfont.h"
#include "time.h"

using namespace SfTools;

// #define DEBUG

#define BE_SHORT(x) ((((x)&0xFF)<<8) | (((x)>>8)&0xFF))
#ifdef __i486__
#define BE_LONG(x) \
    ({ int __value; \
    asm ("bswap %1; movl %1,%0" : "=g" (__value) : "r" (x)); \
    __value; })
#else
#define BE_LONG(x) ((((x)&0xFF)<<24) | \
    (((x)&0xFF00)<<8) | \
    (((x)&0xFF0000)>>8) | \
    (((x)>>24)&0xFF))
#endif

#define FOURCC(a, b, c, d) a << 24 | b << 16 | c << 8 | d

#define BLOCK_SIZE 1024

//---------------------------------------------------------
//   Sample
//---------------------------------------------------------

Sample::Sample()
{
    name = nullptr;
}

Sample::~Sample()
{
    free(name);
}

//---------------------------------------------------------
//   Instrument
//---------------------------------------------------------

Instrument::Instrument()
{
    name = nullptr;
}

Instrument::~Instrument()
{
    free(name);
}

//---------------------------------------------------------
//   SoundFont
//---------------------------------------------------------

SoundFont::SoundFont(const string& s)
{
    path      = s;
    engine    = nullptr;
    name      = nullptr;
    date      = nullptr;
    comment   = nullptr;
    tools     = nullptr;
    creator   = nullptr;
    product   = nullptr;
    copyright = nullptr;
    irom      = nullptr;
    version.major = 0;
    version.minor = 0;
    iver.major = 0;
    iver.minor = 0;
    smallSf   = false;

    isBigEndianSys = !JudgeLittleOrBigEndianSystem();
}

SoundFont::~SoundFont()
{
    free(engine);
    free(name);
    free(date);
    free(comment);
    free(tools);
    free(creator);
    free(product);
    free(copyright);
    free(irom);

    //
    if (_file != nullptr) {
        delete _file;
        _file = nullptr;
    }

    //
    auto pnode = presets.GetHeadNode();
    for (; pnode; pnode = pnode->next) {
        auto p = pnode->elem;
        auto znode = p->zones.GetHeadNode();
        for (; znode; znode = znode->next) {
            auto z = znode->elem;
            auto gnode = z->generators.GetHeadNode();
            for (; gnode; gnode = gnode->next) {
                auto g = gnode->elem;
                delete g;
            }
            z->generators.Release();  
            //
            auto mnode = z->modulators.GetHeadNode();
            for (; mnode; mnode = mnode->next) {
                auto m = mnode->elem;
                delete m;
            }
            z->modulators.Release();
            delete z;
        }
        p->zones.Release();
        delete p;
    }
    presets.Release();

    //
    auto inode = instruments.GetHeadNode();
    for (; inode; inode = inode->next) {
        auto i = inode->elem;
        auto znode = i->zones.GetHeadNode();
        for (; znode; znode = znode->next) {
            auto z = znode->elem;
            auto gnode = z->generators.GetHeadNode();
            for (; gnode; gnode = gnode->next) {
                auto g = gnode->elem;
                delete g;
            }
            z->generators.Release();
            //
            auto mnode = z->modulators.GetHeadNode();
            for (; mnode; mnode = mnode->next) {
                auto m = mnode->elem;
                delete m;
            }
            z->modulators.Release();
            delete z;
        }
        i->zones.Release();
        delete i;
    }
    instruments.Release();

    //
    pZones.Release();
    iZones.Release();

    //
    auto snode = samples.GetHeadNode();
    for (; snode; snode = snode->next) {
        auto s = snode->elem;
        delete s;
    }
    samples.Release();

}

//---------------------------------------------------------
//   read
//---------------------------------------------------------


bool SoundFont::read()
{
    std::ifstream t;
    char* buffer = nullptr;
    size_t length = 0;
    try
    {
        t.open(path, ios::in | ios::binary);
        t.seekg(0, std::ios::end);
        length = t.tellg();
        t.seekg(0, std::ios::beg);
        buffer = new char[length];
        t.read(buffer, length);
        t.close();
    }
    catch (exception)
    {
        t.close();
        cout << path << "文件打开出错!" << endl;
        return false;
    }

    fileLen = length;
    _file = new ByteStream(buffer, length);
    delete[] buffer;

    //
    try {
        int len = readFourcc("RIFF");
        readSignature("sfbk");
        len -= 4;
        while (len) {
            int len2 = readFourcc("LIST");
            len -= (len2 + 8);
            char fourcc[5];
            fourcc[0] = 0;
            readSignature(fourcc);
            fourcc[4] = 0;
            len2 -= 4;
            while (len2) {
                fourcc[0] = 0;
                int len3 = readFourcc(fourcc);
                fourcc[4] = 0;
                len2 -= (len3 + 8);
                readSection(fourcc, len3);
            }
        }
    }
    catch (string s) {
        cout << "read sf file failed:" << s <<"\n" << endl;
        delete _file;
        return false;
    }

    delete _file;
    _file = nullptr;
    return true;
}


//---------------------------------------------------------
//   skip
//---------------------------------------------------------

void SoundFont::skip(int n)
{
    size_t pos = _file->getReadCursor();
    _file->setReadCursor(pos + n);
}

//---------------------------------------------------------
//   readFourcc
//---------------------------------------------------------

int SoundFont::readFourcc(char* signature)
{
    readSignature(signature);
    return readDword();
}

int SoundFont::readFourcc(const char* signature)
{
    readSignature(signature);
    return readDword();
}

//---------------------------------------------------------
//   readSignature
//---------------------------------------------------------

void SoundFont::readSignature(const char* signature)
{
    char fourcc[4];
    readSignature(fourcc);
    if (memcmp(fourcc, signature, 4) != 0)
        throw(string(FormatStr("fourcc %s expected", signature)));
}

void SoundFont::readSignature(char* signature)
{
    if (_file->read((byte*)signature, 0, 4) != 4)
        throw string("unexpected end of file");
}


//---------------------------------------------------------
//   readDword
//---------------------------------------------------------

unsigned SoundFont::readDword()
{
    unsigned format;
    format = _file->read<uint32_t>();
    if(isBigEndianSys)
        return BE_LONG(format);
    else
        return format;
}

//---------------------------------------------------------
//   writeDword
//---------------------------------------------------------

void SoundFont::writeDword(int val)
{
    if (isBigEndianSys)
        val = BE_LONG(val);
    write((char*)&val, 4);
}

//---------------------------------------------------------
//   writeWord
//---------------------------------------------------------

void SoundFont::writeWord(unsigned short int val)
{
    if (isBigEndianSys)
        val = BE_SHORT(val);
    write((char*)&val, 2);
}

//---------------------------------------------------------
//   writeByte
//---------------------------------------------------------

void SoundFont::writeByte(unsigned char val)
{
    write((char*)&val, 1);
}

//---------------------------------------------------------
//   writeChar
//---------------------------------------------------------

void SoundFont::writeChar(char val)
{
    write((char*)&val, 1);
}

//---------------------------------------------------------
//   writeShort
//---------------------------------------------------------

void SoundFont::writeShort(short val)
{
    if (isBigEndianSys)
        val = BE_SHORT(val);
    write((char*)&val, 2);
}

//---------------------------------------------------------
//   readWord
//---------------------------------------------------------

int SoundFont::readWord()
{
    unsigned short format;
    format = _file->read<uint16_t>();
    if (isBigEndianSys)
        return BE_SHORT(format);
    else
        return format;
}

//---------------------------------------------------------
//   readShort
//---------------------------------------------------------

int SoundFont::readShort()
{
    short format;
    format = _file->read<int16_t>();
    if (isBigEndianSys)
        return BE_SHORT(format);
    else
        return format;
}

//---------------------------------------------------------
//   readByte
//---------------------------------------------------------

int SoundFont::readByte()
{
    uint8_t val = _file->read<uint8_t>();
    return val;
}

//---------------------------------------------------------
//   readChar
//---------------------------------------------------------

int SoundFont::readChar()
{
    char val = _file->read<int8_t>();
    return val;
}

//---------------------------------------------------------
//   readVersion
//---------------------------------------------------------

void SoundFont::readVersion(sfVersionTag * v)
{
    unsigned char data[4];
    if (_file->read((byte*)data, 0, 4) != 4)
        throw string("unexpected end of file");
    v->major = data[0] + (data[1] << 8);
    v->minor = data[2] + (data[3] << 8);
}

//---------------------------------------------------------
//   readString
//---------------------------------------------------------

char* SoundFont::readString(int n)
{
    char data[2500];
    if (_file->read((byte*)data, 0, n) != n)
        throw string("unexpected end of file");
    if (data[n-1] != 0)
        data[n] = 0;
    return _strdup(data);
}

//---------------------------------------------------------
//   readSection
//---------------------------------------------------------

void SoundFont::readSection(const char* fourcc, int len)
{
  //  printf("readSection <%s> len %d\n", fourcc, len);

    switch(FOURCC(fourcc[0], fourcc[1], fourcc[2], fourcc[3])) {
    case FOURCC('i', 'f', 'i', 'l'):    // version
        readVersion(&version);
        break;
    case FOURCC('I','N','A','M'):       // sound font name
        name = readString(len);
        break;
    case FOURCC('i','s','n','g'):       // target render engine
        engine = readString(len);
        break;
    case FOURCC('I','P','R','D'):       // product for which the bank was intended
        product = readString(len);
        break;
    case FOURCC('I','E','N','G'): // sound designers and engineers for the bank
        creator = readString(len);
        break;
    case FOURCC('I','S','F','T'): // SoundFont tools used to create and alter the bank
        tools = readString(len);
        break;
    case FOURCC('I','C','R','D'): // date of creation of the bank
        date = readString(len);
        break;
    case FOURCC('I','C','M','T'): // comments on the bank
        comment = readString(len);
        break;
    case FOURCC('I','C','O','P'): // copyright message
        copyright = readString(len);
        break;
    case FOURCC('s','m','p','l'): // the digital audio samples
        samplePos = _file->getReadCursor();
        sampleLen = len;
        skip(len);
        break;
    case FOURCC('s','m','2','4'): // audio samples (24-bit part)
        skip(len); // Just skip it
        break;
    case FOURCC('p','h','d','r'): // preset headers
        readPhdr(len);
        break;
    case FOURCC('p','b','a','g'): // preset index list
        readBag(len, &pZones);
        break;
    case FOURCC('p','m','o','d'): // preset modulator list
        readMod(len, &pZones);
        break;
    case FOURCC('p','g','e','n'): // preset generator list
        readGen(len, &pZones);
        break;
    case FOURCC('i','n','s','t'): // instrument names and indices
        readInst(len);
        break;
    case FOURCC('i','b','a','g'): // instrument index list
        readBag(len, &iZones);
        break;
    case FOURCC('i','m','o','d'): // instrument modulator list
        readMod(len, &iZones);
        break;
    case FOURCC('i','g','e','n'): // instrument generator list
        readGen(len, &iZones);
        break;
    case FOURCC('s','h','d','r'): // sample headers
        readShdr(len);
        break;
    case FOURCC('i', 'r', 'o', 'm'):    // sample rom
        irom = readString(len);
        break;
    case FOURCC('i', 'v', 'e', 'r'):    // sample rom version
        readVersion(&iver);
        break;
    default:
        skip(len);
        throw(string(FormatStr("unknown fourcc <%s>", fourcc)));
        break;
    }
}

//---------------------------------------------------------
//   readPhdr
//---------------------------------------------------------

void SoundFont::readPhdr(int len)
{
    if (len < (38 * 2))
        throw(string("phdr too short"));
    if (len % 38)
        throw(string("phdr not a multiple of 38"));
    int n = len / 38;
    if (n <= 1) {
        printf("no presets\n");
        skip(len);
        return;
    }
    int index1 = 0, index2;
    for (int i = 0; i < n; ++i) {
        Preset* preset       = new Preset;
        preset->name         = readString(20);
        preset->preset       = readWord();
        preset->bank         = readWord();
        index2               = readWord();
        preset->library      = readDword();
        preset->genre        = readDword();
        preset->morphology   = readDword();
        if (index2 < index1)
            throw("preset header indices not monotonic");
        if (i > 0) {
            int n = index2 - index1;
            while (n--) {
                Zone* z = new Zone;
                presets.GetLastNode()->elem->zones.AddLast(z);
                pZones.AddLast(z);
            }
        }
        index1 = index2;
        presets.AddLast(preset);
    }

}

//---------------------------------------------------------
//   readBag
//---------------------------------------------------------

void SoundFont::readBag(int len, LinkedList<Zone*>* zones)
{
    if (len % 4)
        throw(string("bag size not a multiple of 4"));
    int gIndex2, mIndex2;
    int gIndex1 = readWord();
    int mIndex1 = readWord();
    len -= 4;
    //
    Zone* zone;
    auto node = zones->GetHeadNode();
    for (; node != nullptr; node = node->next) {
        zone = node->elem;
        gIndex2 = readWord();
        mIndex2 = readWord();
        len -= 4;
        if (len < 0)
            throw(string("bag size too small"));
        if (gIndex2 < gIndex1)
            throw("generator indices not monotonic");
        if (mIndex2 < mIndex1)
            throw("modulator indices not monotonic");
        int n = mIndex2 - mIndex1;
        while (n--)
            zone->modulators.AddLast(new ModulatorList);
        n = gIndex2 - gIndex1;
        while (n--)
            zone->generators.AddLast(new GeneratorList);
        gIndex1 = gIndex2;
        mIndex1 = mIndex2;
    }
}

//---------------------------------------------------------
//   readMod
//---------------------------------------------------------

void SoundFont::readMod(int size, LinkedList<Zone*>* zones)
{
    Zone* zone;
    auto node = zones->GetHeadNode();
    for (; node != nullptr; node = node->next) 
    {
        zone = node->elem;
        ModulatorList* m;
        auto mnode = zone->modulators.GetHeadNode();
        for (; mnode != nullptr; mnode = mnode->next)
        {
            m = mnode->elem;
            size -= 10;
            if (size < 0)
                throw(string("pmod size mismatch"));
            m->src           = static_cast<Modulator>(readWord());
            m->dst           = static_cast<Generator>(readWord());
            m->amount        = readShort();
            m->amtSrc        = static_cast<Modulator>(readWord());
            m->transform     = static_cast<Transform>(readWord());
        }
    }
    if (size != 10)
        throw(string("modulator list size mismatch"));
    skip(10);
}

//---------------------------------------------------------
//   readGen
//---------------------------------------------------------

void SoundFont::readGen(int size, LinkedList<Zone*>* zones)
{
    if (size % 4)
        throw(string("bad generator list size"));

    Zone* zone;
    auto node = zones->GetHeadNode();
    for (; node != nullptr; node = node->next)
    {
        zone = node->elem;
        size -= (zone->generators.Size() * 4);
        if (size < 0)
            break;

        GeneratorList* gen;
        auto gnode = zone->generators.GetHeadNode();
        for (; gnode != nullptr; gnode = gnode->next)
        {
            gen = gnode->elem;
            gen->gen = static_cast<Generator>(readWord());
            if (gen->gen == Gen_KeyRange || gen->gen == Gen_VelRange) {
                gen->amount.lo = readByte();
                gen->amount.hi = readByte();
            }
            else if (gen->gen == Gen_Instrument)
                gen->amount.uword = readWord();
            else
                gen->amount.sword = readWord();
        }
    }
    if (size != 4)
        throw(string(FormatStr("generator list size mismatch %d != 4", size)));
    skip(size);
}

//---------------------------------------------------------
//   readInst
//---------------------------------------------------------

void SoundFont::readInst(int size)
{
    int n = size / 22;
    int index1 = 0, index2;
    for (int i = 0; i < n; ++i) {
        Instrument* instrument = new Instrument;
        instrument->name = readString(20);
        index2           = readWord();
        if (index2 < index1)
            throw("instrument header indices not monotonic");
        if (i > 0) {
            int n = index2 - index1;
            while (n--) {
                Zone* z = new Zone;
                instruments.GetLastNode()->elem->zones.AddLast(z);
                iZones.AddLast(z);
            }
        }
        index1 = index2;
        instruments.AddLast(instrument);
    }
}

//---------------------------------------------------------
//   readShdr
//---------------------------------------------------------

void SoundFont::readShdr(int size)
{
    int n = size / 46;
    for (int i = 0; i < n-1; ++i)
    {
        Sample* s = new Sample;
        s->name       = readString(20);
        s->start      = readDword();
        s->end        = readDword();
        s->loopstart  = readDword();
        s->loopend    = readDword();
        s->samplerate = readDword();
        s->origpitch  = readByte();
        s->pitchadj   = readChar();
        s->sampleLink = readWord();
        s->sampletype = readWord();

        // printf("readFontHeader %d %d   %d %d\n", s->start, s->end, s->loopstart, s->loopend);
        samples.AddLast(s);
    }
    skip(46);   // trailing record
}

//---------------------------------------------------------
//   write
//---------------------------------------------------------
bool SoundFont::write()
{
    int64_t riffLenPos;
    int64_t listLenPos;
    try {
        _file->write("RIFF", 4);
        riffLenPos = _file->getWriteCursor();
        writeDword(0);
        _file->write("sfbk", 4);

        _file->write("LIST", 4);
        listLenPos = _file->getWriteCursor();
        writeDword(0);
        _file->write("INFO", 4);

        writeIfil();
        if (name)
            writeStringSection("INAM", name);
        if (engine)
            writeStringSection("isng", engine);
        if (product)
            writeStringSection("IPRD", product);
        if (creator)
            writeStringSection("IENG", creator);
        if (tools)
            writeStringSection("ISFT", tools);
        if (date)
            writeStringSection("ICRD", date);
        if (comment)
            writeStringSection("ICMT", comment);
        if (copyright)
            writeStringSection("ICOP", copyright);
        if (irom)
            writeStringSection("irom", irom);
        writeIver();

        int64_t pos = _file->getWriteCursor();
        _file->setWriteCursor(listLenPos);
        writeDword(pos - listLenPos - 4);
        _file->setWriteCursor(pos);

        _file->write("LIST", 4);
        listLenPos = _file->getWriteCursor();
        writeDword(0);
        _file->write("sdta", 4);
        writeSmpl();
        pos = _file->getWriteCursor();
        _file->setWriteCursor(listLenPos);
        writeDword(pos - listLenPos - 4);
        _file->setWriteCursor(pos);

        _file->write("LIST", 4);
        listLenPos = _file->getWriteCursor();
        writeDword(0);
        _file->write("pdta", 4);

        writePhdr();
        writeBag("pbag", &pZones);
        writeMod("pmod", &pZones);
        writeGen("pgen", &pZones);
        writeInst();
        writeBag("ibag", &iZones);
        writeMod("imod", &iZones);
        writeGen("igen", &iZones);
        writeShdr();

        pos = _file->getWriteCursor();
        _file->setWriteCursor(listLenPos);
        writeDword(pos - listLenPos - 4);
        _file->setWriteCursor(pos);

        int64_t endPos = _file->getWriteCursor();
        _file->setWriteCursor(riffLenPos);
        writeDword(endPos - riffLenPos - 4);
    }
    catch (string s) {
        printf("write sf file failed: %s\n", s.c_str());
        return false;
    }
    return true;
}

bool SoundFont::compress(ByteStream* f, double oggQuality, double oggAmp, int64_t oggSerial)
{
    _file = f;
    _compress = true;
    _oggQuality = oggQuality;
    _oggAmp = oggAmp;
    _oggSerial = oggSerial;

    return write();
}

bool SoundFont::uncompress(ByteStream* f)
{
    _file = f;
    _compress = false;
    return write();
}

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void SoundFont::write(const char* p, int n)
{
    _file->write(p, n);
}

//---------------------------------------------------------
//   writeStringSection
//---------------------------------------------------------

void SoundFont::writeStringSection(const char* fourcc, char* s)
{
    write(fourcc, 4);
    int nn = strlen(s) + 1;
    int n = ((nn + 1) / 2) * 2;
    writeDword(n);
    write(s, nn);
    if (n - nn) {
        char c = 0;
        write(&c, 1);
    }
}

//---------------------------------------------------------
//   writeIfil
//---------------------------------------------------------

void SoundFont::writeIfil()
{
    write("ifil", 4);
    writeDword(4);
    unsigned char data[4];
    if (_compress)
    {
        version.major = 3;
        version.minor = 0;
    }
    else
    {
        version.major = 2;
        version.minor = 1;
    }
    data[0] = version.major;
    data[1] = version.major >> 8;
    data[2] = version.minor;
    data[3] = version.minor >> 8;
    write((char*)data, 4);
}

//---------------------------------------------------------
//   writeIVer
//---------------------------------------------------------

void SoundFont::writeIver()
{
    write("iver", 4);
    writeDword(4);
    unsigned char data[4];
    data[0] = iver.major;
    data[1] = iver.major >> 8;
    data[2] = iver.minor;
    data[3] = iver.minor >> 8;
    write((char*)data, 4);
}

//---------------------------------------------------------
//   writeSmpl
//---------------------------------------------------------

void SoundFont::writeSmpl()
{
    write("smpl", 4);
    int64_t pos = _file->getWriteCursor();
    writeDword(0);
    int currentSamplePos = 0;
    if (_compress)
    {
        // Compress wave data
        Sample* s;
        auto node = samples.GetHeadNode();
        for (; node != nullptr; node = node->next)
        {
            s = node->elem;
            // Loop start and end are now based on the beginning of each sample
            s->loopstart -= s->start;
            s->loopend   -= s->start;

            // OGG flag added
            s->sampletype |= 0x10;
            int len = writeCompressedSample(s); // In byte
            s->start = currentSamplePos;
            currentSamplePos += len;
            s->end = currentSamplePos;
        }
    }
    else
    {
        // Uncompress from OGG data

        Sample* s;
        auto node = samples.GetHeadNode();
        for (; node != nullptr; node = node->next)
        {
            s = node->elem;
            // OGG flag is removed
            s->sampletype &= ~0x10;
            int len = writeUncompressedSample(s) / 2; // In sample data points (16 bits)
            s->start = currentSamplePos;
            currentSamplePos += len;
            s->end = currentSamplePos;

            // Loop start and end are now based on the beginning of the section "smpl"
            s->loopstart += s->start;
            s->loopend   += s->start;
        }
    }

    int64_t npos = _file->getWriteCursor();
    _file->setWriteCursor(pos);
    writeDword(npos - pos - 4);
    _file->setWriteCursor(npos);
}

// Uncompress from OGG data
short* SoundFont::createUncompressSmpl() 
{
    std::ifstream t;
    char* buffer = nullptr;
    size_t length = 0;
    try
    {
        t.open(path, ios::in | ios::binary);
        t.seekg(0, std::ios::end);
        length = t.tellg();
        t.seekg(0, std::ios::beg);
        buffer = new char[length];
        t.read(buffer, length);
        t.close();
    }
    catch (exception)
    {
        t.close();
        cout << path << "文件打开出错!" << endl;
        return nullptr;
    }

    ByteStream f(buffer, length);
    delete[] buffer;
    //
    _file = new ByteStream(nullptr, length*5);
    int currentSamplePos = 0;
    Sample* s;
    auto node = samples.GetHeadNode();
    for (; node != nullptr; node = node->next)
    {
        s = node->elem;
        // OGG flag is removed
        s->sampletype &= ~0x10;
        int len = writeUncompressedSample(s, f) / 2; // In sample data points (16 bits)
        s->start = currentSamplePos;
        currentSamplePos += len;
        s->end = currentSamplePos;

        //根据sf2格式规范将在样本末尾多添加46个零值点
        //sf24文档规范的6.1 Sample Data Format in the smpl Sub-chunk有提到
        currentSamplePos += 46;

        // Loop start and end are now based on the beginning of the section "smpl"
        s->loopstart += s->start;
        s->loopend += s->start;
    }

    length = _file->getWriteCursor();
    short* smpls = (short*)malloc(length);
    if (smpls == nullptr) {
        delete _file;
        _file = nullptr;
        return nullptr;
    }

    _file->read((byte*)smpls, 0, length);
    sampleLen = length;
    delete _file;
    _file = nullptr;
    return smpls;
}


//---------------------------------------------------------
//   writePhdr
//---------------------------------------------------------

void SoundFont::writePhdr()
{
    write("phdr", 4);
    int n = presets.Size();
    writeDword((n + 1) * 38);
    int zoneIdx = 0;

    auto node = presets.GetHeadNode();
    for (; node != nullptr; node = node->next)
    {
        Preset* p = node->elem;
        writePreset(zoneIdx, p);
        zoneIdx += p->zones.Size();
    }
    Preset p;
    writePreset(zoneIdx, &p);
}

//---------------------------------------------------------
//   writePreset
//---------------------------------------------------------

void SoundFont::writePreset(int zoneIdx, const Preset* preset)
{
    char name[20];
    memset(name, 0, 20);
    if (preset->name)
        memcpy(name, preset->name, strlen(preset->name));
    write(name, 20);
    writeWord(preset->preset);
    writeWord(preset->bank);
    writeWord(zoneIdx);
    writeDword(preset->library);
    writeDword(preset->genre);
    writeDword(preset->morphology);
}

//---------------------------------------------------------
//   writeBag
//---------------------------------------------------------

void SoundFont::writeBag(const char* fourcc, LinkedList<Zone*>* zones)
{
    write(fourcc, 4);
    int n = zones->Size();
    writeDword((n + 1) * 4);
    int gIndex = 0;
    int pIndex = 0;

    auto node = zones->GetHeadNode();
    for (; node != nullptr; node = node->next)
    {
        Zone* z = node->elem;
        writeWord(gIndex);
        writeWord(pIndex);
        gIndex += z->generators.Size();
        pIndex += z->modulators.Size();
    }
    writeWord(gIndex);
    writeWord(pIndex);
}

//---------------------------------------------------------
//   writeMod
//---------------------------------------------------------

void SoundFont::writeMod(const char* fourcc, LinkedList<Zone*>* zones)
{
    write(fourcc, 4);
    int n = 0;

    LinkedListNode<Zone*>* node = zones->GetHeadNode();
    for (; node != nullptr; node = node->next){
        Zone* z = node->elem;
        n += z->modulators.Size();
    }
    writeDword((n + 1) * 10);

    node = zones->GetHeadNode();
    for (; node != nullptr; node = node->next)
    {
        Zone* zone = node->elem;
        auto mnode = zone->modulators.GetHeadNode();
        for (; mnode != nullptr; mnode = mnode->next)
        {
            writeModulator(mnode->elem);
        }
    }

    ModulatorList mod;
    memset(&mod, 0, sizeof(mod));
    writeModulator(&mod);
}

//---------------------------------------------------------
//   writeModulator
//---------------------------------------------------------

void SoundFont::writeModulator(const ModulatorList* m)
{
    writeWord(m->src);
    writeWord(m->dst);
    writeShort(m->amount);
    writeWord(m->amtSrc);
    writeWord(m->transform);
}

//---------------------------------------------------------
//   writeGen
//---------------------------------------------------------

void SoundFont::writeGen(const char* fourcc, LinkedList<Zone*>* zones)
{
    write(fourcc, 4);
    int n = 0;

    LinkedListNode<Zone*>* node = zones->GetHeadNode();
    for (; node != nullptr; node = node->next){
        Zone* z = node->elem;
        n += z->generators.Size();
    }
    writeDword((n + 1) * 4);

    node = zones->GetHeadNode();
    for (; node != nullptr; node = node->next)
    {
        Zone* zone = node->elem;
        auto gnode = zone->generators.GetHeadNode();
        for (; gnode != nullptr; gnode = gnode->next)
        {
            writeGenerator(gnode->elem);
        }
    }

    GeneratorList gen;
    memset(&gen, 0, sizeof(gen));
    writeGenerator(&gen);
}

//---------------------------------------------------------
//   writeGenerator
//---------------------------------------------------------

void SoundFont::writeGenerator(const GeneratorList* g)
{
    writeWord(g->gen);
    if (g->gen == Gen_KeyRange || g->gen == Gen_VelRange) {
        writeByte(g->amount.lo);
        writeByte(g->amount.hi);
    }
    else if (g->gen == Gen_Instrument)
        writeWord(g->amount.uword);
    else
        writeWord(g->amount.sword);
}

//---------------------------------------------------------
//   writeInst
//---------------------------------------------------------

void SoundFont::writeInst()
{
    write("inst", 4);
    int n = instruments.Size();
    writeDword((n + 1) * 22);
    int zoneIdx = 0;

    auto node = instruments.GetHeadNode();
    for (; node != nullptr; node = node->next)
    {
        Instrument* p = node->elem;
        writeInstrument(zoneIdx, p);
        zoneIdx += p->zones.Size();
    }

    Instrument p;
    writeInstrument(zoneIdx, &p);
}

//---------------------------------------------------------
//   writeInstrument
//---------------------------------------------------------

void SoundFont::writeInstrument(int zoneIdx, const Instrument* instrument)
{
    char name[20];
    memset(name, 0, 20);
    if (instrument->name)
        memcpy(name, instrument->name, strlen(instrument->name));
    write(name, 20);
    writeWord(zoneIdx);
}

//---------------------------------------------------------
//   writeShdr
//---------------------------------------------------------

void SoundFont::writeShdr()
{
    write("shdr", 4);
    writeDword(46 * (samples.Size() + 1));

    auto node = samples.GetHeadNode();
    for (; node != nullptr; node = node->next)
        writeSample(node->elem);
    
    Sample s;
    memset(&s, 0, sizeof(s));
    writeSample(&s);
}

//---------------------------------------------------------
//   writeSample
//---------------------------------------------------------

void SoundFont::writeSample(const Sample* s)
{
    char name[20];
    memset(name, 0, 20);
    if (s->name)
        memcpy(name, s->name, strlen(s->name));
    write(name, 20);
    writeDword(s->start);
    writeDword(s->end);
    writeDword(s->loopstart);
    writeDword(s->loopend);
    writeDword(s->samplerate);
    writeByte(s->origpitch);
    writeChar(s->pitchadj);
    writeWord(s->sampleLink);
    writeWord(s->sampletype);
}

//---------------------------------------------------------
//   writeCompressedSample
//---------------------------------------------------------

int SoundFont::writeCompressedSample(Sample* s)
{
    std::ifstream t;
    char* buffer = nullptr;
    size_t length = 0;
    try
    {
        t.open(path, ios::in | ios::binary);
        t.seekg(0, std::ios::end);
        length = t.tellg();
        t.seekg(0, std::ios::beg);
        buffer = new char[length];
        t.read(buffer, length);
        t.close();
    }
    catch (exception)
    {
        t.close();
        cout << path << "文件打开出错!" << endl;
        return false;
    }

    ByteStream f(buffer, length);
    delete[] buffer;

    //
    f.setReadCursor(samplePos + s->start * sizeof(short));
    int samples = s->end - s->start;
    short * ibuffer = new short[samples];
    f.read((byte*)ibuffer, 0, samples * sizeof(short));
  

    ogg_stream_state os;
    ogg_page         og;
    ogg_packet       op;
    vorbis_info      vi;
    vorbis_dsp_state vd;
    vorbis_block     vb;
    vorbis_comment   vc;

    vorbis_info_init(&vi);
    int ret = vorbis_encode_init_vbr(&vi, 1, s->samplerate, _oggQuality);
    if (ret) {
        printf("vorbis init failed\n");
        delete [] ibuffer;
        return false;
    }
    vorbis_comment_init(&vc);
    vorbis_analysis_init(&vd, &vi);
    vorbis_block_init(&vd, &vb);
    srand(time(nullptr));

    ogg_stream_init(&os, _oggSerial == (std::numeric_limits<int64_t>::max)() ? rand() : (int)_oggSerial);

    ogg_packet header;
    ogg_packet header_comm;
    ogg_packet header_code;

    // Keep a track of the attenuation used before the compression
    vorbis_comment_add(&vc, FormatStr("AMP=%f\0", _oggAmp));

    vorbis_analysis_headerout(&vd, &vc, &header, &header_comm, &header_code);
    ogg_stream_packetin(&os, &header);
    ogg_stream_packetin(&os, &header_comm);
    ogg_stream_packetin(&os, &header_code);

    char* obuf = new char[1048576]; // 1024 * 1024
    char* p = obuf;

    for (;;) {
        int result = ogg_stream_flush(&os, &og);
        if (result == 0)
            break;
        memcpy(p, og.header, og.header_len);
        p += og.header_len;
        memcpy(p, og.body, og.body_len);
        p += og.body_len;
    }

    long i;
    int page = 0;
    double linearAmp = pow(10.0, _oggAmp / 20.0);
    for(;;) {
        int bufflength = min(BLOCK_SIZE, samples - page * BLOCK_SIZE);
        float **buffer = vorbis_analysis_buffer(&vd, bufflength);
        int j = 0;
        int max = min((page+1) * BLOCK_SIZE, samples);
        for (i = page * BLOCK_SIZE; i < max ; i++) {
            buffer[0][j] = (ibuffer[i] / 32768.f) * linearAmp;
            j++;
        }

        vorbis_analysis_wrote(&vd, bufflength);

        while (vorbis_analysis_blockout(&vd, &vb) == 1) {
            vorbis_analysis(&vb, 0);
            vorbis_bitrate_addblock(&vb);

            while (vorbis_bitrate_flushpacket(&vd, &op)) {
                ogg_stream_packetin(&os, &op);

                for(;;) {
                    int result = ogg_stream_pageout(&os, &og);
                    if (result == 0)
                        break;
                    memcpy(p, og.header, og.header_len);
                    p += og.header_len;
                    memcpy(p, og.body, og.body_len);
                    p += og.body_len;
                }
            }
        }
        page++;
        if ((max == samples) || !((samples-page*BLOCK_SIZE)>0))
            break;
    }

    vorbis_analysis_wrote(&vd, 0);

    while (vorbis_analysis_blockout(&vd, &vb) == 1) {
        vorbis_analysis(&vb, 0);
        vorbis_bitrate_addblock(&vb);

        while (vorbis_bitrate_flushpacket(&vd, &op)) {
            ogg_stream_packetin(&os, &op);

            for(;;) {
                int result = ogg_stream_pageout(&os, &og);
                if (result == 0)
                    break;
                memcpy(p, og.header, og.header_len);
                p += og.header_len;
                memcpy(p, og.body, og.body_len);
                p += og.body_len;
            }
        }
    }

    ogg_stream_clear(&os);
    vorbis_block_clear(&vb);
    vorbis_dsp_clear(&vd);
    vorbis_comment_clear(&vc);
    vorbis_info_clear(&vi);

    int n = p - obuf;
    write(obuf, n);

    delete [] obuf;
    delete [] ibuffer;

    return n;
}

//---------------------------------------------------------
//   writeCSample
//---------------------------------------------------------

bool SoundFont::writeCSample(Sample* s, int idx)
{
    std::ifstream t;
    char* buffer = nullptr;
    size_t length = 0;
    try
    {
        t.open(path, ios::in | ios::binary);
        t.seekg(0, std::ios::end);
        length = t.tellg();
        t.seekg(0, std::ios::beg);
        buffer = new char[length];
        t.read(buffer, length);
        t.close();
    }
    catch (exception)
    {
        t.close();
        cout << path << "文件打开出错!" << endl;
        return false;
    }

    ByteStream fi(buffer, length);
    delete[] buffer;




    fi.setReadCursor(samplePos + s->start * sizeof(short));
    int samples = s->end - s->start;
    short * ibuffer = new short[samples];
    fi.read((byte*)ibuffer, 0, samples * sizeof(short));

    fprintf(f, "static const short wave%d[] = {\n      ", idx);
    int n = 0;
    for (int row = 0;; ++row) {
        for (int col = 0; col < 16; ++col) {
            if (n >= samples)
                break;
            if (col != 0)
                fprintf(f, ", ");
            fprintf(f, "%d", ibuffer[n]);
            ++n;
        }
        if (n >= samples)
            break;
        fprintf(f, ",\n      ");
    }
    fprintf(f, "\n      };\n");
    delete [] ibuffer;
    return true;
}

//---------------------------------------------------------
//   checkInstrument
//---------------------------------------------------------

static bool checkInstrument(LinkedList<int> pnums, LinkedList<Preset*> presets, int instrIdx)
{

    auto node = pnums.GetHeadNode();
    for (; node != nullptr; node = node->next)
    {
        int idx = node->elem;
        Preset* p = presets[idx]->elem;
        auto znode = p->zones.GetHeadNode();
        for (; znode != nullptr; znode = znode->next)
        {
            Zone* z = znode->elem;
            auto gnode = z->generators.GetHeadNode();
            for (; gnode != nullptr; gnode = gnode->next)
            {
                GeneratorList* g = gnode->elem;
                if (g->gen == Gen_Instrument) {
                    if (instrIdx == g->amount.uword)
                        return true;
                }
            }
        }
    }
    return false;
}

static bool checkInstrument(LinkedList<Preset*> presets, int instrIdx) {

    auto node = presets.GetHeadNode();
    for (; node != nullptr; node = node->next)
    {
        Preset* p = node->elem;
        Zone* z = p->zones.GetHeadNode()->elem;
        auto gnode = z->generators.GetHeadNode();
        for (; gnode != nullptr; gnode = gnode->next)
        {
            GeneratorList* g = gnode->elem;
            if (g->gen == Gen_Instrument) {
                if (instrIdx == g->amount.uword)
                    return true;
            }
        }
    }

    return false;
}

//---------------------------------------------------------
//   checkSample
//---------------------------------------------------------

static bool checkSample(LinkedList<int> pnums, LinkedList<Preset*> presets, LinkedList<Instrument*> instruments,
                        int sampleIdx)
{
    int idx = 0;
    auto node = instruments.GetHeadNode();
    for (; node != nullptr; node = node->next)
    {
        Instrument* instrument = node->elem;
        if (!checkInstrument(pnums, presets, idx)) {
            ++idx;
            continue;
        }

        auto znode = instrument->zones.GetHeadNode();
        for (; znode != nullptr; znode = znode->next)
        {
            Zone* z = znode->elem;
            auto gnode = z->generators.GetHeadNode();
            for (; gnode != nullptr; gnode = gnode->next){
                GeneratorList* g = gnode->elem;
                if (g->gen == Gen_SampleId) {
                    if (sampleIdx == g->amount.uword)
                        return true;
                }
            }
        }
        ++idx;
    }
    return false;
}

//---------------------------------------------------------
//   checkSample
//---------------------------------------------------------

static bool checkSample(LinkedList<Preset*> presets, LinkedList<Instrument*> instruments,
                        int sampleIdx)
{
    int idx = 0;
    auto node = instruments.GetHeadNode();
    for (; node != nullptr; node = node->next)
    {
        Instrument* instrument = node->elem;
        if (!checkInstrument(presets, idx)) {
            ++idx;
            continue;
        }

        auto znode = instrument->zones.GetHeadNode();
        for (; znode != nullptr; znode = znode->next)
        {
            Zone* z = znode->elem;
            auto gnode = z->generators.GetHeadNode();
            for (; gnode != nullptr; gnode = gnode->next){
                GeneratorList* g = gnode->elem;
                if (g->gen == Gen_SampleId) {
                    if (sampleIdx == g->amount.uword)
                        return true;
                }
            }
        }

        ++idx;
    }
    return false;
}


//---------------------------------------------------------
//   writeCode
//---------------------------------------------------------

bool SoundFont::writeCode()
{
    int segments = 8;
    int n        = samples.Size() / segments;
    for (int i = 0; i < segments; ++i) {
        char buffer[16];
        sprintf(buffer, "sf%d.cpp", i + 1);
        f = fopen(buffer, "w+");
        fprintf(f, "//\n//      this is generated code, do not change\n//\n");
        fprintf(f, "#include \"sfont.h\"\n\n");
        fprintf(f, "namespace FluidS {\n\n");

        //
        // dump wave samples
        //
        int end;
        if (i+1 == segments)
            end = samples.Size();
        else
            end = (i+1) * n;
        vector<int> sampleIdx;
        for (int idx = i * n; idx < end; ++idx) {
            if (smallSf && !checkSample(presets, instruments, idx))
                continue;
            Sample* s = samples[idx]->elem;
            writeCSample(s, idx);
            sampleIdx.push_back(idx);
        }

        //
        // dump Sample[]
        //
        for (int j = 0; j < sampleIdx.size(); j++) {
            int idx = sampleIdx[i];
            Sample* s = samples[idx]->elem;
            fprintf(f, "Sample sample%d(%d, %d, %d, %d, %d, %d, %d, %d, wave%d);\n",
                    idx,
                    0,
                    s->end       - (s->start + 1),
                    s->loopstart, //  - s->start,
                    s->loopend,   //  - s->start,
                    s->samplerate, s->origpitch, s->pitchadj, s->sampletype,
                    idx);
        }
        fprintf(f, "} // end namespace\n");
        fclose(f);
    }

    f = fopen("sf.cpp", "w+");
    fprintf(f, "//\n//      this is generated code, do not change\n//\n");
    fprintf(f, "#include \"sfont.h\"\n\n");
    fprintf(f, "namespace FluidS {\n\n");

    fprintf(f, "extern Sample ");
    for (int i = 0; i < samples.Size(); ++i) {
        if (i) {
            if ((i % 8) == 0)
                fprintf(f, ";\nextern Sample ");
            else
                fprintf(f, ", ");
        }
        fprintf(f, "sample%d", i);
    }
    fprintf(f, ";\n");

    //
    // dump Instrument[]
    //
    int idx2;
    int idx = 0;
    auto node = instruments.GetHeadNode();
    for (; node != nullptr; node = node->next)
    {
        Instrument* instrument = node->elem;
        if (smallSf && !checkInstrument(presets, idx)) {
            ++idx;
            continue;
        }
        int zones = instrument->zones.Size();
        idx2 = 0;

        auto znode = instrument->zones.GetHeadNode();
        for (; znode != nullptr; znode = znode->next)
        {
            Zone* z = znode->elem;
            int keyLo     = 0;
            int keyHi     = 127;
            int veloLo    = 0;
            int veloHi    = 127;
            int sampleIdx = -1;

            vector<GeneratorList*> gl;

            auto gnode = z->generators.GetHeadNode();
            for (; gnode != nullptr; gnode = gnode->next) {
                GeneratorList* g = gnode->elem;
                if (g->gen == Gen_KeyRange) {
                    keyLo = g->amount.lo;
                    keyHi = g->amount.hi;
                }
                else if (g->gen == Gen_VelRange) {
                    veloLo = g->amount.lo;
                    veloHi = g->amount.hi;
                }
                else if (g->gen == Gen_SampleId)
                    sampleIdx = g->amount.uword;
                else
                    gl.push_back(g);
                if (smallSf && g->gen == Gen_Pan)
                    g->amount.uword = 0;
            }
            int idx3 = 0;
            GeneratorList* g;
            for (int j = 0; j < gl.size(); j++) {
                g = gl[j]; 
                if ((idx3 % 8) == 0) {
                    if (idx3)
                        fprintf(f, ";\n");
                    fprintf(f, "static Generator ");
                }
                else
                    fprintf(f, ", ");
                fprintf(f, "ge_%d_%d_%d(%d, %d)",
                        idx, idx2, idx3, int(g->gen), g->amount.sword);
                ++idx3;
            }
            fprintf(f, ";\n");
            int n = gl.size();
            if (n) {
                fprintf(f, "static Generator* geList_%d_%d[%d] = {\n      ", idx, idx2, n);
                for (int i = 0; i < n; ++i) {
                    if (i) {
                        fprintf(f, ", ");
                        if ((i % 8) == 0)
                            fprintf(f, "\n      ");
                    }
                    fprintf(f, "&ge_%d_%d_%d", idx, idx2, i);
                }
                fprintf(f, "\n      };\n");
            }
            if (sampleIdx == -1)
                fprintf(f, "static IZone iz%d_%d(%d, %d, %d, %d, 0, 0, 0);\n", idx, idx2, keyLo, keyHi, veloLo, veloHi);
            else if (n)
                fprintf(f, "static IZone iz%d_%d(%d, %d, %d, %d, &sample%d, %d, geList_%d_%d);  // %s\n",
                        idx, idx2, keyLo, keyHi, veloLo, veloHi, sampleIdx, n, idx, idx2,
                        samples[sampleIdx]->elem->name);
            else
                fprintf(f, "static IZone iz%d_%d(%d, %d, %d, %d, &sample%d, %d, 0);  // %s\n",
                        idx, idx2, keyLo, keyHi, veloLo, veloHi, sampleIdx, n,
                        samples[sampleIdx]->elem->name);

            ++idx2;
        }

        fprintf(f, "static IZone* izones%d[%d] = {\n", idx, zones);
        idx2 = 0;
        for (int i = 0; i < zones; ++i)
            fprintf(f, "      &iz%d_%d,\n", idx, idx2++);
        fprintf(f, "      };\n");
        fprintf(f, "static Instrument instr%d(0, %d, izones%d);\n\n", idx, zones, idx);
        ++idx;
    }
    //
    // dump Preset[]
    //
    idx = 0;

    LinkedListNode<Preset*>* pnode = presets.GetHeadNode();
    for (; pnode != nullptr; pnode = pnode->next)
    {
        Preset* p = pnode->elem;
        idx2 = 0;
        int zones = p->zones.Size();
        if (smallSf)
            zones = 1;

        auto znode = p->zones.GetHeadNode();
        for (; znode != nullptr; znode = znode->next)
        {
            Zone* z = znode->elem;
            int keyLo    = 0;
            int keyHi    = 127;
            int veloLo   = 0;
            int veloHi   = 127;
            int instrIdx = -1;

            auto gnode = z->generators.GetHeadNode();
            for (; gnode != nullptr; gnode = gnode->next)
            {
                GeneratorList* g = gnode->elem;
                if (g->gen == Gen_KeyRange) {
                    keyLo = g->amount.lo;
                    keyHi = g->amount.hi;
                }
                else if (g->gen == Gen_VelRange) {
                    veloLo = g->amount.lo;
                    veloHi = g->amount.hi;
                }
                else if (g->gen == Gen_Instrument)
                    instrIdx = g->amount.uword;
            }
            if (keyLo == 0
                    && keyHi == 127
                    && veloLo == 0
                    && veloHi == 127) {
                if (instrIdx == -1)
                    fprintf(f, "static PZone pz%d_%d(0);\n", idx, idx2);
                else
                    fprintf(f, "static PZone pz%d_%d(&instr%d);\n", idx, idx2, instrIdx);
            }
            else {
                //abort();

                if (instrIdx == -1)
                    fprintf(f, "static PZone pz%d_%d(%d, %d, %d, %d, 0);\n", idx, idx2, keyLo, keyHi, veloLo, veloHi);
                else
                    fprintf(f, "static PZone pz%d_%d(%d, %d, %d, %d, &instr%d);\n", idx, idx2, keyLo, keyHi, veloLo, veloHi, instrIdx);
            }
            if (smallSf)
                break;
            ++idx2;
        }
        fprintf(f, "static PZone* pzones%d[%d] = {\n", idx, zones);
        idx2 = 0;
        for (int i = 0; i < zones; ++i) {
            fprintf(f, "      &pz%d_%d,\n", idx, idx2);
            ++idx2;
        }
        fprintf(f, "      };\n");

        fprintf(f, "static Preset preset%d(%d, %d, 0, %d, pzones%d);  // %s\n\n",
                idx, p->preset, p->bank, zones, idx, p->name);
        ++idx;
    }


    fprintf(f, "static Preset* sfPresets[%d] = {\n", presets.Size());
    
    //
    pnode = presets.GetHeadNode();
    for (; pnode != nullptr; pnode = pnode->next)
    {
        Preset* p = pnode->elem;
        fprintf(f, "      &preset%d,   // %s\n", idx, p->name);
    }

    //
    fprintf(f, "      };\n");

    fprintf(f, "static SFont _buildinSf(%d, sfPresets);\n", presets.Size());
    fprintf(f, "SFont* createSf() { return &_buildinSf; }\n");
    fprintf(f, "} // end namespace\n");
    fclose(f);
    return true;
}



//---------------------------------------------------------
//   writeCode
//---------------------------------------------------------

bool SoundFont::writeCode(LinkedList<int> pnums)
{
    printf("write code\n");

    int n = samples.Size();
    f = fopen("sf.cpp", "w+");
    fprintf(f, "//\n//      this is generated code, do not change\n//\n");
    fprintf(f, "#include \"sfont.h\"\n\n");
    fprintf(f, "namespace FluidS {\n\n");

    //
    // dump wave samples
    //
    vector<int> sampleIdx;
    for (int idx = 0; idx < n; ++idx) {
        if (!checkSample(pnums, presets, instruments, idx))
            continue;
        Sample* s = samples[idx]->elem;
        writeCSample(s, idx);
        sampleIdx.push_back(idx);
    }

    //
    // dump Sample[]
    //
    for(int i=0; i<sampleIdx.size(); i++){
        int idx = sampleIdx[i];
        Sample* s = samples[idx]->elem;
        fprintf(f, "Sample sample%d(%d, %d, %d, %d, %d, %d, %d, %d, wave%d);\n",
                idx,
                0,                         // sample start
                s->end       - s->start,   // samples
                s->loopstart - s->start,
                s->loopend   - s->start,
                s->samplerate,
                s->origpitch, s->pitchadj, s->sampletype,
                idx);
    }

    //
    // dump Instrument[]
    //
    int idx2;
    int idx = 0;

    auto node = instruments.GetHeadNode();
    for (; node != nullptr; node = node->next)
    {
        Instrument* instrument = node->elem;
        if (!checkInstrument(pnums, presets, idx)) {
            ++idx;
            continue;
        }
        int zones = instrument->zones.Size();
        idx2 = 0;


        auto znode = instrument->zones.GetHeadNode();
        for (; znode != nullptr; znode = znode->next)
        {
            Zone* z = znode->elem;
            int keyLo     = 0;
            int keyHi     = 127;
            int veloLo    = 0;
            int veloHi    = 127;
            int sampleIdx = -1;

            vector<GeneratorList*> gl;
            auto gnode = z->generators.GetHeadNode();
            for (; gnode != nullptr; gnode = gnode->next)
            {
                GeneratorList* g = gnode->elem;
                if (g->gen == Gen_KeyRange) {
                    keyLo = g->amount.lo;
                    keyHi = g->amount.hi;
                }
                else if (g->gen == Gen_VelRange) {
                    veloLo = g->amount.lo;
                    veloHi = g->amount.hi;
                }
                else if (g->gen == Gen_SampleId)
                    sampleIdx = g->amount.uword;
                else
                    gl.push_back(g);
            }
            int idx3 = 0;

            for(int i=0; i<gl.size(); i++){
                GeneratorList* g = gl[i];
                if ((idx3 % 8) == 0) {
                    if (idx3)
                        fprintf(f, ";\n");
                    fprintf(f, "static Generator ");
                }
                else
                    fprintf(f, ", ");
                fprintf(f, "ge_%d_%d_%d(%d, %d)",
                        idx, idx2, idx3, int(g->gen), g->amount.sword);
                ++idx3;
            }
            fprintf(f, ";\n");
            int n = gl.size();
            if (n) {
                fprintf(f, "static Generator* geList_%d_%d[%d] = {\n      ", idx, idx2, n);
                for (int i = 0; i < n; ++i) {
                    if (i) {
                        fprintf(f, ", ");
                        if ((i % 8) == 0)
                            fprintf(f, "\n      ");
                    }
                    fprintf(f, "&ge_%d_%d_%d", idx, idx2, i);
                }
                fprintf(f, "\n      };\n");
            }
            if (sampleIdx == -1)
                fprintf(f, "static IZone iz%d_%d(%d, %d, %d, %d, 0, 0, 0);\n", idx, idx2, keyLo, keyHi, veloLo, veloHi);
            else
                fprintf(f, "static IZone iz%d_%d(%d, %d, %d, %d, &sample%d, %d, geList_%d_%d);\n",
                        idx, idx2, keyLo, keyHi, veloLo, veloHi, sampleIdx, n, idx, idx2);
            ++idx2;
        }

        fprintf(f, "static IZone* izones%d[%d] = {\n", idx, zones);
        idx2 = 0;
        for (int i = 0; i < zones; ++i)
            fprintf(f, "      &iz%d_%d,\n", idx, idx2++);
        fprintf(f, "      };\n");
        fprintf(f, "static Instrument instr%d(0, %d, izones%d);\n\n", idx, zones, idx);
        ++idx;
    }
    //
    // dump Preset[]
    //

    auto numnode = pnums.GetHeadNode();
    for (; numnode != nullptr; numnode = numnode->next)
    {
        int idx = numnode->elem;
        Preset* p = presets[idx]->elem;
        idx2 = 0;
        int zones = p->zones.Size();

        auto znode = p->zones.GetHeadNode();
        for (; znode != nullptr; znode = znode->next)
        {
            Zone* z = znode->elem;
            int keyLo    = 0;
            int keyHi    = 127;
            int veloLo   = 0;
            int veloHi   = 127;
            int instrIdx = -1;

            auto gnode = z->generators.GetHeadNode();
            for (; gnode != nullptr; gnode = gnode->next)
            {
                GeneratorList* g = gnode->elem;
                if (g->gen == Gen_KeyRange) {
                    keyLo = g->amount.lo;
                    keyHi = g->amount.hi;
                }
                else if (g->gen == Gen_VelRange) {
                    veloLo = g->amount.lo;
                    veloHi = g->amount.hi;
                }
                else if (g->gen == Gen_Instrument)
                    instrIdx = g->amount.uword;
            }
            if (keyLo == 0
                    && keyHi == 127
                    && veloLo == 0
                    && veloHi == 127) {
                if (instrIdx == -1)
                    fprintf(f, "static PZone pz%d_%d(0);\n", idx, idx2);
                else
                    fprintf(f, "static PZone pz%d_%d(&instr%d);\n", idx, idx2, instrIdx);
            }
            else {
                if (instrIdx == -1)
                    fprintf(f, "static PZone pz%d_%d(%d, %d, %d, %d, 0);\n", idx, idx2, keyLo, keyHi, veloLo, veloHi);
                else
                    fprintf(f, "static PZone pz%d_%d(%d, %d, %d, %d, &instr%d);\n", idx, idx2, keyLo, keyHi, veloLo, veloHi, instrIdx);
            }
            ++idx2;
        }
        fprintf(f, "static PZone* pzones%d[%d] = {\n", idx, zones);
        idx2 = 0;
        for (int i = 0; i < zones; ++i) {
            fprintf(f, "      &pz%d_%d,\n", idx, idx2);
            ++idx2;
        }
        fprintf(f, "      };\n");

        fprintf(f, "static Preset preset%d(%d, %d, 0, %d, pzones%d);    // %s\n\n",
                idx, p->preset, p->bank, p->zones.Size(), idx, p->name);
        ++idx;
    }

    fprintf(f, "static Preset* sfPresets[%d] = {\n", pnums.Size());

    auto nnode = pnums.GetHeadNode();
    for (; nnode != nullptr; nnode = nnode->next)
    {
        int idx = nnode->elem;
        fprintf(f, "      &preset%d,   // %s\n", idx, presets[idx]->elem->name);
    }
    fprintf(f, "      };\n");

    fprintf(f, "static SFont _buildinSf(%d, sfPresets);\n", pnums.Size());
    fprintf(f, "SFont* createSf() { return &_buildinSf; }\n");
    fprintf(f, "} // end namespace\n");
    fclose(f);
    return true;
}

//---------------------------------------------------------
//   dumpPresets
//---------------------------------------------------------

void SoundFont::dumpPresets()
{
    int idx = 0;

    auto node = presets.GetHeadNode();
    for (; node != nullptr; node = node->next)
    {
        Preset* p = node->elem;
        printf("%03d %04x-%02x %s\n", idx, p->bank, p->preset, p->name);
        ++idx;
    }
}

struct VorbisData {
    int pos;
    ByteStream data;
};

static VorbisData vorbisData;
static size_t ovRead(void* ptr, size_t size, size_t nmemb, void* datasource);
static int ovSeek(void* datasource, ogg_int64_t offset, int whence);
static long ovTell(void* datasource);
static ov_callbacks ovCallbacks = { ovRead, ovSeek, 0, ovTell };

//---------------------------------------------------------
//   ovRead
//---------------------------------------------------------

static size_t ovRead(void* ptr, size_t size, size_t nmemb, void* datasource)
{
    VorbisData* vd = (VorbisData*)datasource;
    size_t n = size * nmemb;
    if (vd->data.size() < int(vd->pos + n))
        n = vd->data.size() - vd->pos;
    if (n) {
        const char* src = (char*)(vd->data.begin()) + vd->pos;
        memcpy(ptr, src, n);
        vd->pos += n;
    }

    return n;
}

//---------------------------------------------------------
//   ovSeek
//---------------------------------------------------------

static int ovSeek(void* datasource, ogg_int64_t offset, int whence)
{
    VorbisData* vd = (VorbisData*)datasource;
    switch(whence) {
    case SEEK_SET:
        vd->pos = offset;
        break;
    case SEEK_CUR:
        vd->pos += offset;
        break;
    case SEEK_END:
        vd->pos = vd->data.size() - offset;
        break;
    }
    return 0;
}

//---------------------------------------------------------
//   ovTell
//---------------------------------------------------------

static long ovTell(void* datasource)
{
    VorbisData* vd = (VorbisData*)datasource;
    return vd->pos;
}

//---------------------------------------------------------
//   writeUncompressedSample
//---------------------------------------------------------

int SoundFont::writeUncompressedSample(Sample* s)
{
    // Prepare input data
    std::ifstream t;
    char* buffer = nullptr;
    size_t length = 0;
    try
    {
        t.open(path, ios::in | ios::binary);
        t.seekg(0, std::ios::end);
        length = t.tellg();
        t.seekg(0, std::ios::beg);
        buffer = new char[length];
        t.read(buffer, length);
        t.close();
    }
    catch (exception)
    {
        t.close();
        cout << path << "文件打开出错!" << endl;
        return false;
    }

    ByteStream f(buffer, length);
    delete[] buffer;

    //
    f.setReadCursor(samplePos + s->start);
    int oggSize = s->end - s->start;
    short * ibuffer = new short[oggSize];
    f.read((byte*)ibuffer, 0, oggSize);

    vorbisData.pos  = 0;
    vorbisData.data = ByteStream(ibuffer, oggSize);


    OggVorbis_File vf;
    vorbisData.pos  = 0;
    string sep = "=";
    string amp = "AMP=";
    length = 0;
    if (ov_open_callbacks(&vorbisData, &vf, nullptr, 0, ovCallbacks) == 0)
    {
        double attenuation = 0;
        for (int i = 0; i < vf.vc->comments; i++)
        {
            string comment(vf.vc->user_comments[i]);
            if (contains(comment, amp))
            {
               vector<string> splitStrs = split(comment, sep);
                if (splitStrs.size() == 2)
                {
                    bool ok = false;
                    try {
                        attenuation = std::stod(splitStrs[1]);
                    }
                    catch (const std::invalid_argument& e) {
                        ok = false;
                    }
                    catch (const std::out_of_range& e) {
                        ok = false;
                    }

                    if (!ok)
                        attenuation = 0;
                }
            }
        }
        double linearAmp = pow(10.0, attenuation / 20.0);

        short buffer[2048];
        int numberRead = 0;
        int section = 0;
        do {
            numberRead = ov_read(&vf, (char *)buffer, 2048 * sizeof(short), 0, 2, 1, &section);
            for (unsigned int i = 0; i < numberRead / sizeof(short); i++)
                buffer[i] = (double)buffer[i] / linearAmp;
            write((char *)buffer, numberRead);
            length += numberRead;
        } while (numberRead);

        ov_clear(&vf);
    }

    delete [] ibuffer;
    return length;
}


int SoundFont::writeUncompressedSample(Sample* s, ByteStream& f)
{
    f.setReadCursor(samplePos + s->start);
    int oggSize = s->end - s->start;
    short* ibuffer = new short[oggSize];
    f.read((byte*)ibuffer, 0, oggSize);

    vorbisData.pos = 0;
    vorbisData.data = ByteStream(ibuffer, oggSize);


    OggVorbis_File vf;
    vorbisData.pos = 0;
    string sep = "=";
    string amp = "AMP=";
    int length = 0;

    if (ov_open_callbacks(&vorbisData, &vf, nullptr, 0, ovCallbacks) == 0)
    {
        double attenuation = 0;
        for (int i = 0; i < vf.vc->comments; i++)
        {
            string comment(vf.vc->user_comments[i]);
            if (contains(comment, amp))
            {
                vector<string> splitStrs = split(comment, sep);
                if (splitStrs.size() == 2)
                {
                    bool ok = false;
                    try {
                        attenuation = std::stod(splitStrs[1]);
                    }
                    catch (const std::invalid_argument& e) {
                        ok = false;
                    }
                    catch (const std::out_of_range& e) {
                        ok = false;
                    }

                    if (!ok)
                        attenuation = 0;
                }
            }
        }
        double linearAmp = pow(10.0, attenuation / 20.0);

        short buffer[2048];
        int numberRead = 0;
        int section = 0;
        do {
            numberRead = ov_read(&vf, (char*)buffer, 2048 * sizeof(short), 0, 2, 1, &section);
            if (linearAmp != 1) {
                for (unsigned int i = 0; i < numberRead / sizeof(short); i++)
                    buffer[i] = (double)buffer[i] / linearAmp;
            }
            write((char*)buffer, numberRead);
            length += numberRead;
        } while (numberRead);

        //根据sf2格式规范将在样本末尾多添加46个零值点,即92字节个0
        //sf24文档规范的6.1 Sample Data Format in the smpl Sub-chunk有提到
        short zeros[46] = { 0 };
        _file->write(zeros, 92);
        ov_clear(&vf);
    }

    delete[] ibuffer;
    return length;
}