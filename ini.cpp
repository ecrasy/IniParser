/*=============================================================================
#
# Author: carbon - ecras_y@163.com
#
# QQ : 1330186768
#
# Last modified: 2016-05-09 11:08
#
# Filename: ini.cpp
#
# Description: CIniParser implementation
# CStdString a class emulate CString & std::string
# Utf8_16_Read from notepadd++ dealing with text encoding(utf8, unicode le & be)
#
=============================================================================*/
#include <iostream>
#include "ini.h"

CIniParser::CIniParser(LPCTSTR lpIniFile)
    : m_strIniFile(lpIniFile)
    , m_bParse(false)
    , m_bUtf8Bom(false)
    , m_bModified(false)
    , m_bDosFormat(false)
{
}

CIniParser::~CIniParser()
{
    Save();
}

bool CIniParser::Save()
{
    if (m_bModified)
    {
        m_bModified = false;
        IniData tmpData = m_iniData;
        
        CStdString tmpFile = m_strIniFile + _T(".tmp");
        std::ofstream fout;
        fout.open(tmpFile.c_str(),
            std::ios_base::trunc | std::ios_base::out | std::ios_base::binary);

        std::ifstream fin;
        fin.open(m_strIniFile.c_str(),
            std::ios_base::in | std::ios_base::binary);

        if (fin.good() && fout.good())
        {
            if (m_bUtf8Bom)
            {
                unsigned char bom[3] = { 0xef, 0xbb, 0xbf };
                fout.write((char*)bom, 3);
                fin.seekg(3, fin.beg);
            }

            CStdString strSection;
            CStdString strOldSection;
            std::string s;
            
            while (!fin.eof())
            {
                if (std::getline(fin, s).good())
                {
                    WriteSectionOldLine(fout, strSection, strOldSection, s);
                }
            }

            WriteNewSection(fout);
            fout.close();

#ifdef _WIN32
            MoveFileEx(tmpFile, m_strIniFile, MOVEFILE_REPLACE_EXISTING);
#else
            rename(tmpFile.c_str(), m_strIniFile.c_str());
#endif
        }
        else
        {
            return false;
        }
        
        m_iniData = tmpData;
        for (auto& kvp : m_iniData) {
            for (auto& ivp : kvp.second) {
                ivp.second.bModify = false;
            }
        }
        
        return true;
    }

    return true;
}

bool CIniParser::IsLineModified(const CStdString& strLine, const CStdString& strSection, CStdString& strKey)
{
    if (IsComment(strLine)) return false;

    IniVal iniVal;
    if (GetIniValue(strLine, strKey, iniVal))
    {
        auto it = m_iniData.find(strSection);
        if (it != m_iniData.end())
        {
            auto itsub = it->second.find(strKey);
            if (itsub != it->second.end())
            {
                return itsub->second.bModify;
            }
        }
    }

    return false;
}

bool CIniParser::IsEmptySection(const CStdString& strSection)
{
    auto it = m_iniData.find(strSection);
    if (it != m_iniData.end())
    {
        return it->second.empty();
    }

    return true;
}

bool CIniParser::WriteSectionOldLine(std::ofstream& fout, CStdString& strSection, CStdString& strOldSection, const std::string& s)
{
    CStdString strLine = s.c_str();
    static bool bEmptySection = false;
    strLine.Trim();
    strOldSection = strSection;

    if (IsSectionLine(strLine, strSection))
    {
        //write new key value pair to old section
        WriteSectionNewLine(fout, strOldSection);

        if (IsEmptySection(strSection))
        {
            //ignore section
            bEmptySection = true;
        }
        else
        {
            bEmptySection = false;
            //write section
            fout.write(s.c_str(), s.size());
            fout.write(("\n"), 1);
        }
        printf("section line [%s] in empty section: %d\n", strLine.c_str(), bEmptySection);
    }
    else
    {
        printf("normal line [%s] in empty section: %d\n", strLine.c_str(), bEmptySection);
        if (bEmptySection == false)
        {
            CStdString strKey;
            if (IsLineModified(strLine, strSection, strKey))
            {
                WriteModifiedLine(fout, strSection, strKey);
            }
            else
            {
                fout.write(s.c_str(), s.size());
                fout.write(("\n"), 1);
            }

            m_iniData[strSection].erase(strKey);
        }
    }

    return true;
}

bool CIniParser::WriteModifiedLine(std::ofstream& fout, const CStdString& strSection, const CStdString& strKey)
{
    CStdString strLine;
    IniSection& is = m_iniData[strSection];
    CStdString& strVal = is[strKey].strVal;

    if (strVal.IsEmpty()) {
        return true;
    }

    if (m_bDosFormat) strLine.Format(("%s=%s\r\n"), strKey.c_str(), strVal.c_str());
    else strLine.Format(("%s=%s\n"), strKey.c_str(), strVal.c_str());
    fout.write(strLine.c_str(), strLine.GetLength());

    return true;
}

