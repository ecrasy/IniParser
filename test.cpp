/*=============================================================================
#
# Author: carbon - ecras_y@163.com
#
# QQ : 1330186768
#
# Last modified: 2016-05-09 11:11
#
# Filename: test.cpp
#
# Description: demo for testing test.ini
#
=============================================================================*/
#include <iostream>
#include "ini.h"
#include "stdstring.h"

CStdString GetFileName(const CStdString& filepath)
{
    CStdString strName = filepath;
    int nfind = strName.ReverseFind(CHAR_SLASH);
    if (nfind != -1)
    {
        strName = strName.Mid(nfind + 1);
    }

    return strName;
}

CStdString GetFilePath(const CStdString& filepath)
{
    CStdString strPath = filepath;
    int nfind = strPath.ReverseFind(CHAR_SLASH);
    if (nfind != -1)
    {
        strPath = strPath.Mid(0, nfind + 1);
    }

    return strPath;
}

CStdString GetFileExt(const CStdString& filepath)
{
    CStdString strExt;
    int nfind = filepath.ReverseFind('.');
    if (nfind != -1)
    {
        strExt = filepath.Mid(nfind + 1);
    }

    return strExt;
}

int main(int argc, char* argv[])
{
    CStdString filepath = "/home/liuhuan/test/ini/test.ini";
    std::cout << GetFileName(filepath).c_str() << "\n";
    std::cout << GetFilePath(filepath).c_str() << "\n";
    std::cout << GetFileExt(filepath).c_str() << "\n";

    CIniParser cp("/home/liuhuan/test/ini/test.ini");
    if(cp.Parse())
    {
        cp.DumpIni();
        if(argc > 1)
        {
            cp.SetValue(_T("carbon"), _T("vol"), _T("setvol"));
            cp.SetValue(_T("god"), _T("vol"), _T("setvol"));
            cp.SetValue(_T("new"), _T("vol"), _T("setvol"));
            cp.SetValue(_T("new"), _T("hello"), _T("setvol"));
            cp.RemoveKey(_T("carbon"), _T("joke"));
            cp.RemoveSection(_T("remove"));
            cp.DumpIni();
        }
    }

    return 0;
}

