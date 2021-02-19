#ifndef _VentrueFont_h_
#define _VentrueFont_h_
#include "scutils/Utils.h"
#include <iostream>
#include <fstream>
#include <tinyxml2\tinyxml2.h>
#include"Synth/Generator.h"
#include"Synth/SoundFontParser.h"
using namespace tinyxml2;

namespace ventrue
{
    enum VentrueXmlUnitType
    {
        UnitType_Ventrue,
        UnitType_SoundFont,
        UnitType_Polyphone
    };

    class VentrueFont :public SoundFontParser
    {
    public:
        VentrueFont(Ventrue* ventrue)
            :SoundFontParser(ventrue)
        {
        }

        ~VentrueFont();
        /// <summary>
        /// 解析乐器配置文件到ventrue
        /// </summary>
        void Parse(string filePath);

    private:
        /// <summary>
        /// 解析单位类型
        /// </summary>
        void ParseUnitType();

        /// <summary>  
        /// 解析样本列表    
        /// </summary>      
        void ParseSampleList();

        /// <summary>
        /// 解析乐器列表
        /// </summary>
        void ParseInstrumentList();

        /// <summary>
        /// 解析预设列表
        /// </summary>
        void ParsePresetList();

        void SetGenListByXml(GeneratorList& genList, tinyxml2::XMLElement* regionChildElem, RegionType regionType);

        inline bool IsEqual(const char* s1, const char* s2);

    private:
        tinyxml2::XMLDocument* xmlDoc = nullptr;
        string path;
        string pathRoot;
        VentrueXmlUnitType unitType = VentrueXmlUnitType::UnitType_Ventrue;
    };
}

#endif