bool CIniParser::WriteSectionNewLine(std::ofstream& fout, const CStdString& strSection)
{
    if (strSection.IsEmpty() == false)
    {
        auto it = m_iniData.find(strSection);
        if (it != m_iniData.end())
        {
            for (const auto& ivp : it->second)
            {
                CStdString strLine;
                if (m_bDosFormat) strLine.Format(("%s=%s\r\n"), ivp.first.c_str(), ivp.second.strVal.c_str());
                else strLine.Format(("%s=%s\n"), ivp.first.c_str(), ivp.second.strVal.c_str());

                fout.write(strLine.c_str(), strLine.GetLength());
            }

            m_iniData.erase(it);
        }
    }

    m_iniData.erase(strSection);

    return true;
}

bool CIniParser::WriteNewSection(std::ofstream& fout)
{
    for (const auto& kvp : m_iniData)
    {
        if (kvp.second.empty()) continue;

        if (m_bDosFormat) fout << "[" << kvp.first.c_str() << "]\r\n";
        else fout << "[" << kvp.first.c_str() << "]\n";
        for (const auto& ivp : kvp.second)
        {
            if (m_bDosFormat) fout << ivp.first.c_str() << "=" << ivp.second.strVal.c_str() << "\r\n";
            else fout << ivp.first.c_str() << "=" << ivp.second.strVal.c_str() << "\n";
        }
    }
    
    return true;
}

bool CIniParser::Parse(LPCTSTR lpIniFile /* = _T("") */)
{
    //if file name same as old name
    if (m_strIniFile == lpIniFile) {
        if (m_bParse) return m_iniData.empty() == false;
    }

    //else go through
    CStdString strTmpName = lpIniFile;
    if (strTmpName.IsEmpty()) //if file name is empty
    {
        if (m_bParse) return m_iniData.empty() == false;
    }
    else m_strIniFile = strTmpName;

    m_iniData.clear();
    m_bParse = false;
    m_bUtf8Bom = false;

    if (m_strIniFile.IsEmpty()) return m_bParse;

    std::ifstream infile;
    infile.open(m_strIniFile.c_str(), std::ios_base::binary | std::ios_base::in);
    if (infile.good())
    {
        infile.seekg(0, infile.end);
        std::streamoff len = infile.tellg();
        infile.seekg(0, infile.beg);

        if (len > 0)
        {
            std::vector<BYTE> buf;
            buf.resize(static_cast<size_t>(len));
            infile.read((char *)&buf[0], len);
            infile.close();
            UniMode encoding = Utf8_16_Read::determineEncoding(&buf[0], buf.size());
            if (encoding == uniUTF8) m_bUtf8Bom = true;
            else if (encoding == uni16BE || encoding == uni16LE)
            {
                //Convert buffer to utf8
                Utf8_16_Read convertor;
                size_t nret = convertor.convert((char*)&buf[0], buf.size());
                buf.resize(nret);
                memcpy(&buf[0], convertor.getNewBuf(), nret);

                std::ofstream fout;
                fout.open(m_strIniFile.c_str(),
                    std::ios_base::binary | std::ios_base::out | std::ios_base::trunc);
                if (fout.good())
                {
                    unsigned char bom[3] = { 0xef, 0xbb, 0xbf };
                    fout.write((char*)bom, 3);
                    fout.write((char*)(&buf[0]), buf.size());
                    fout.close();
                }
            }

            Parse(buf);
        }
    }

    m_bParse = true;

    return m_iniData.empty() == false;
}


CStdString CIniParser::GetLineString(const std::vector<BYTE>& buf, int& posBegin)
{
    int buflen = static_cast<int>(buf.size());
    if (buf.empty() || posBegin >= buflen)
    {
        posBegin = 0;
        return ("");
    }
    else
    {
        if (posBegin < 0) posBegin = 0;
        CStdString strResult;
        auto it = std::find(buf.begin() + posBegin, buf.end(), '\n');

        if (it != buf.end())
        {
            strResult.append(buf.begin() + posBegin, it + 1);
            if (m_bDosFormat == false)
            {
                if (strResult.Find(("\r\n")) != -1)
                    m_bDosFormat = true;
            }

            posBegin = it - buf.begin() + 1;

            strResult.Trim();
            return strResult;
        }
        else
        {
            strResult.append(buf.begin() + posBegin, buf.end());
            strResult.Trim();
            posBegin = buf.size();
            return strResult;
        }
    }
}

