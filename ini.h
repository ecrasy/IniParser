/*=============================================================================
#
# Author: carbon - ecras_y@163.com
#
# QQ : 1330186768
#
# Last modified: 2016-05-09 11:06
#
# Filename: ini.h
#
# Description: header file for class declaration
# CIniParser is a final class should not be inherited any more
#
=============================================================================*/
#ifndef __CARBON_INI_PARSER_H
#define __CARBON_INI_PARSER_H

#include <map>
#include <vector>
#include <fstream>

#include "stdstring.h"
#include "Utf8_16.h"

typedef struct _tag_IniVal {
    bool bModify;
    CStdString strVal;

    _tag_IniVal()
        : bModify(false)
    { }

} IniVal;

typedef std::map<CStdString, IniVal> IniSection;
typedef std::map<CStdString, IniSection> IniData;

#ifndef _WIN32
typedef const char* LPCTSTR;
typedef unsigned char BYTE;

#define _T(x) x
#define CHAR_SLASH '/'
#else
#define CHAR_SLASH '\\'
#endif

/* 
 * [section1]
 * key1=value1
 * key2=value2
 * [section2]
 * key1=value2
*/
class CIniParser
{
public:
    CIniParser(LPCTSTR lpIniFile = _T(""));
    ~CIniParser();

    bool Parse(LPCTSTR lpIniFile = _T(""));
    bool SetValue(const CStdString& strSection, const CStdString& strKey, const CStdString& strVal);
    bool RemoveKey(const CStdString& strSection, const CStdString& strKey);
    bool RemoveSection(const CStdString& strSection);

    void DumpIni(void);

    bool Save();

protected:
    bool Parse(const std::vector<BYTE>& buf);
    bool IsComment(const CStdString& strLine);
    bool GetIniValue(const CStdString& strLine, CStdString& strKey, IniVal& iniVal);
    bool IsSectionLine(const CStdString& strLine, CStdString& strSection);
    CStdString GetLineString(const std::vector<BYTE>& buf, int& pos);

    bool IsEmptySection(const CStdString& strSection);
    bool IsLineModified(const CStdString& strLine, const CStdString& strSection, CStdString& strKey);
    bool WriteModifiedLine(std::ofstream& fout, const CStdString& strSection, const CStdString& strKey);
    bool WriteSectionNewLine(std::ofstream& fout, const CStdString& strSection);
    bool WriteSectionOldLine(std::ofstream& fout, CStdString& strSection, CStdString& strOldSection, const std::string& s);
    bool WriteNewSection(std::ofstream& fout);

private:
    CStdString  m_strIniFile;
    bool        m_bParse;
    bool        m_bUtf8Bom;
    bool        m_bModified;
    bool        m_bDosFormat;
    IniData     m_iniData;
};

#endif //__CARBON_INI_PARSER_H