bool CIniParser::IsSectionLine(const CStdString& strLine, CStdString& strSection)
{
    if (strLine.Left(1) != ("[")) return false;

    char cleft = '[';
    char cright = ']';
    int n1 = strLine.Find(cleft);
    if (n1 != -1)
    {
        int n2 = strLine.Find(cright, n1 + 1);
        if (n2 != -1)
        {
            CStdString tmpValue;
            tmpValue = strLine.Mid(n1 + 1, n2 - n1 - 1);
            tmpValue.Trim();
            if (tmpValue.IsEmpty() == false)
            {
                strSection = tmpValue;
                return true;
            }
        }
    }

    return false;
}

bool CIniParser::GetIniValue(const CStdString& strLine, CStdString& strKey, IniVal& iniVal)
{
    int nfind = strLine.Find('=');
    if (nfind != -1)
    {
        strKey = strLine.Mid(0, nfind);
        strKey.Trim();
        if (strKey.IsEmpty() == false)
        {
            iniVal.strVal = strLine.Mid(nfind + 1);
            iniVal.strVal.Trim();
            return true;
        }
    }

    return false;
}

bool CIniParser::IsComment(const CStdString& strLine)
{
    if (strLine.Left(1) == (";")) return true;
    else return false;
}

bool CIniParser::Parse(const std::vector<BYTE>& buf)
{
    CStdString strLine;
    int pos = 0;
    int buflen = static_cast<int>(buf.size());
    if (m_bUtf8Bom) pos += 3;

    CStdString strSection;
    CStdString strNewSection;
    IniSection sectionData;

    while (pos < buflen)
    {
        strLine = GetLineString(buf, pos);
        if (strLine.IsEmpty() == false)
        {
            if (IsComment(strLine.c_str()) == false)
            {
                if (IsSectionLine(strLine, strNewSection))  //new section coming
                {
                    if (!strSection.IsEmpty())   //save old section data
                    {
                        m_iniData.insert(std::make_pair(strSection, sectionData));
                    }

                    //deal with coming section
                    sectionData.clear();
                    strSection = strNewSection;
                }
                else
                {
                    if (strSection.IsEmpty()) continue;

                    //deal with coming section
                    strNewSection.clear();
                    CStdString strKey;
                    IniVal iniValue;
                    if (GetIniValue(strLine, strKey, iniValue))
                    {
                        sectionData.insert(std::make_pair(strKey, iniValue));
                    }
                }
            }   //if (IsComment(strLine.c_str()) == false)
        }   //if (strLine.IsEmpty() == false)
    }   //while (pos < buflen)

    //save last section data
    m_iniData.insert(std::make_pair(strSection, sectionData));

    return true;
}

bool CIniParser::SetValue(const CStdString& strSection, const CStdString& strKey, const CStdString& strVal)
{
    auto it = m_iniData.find(strSection);
    if (it != m_iniData.end())  //find section
    {
        auto itsub = it->second.find(strKey);
        if (itsub != it->second.end())  //find key
        {
            if ((strVal.IsEmpty() == false) && (itsub->second.strVal == strVal)) return true;
            else it->second.erase(itsub);
        }

        {
            // if key exist and value is not equal, remove exist key value pair
            // add new key value pair to exist section
            IniVal iniVal;
            iniVal.strVal = strVal;
            iniVal.bModify = true;
            it->second.insert(std::make_pair(strKey, iniVal));
        }
    }
    else    //add new key to new setion
    {
        IniSection newSection;
        IniVal iniVal;
        iniVal.strVal = strVal;
        iniVal.bModify = true;
        newSection.insert(std::make_pair(strKey, iniVal));
        m_iniData.insert(std::make_pair(strSection, newSection));
    }

    m_bModified = true;

    return true;
}

bool CIniParser::RemoveKey(const CStdString& strSection, const CStdString& strKey)
{
    return SetValue(strSection, strKey, (""));
}

bool CIniParser::RemoveSection(const CStdString& strSection)
{
    auto it = m_iniData.find(strSection);
    if (it != m_iniData.end())  //find section
    {
        it->second.clear();
        m_iniData.erase(it);
    }

    m_bModified = true;

    return true;
}

void CIniParser::DumpIni(void)
{
    std::cout << "dos format: " << (m_bDosFormat ? "true" : "false") << "\n";
    for (const auto& kvp : m_iniData)
    {
        std::cout << "[" << kvp.first.c_str() << "]\n";
        for (const auto& ivp : kvp.second)
        {
            std::cout << ivp.first.c_str() << "=" << ivp.second.strVal.c_str() << "\n";
        }
        std::cout << "\n";
    }
}

