// ZenLib::Ztring - std::(w)string is better
// Copyright (C) 2002-2008 Jerome Martinez, Zen@MediaArea.net
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// More methods for std::(w)string
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#include "ZenLib/Conf_Internal.h"
#ifdef __BORLANDC__
    #pragma hdrstop
#endif
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#ifdef ZENLIB_USEWX
    #include <wx/strconv.h>
    #include <wx/datetime.h>
#else //ZENLIB_USEWX
    #ifdef ZENLIB_STANDARD
        #undef WINDOWS
    #endif
    #ifdef WINDOWS
        #undef __TEXT
        #include <windows.h>
        #include <tchar.h>
    #endif
#endif //ZENLIB_USEWX
#ifdef __MINGW32__
    #include <windows.h>
#endif //__MINGW32__
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cmath>
#include "ZenLib/Ztring.h"
#include <ZenLib/OS_Utils.h>
#include <ZenLib/ConvertUTF.h>
using namespace std;
//---------------------------------------------------------------------------

namespace ZenLib
{

//---------------------------------------------------------------------------
Ztring EmptyZtring;
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
typedef basic_stringstream<Char>  tStringStream;
typedef basic_istringstream<Char> tiStringStream;
typedef basic_ostringstream<Char> toStringStream;
//---------------------------------------------------------------------------

//***************************************************************************
// Operators
//***************************************************************************

Char &Ztring::operator() (size_type Pos)
{
    if (Pos>size())
        resize(Pos);
    return operator[] (Pos);
}

//***************************************************************************
// Conversions
//***************************************************************************

Ztring& Ztring::From_Unicode (const wchar_t* S)
{
    if (S==NULL)
        return *this;

    #ifdef _UNICODE
        assign(S);
    #else
        #ifdef ZENLIB_USEWX
            size_type OK=wxConvCurrent->WC2MB(NULL, S, 0);
            if (OK!=0 && OK!=Error)
                assign(wxConvCurrent->cWC2MB(S));
        #else //ZENLIB_USEWX
            #ifdef WINDOWS
                if (IsWin9X())
                {
                    clear();
                    return *this; //Is not possible, UTF-8 is not supported by Win9X
                }
                int Size=WideCharToMultiByte(CP_UTF8, 0, S, -1, NULL, 0, NULL, NULL);
                if (Size!=0)
                {
                    char* AnsiString=new char[Size+1];
                    WideCharToMultiByte(CP_UTF8, 0, S, -1, AnsiString, Size, NULL, NULL);
                    AnsiString[Size]='\0';
                    assign (AnsiString);
                    delete[] AnsiString;
                }
                else
                    clear();
            #else //WINDOWS
                size_t Size=wcstombs(NULL, S, 0);
                if (Size!=0 && Size!=(size_t)-1)
                {
                    char* AnsiString=new char[Size+1];
                    Size=wcstombs(AnsiString, S, wcslen(S));
                    AnsiString[Size]='\0';
                    assign (AnsiString);
                    delete[] AnsiString;
                }
                else
                    clear();
            #endif
        #endif //ZENLIB_USEWX
    #endif
    return *this;
}

Ztring& Ztring::From_Unicode (const wchar_t *S, size_type Start, size_type Length)
{
    if (S==NULL)
        return *this;

    if (Length==Error)
        Length=wcslen(S+Start);
    wchar_t* Temp=new wchar_t[Length+1];
    wcsncpy (Temp, S+Start, Length);
    Temp[Length]=_T('\0');

    From_Unicode(Temp);
    delete[] Temp; //Temp=NULL;
    return *this;
}

Ztring& Ztring::From_UTF8 (const char* S)
{
    if (S==NULL)
        return *this;

    #ifdef ZENLIB_USEWX
        size_type OK=wxConvUTF8.MB2WC(NULL, S, 0);
        if (OK!=0 && OK!=Error)
            #ifdef _UNICODE
                assign(wxConvUTF8.cMB2WC(S).data());
            #else
                assign(wxConvCurrent->cWC2MB(wxConvUTF8.cMB2WC(S)));
            #endif
    #else //ZENLIB_USEWX
        #ifdef _UNICODE
            #ifdef WINDOWS
                if (IsWin9X())
                {
                    const UTF8*  S_Begin          =(const UTF8*)S;
                    size_t       S_Size           =strlen(S);
                    const UTF8*  S_End            =(const UTF8*)(S+S_Size+1);
                    Char*        WideString       =new Char[S_Size+1];
                                 WideString[0]    =_T('\0');
                    Char*        WideString_Copy  =WideString;
                    UTF16*       WideString_Begin =(UTF16*)WideString_Copy;
                    UTF16*       WideString_End   =(UTF16*)(WideString+S_Size+1);
                    if (sizeof(wchar_t)==2)
                        ConvertUTF8toUTF16(&S_Begin, S_End, &WideString_Begin, WideString_End, lenientConversion);
                    else
                    {
                        clear();
                        return *this;
                    }
                    size_t Size=wcslen(WideString);
                    WideString[Size]=L'\0';
                    assign (WideString);
                    delete[] WideString; //WideString=NULL;
                }
                else
                {
                    int Size=MultiByteToWideChar(CP_UTF8, 0, S, -1, NULL, 0);
                    if (Size!=0)
                    {
                        Char* WideString=new Char[Size+1];
                        MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, S, -1, WideString, Size);
                        WideString[Size]=L'\0';
                        assign (WideString);
                        delete[] WideString; //WideString=NULL;
                    }
                    else
                        clear();
                }
            #else //WINDOWS
                size_t Size=mbstowcs(NULL, S, 0);
                if (Size!=0 && Size!=(size_t)-1)
                {
                    wchar_t* WideString=new wchar_t[Size+1];
                    Size=mbstowcs(WideString, S, strlen(S));
                    WideString[Size]=L'\0';
                    assign (WideString);
                    delete[] WideString; //WideString=NULL;
                }
                else
                    clear();
            #endif
        #else
            assign(S); //Not implemented
        #endif
    #endif //ZENLIB_USEWX
    return *this;
}

Ztring& Ztring::From_UTF8 (const char* S, size_type Start, size_type Length)
{
    if (S==NULL)
        return *this;

    if (Length==Error)
        Length=strlen(S+Start);
    char* Temp=new char[Length+1];
    strncpy (Temp, S+Start, Length);
    Temp[Length]='\0';

    From_UTF8(Temp);
    delete[] Temp; //Temp=NULL;
    return *this;
}

Ztring& Ztring::From_UTF16 (const char* S)
{
    if (S==NULL)
        return *this;

         if ((unsigned char)S[0]==(unsigned char)0xFF && (unsigned char)S[1]==(unsigned char)0xFE)
        return From_UTF16LE(S+2);
    else if ((unsigned char)S[0]==(unsigned char)0xFE && (unsigned char)S[1]==(unsigned char)0xFF)
        return From_UTF16BE(S+2);
    else if ((unsigned char)S[0]==(unsigned char)0x00 && (unsigned char)S[1]==(unsigned char)0x00)
    {
        clear(); //No begin!
        return *this;
    }
    else
        return From_UTF16LE(S); //Not sure, default
}

Ztring& Ztring::From_UTF16 (const char* S, size_type Start, size_type Length)
{
    if (S==NULL)
        return *this;

    if (Length<2)
        return *this;

         if ((unsigned char)S[0]==(unsigned char)0xFF && (unsigned char)S[1]==(unsigned char)0xFE)
        return From_UTF16LE(S+2, Start, Length-2);
    else if ((unsigned char)S[0]==(unsigned char)0xFE && (unsigned char)S[1]==(unsigned char)0xFF)
        return From_UTF16BE(S+2, Start, Length-2);
    else if ((unsigned char)S[0]==(unsigned char)0x00 && (unsigned char)S[1]==(unsigned char)0x00)
    {
        clear(); //No begin!
        return *this;
    }
    else
        return From_UTF16LE(S, Start, Length); //Not sure, default
}

Ztring& Ztring::From_UTF16BE (const char* S)
{
    if (S==NULL)
        return *this;

    #ifdef ZENLIB_USEWX
        //clear(); return *this;
        wxMBConvUTF16BE wxConvUTF16BE;
        size_type OK=wxConvUTF16BE.MB2WC(NULL, S, 0);
        if (OK!=0 && OK!=Error)
            #ifdef _UNICODE
                assign(wxConvUTF16BE.cMB2WC(S).data());
            #else
                assign(wxConvCurrent->cWC2MB(wxConvUTF16BE.cMB2WC(S)));
            #endif
    #else //ZENLIB_USEWX
        #ifdef WINDOWS
            clear();
            const wchar_t* SW=(const wchar_t*)S;
            size_t Pos=0;
            while (SW[Pos]!=_T('\0'))
            {
                wchar_t Temp=(wchar_t)(((SW[Pos]&0xFF00)>>8)+((SW[Pos]&0x00FF)<<8));
                append(1, Temp);
                Pos++;
            }
        #else //WINDOWS
            clear(); //Not implemented
        #endif
    #endif //ZENLIB_USEWX
    return *this;
}

Ztring& Ztring::From_UTF16BE (const char* S, size_type Start, size_type Length)
{
    if (S==NULL)
        return *this;

    if (Length==Error)
    {
        Length=0;
        while(S[Length]!=0x0000)
            Length++;
    }

    char* Temp=new char[Length+2];
    memcpy (Temp, S+Start, Length);
    Temp[Length+0]=0x00;
    Temp[Length+1]=0x00;
    reserve(Length);
    From_UTF16BE(Temp);
    delete[] Temp; //Temp=NULL;
    return *this;
}

Ztring& Ztring::From_UTF16LE (const char* S)
{
    if (S==NULL)
        return *this;

    #ifdef ZENLIB_USEWX
        //clear(); return *this;
        wxMBConvUTF16LE wxConvUTF16LE;
        size_type OK=wxConvUTF16LE.MB2WC(NULL, S, 0);
        if (OK!=0 && OK!=Error)
            #ifdef _UNICODE
                assign(wxConvUTF16LE.cMB2WC(S).data());
            #else
                assign(wxConvCurrent->cWC2MB(wxConvUTF16LE.cMB2WC(S)));
            #endif
    #else //ZENLIB_USEWX
        #ifdef WINDOWS
            #ifdef UNICODE
                const wchar_t* SW=(const wchar_t*)S;
                assign(SW);
            #else
                clear(); //Not implemented
            #endif
        #else //WINDOWS
            clear(); //Not implemented
        #endif
    #endif //ZENLIB_USEWX
    return *this;
}

Ztring& Ztring::From_UTF16LE (const char* S, size_type Start, size_type Length)
{
    if (S==NULL)
        return *this;

    if (Length==Error)
    {
        Length=0;
        while(S[Length]!=0x0000)
            Length+=2;
    }

    char* Temp=new char[Length+2];
    memcpy (Temp, S+Start, Length);
    Temp[Length+0]=0x00;
    Temp[Length+1]=0x00;
    From_UTF16LE(Temp);
    delete[] Temp; //Temp=NULL;
    return *this;
}

Ztring& Ztring::From_Local (const char* S)
{
    if (S==NULL)
        return *this;

    #ifdef _UNICODE
        #ifdef ZENLIB_USEWX
            size_type OK=wxConvCurrent->MB2WC(NULL, S, 0);
            if (OK!=0 && OK!=Error)
                assign(wxConvCurrent->cMB2WC(S).data());
        #else //ZENLIB_USEWX
            #ifdef WINDOWS
                int Size=MultiByteToWideChar(CP_ACP, 0, S, -1, NULL, 0);
                if (Size!=0)
                {
                    wchar_t* WideString=new wchar_t[Size+1];
                    MultiByteToWideChar(CP_ACP, 0, S, -1, WideString, Size);
                    WideString[Size]=L'\0';
                    assign (WideString);
                    delete[] WideString; //WideString=NULL;
                }
                else
                    clear();
            #else //WINDOWS
                size_t Size=mbstowcs(NULL, S, 0);
                if (Size!=0 && Size!=(size_t)-1)
                {
                    wchar_t* WideString=new wchar_t[Size+1];
                    Size=mbstowcs(WideString, S, Size);
                    WideString[Size]=L'\0';
                    assign (WideString);
                    delete[] WideString; //WideString=NULL;
                }
                else
                    clear();
            #endif
        #endif //ZENLIB_USEWX
    #else
        assign(S);
    #endif
    return *this;
}

Ztring& Ztring::From_Local (const char* S, size_type Start, size_type Length)
{
    if (S==NULL)
        return *this;

    if (Length==Error)
        Length=strlen(S+Start);
    #ifdef _UNICODE
        char* Temp=new char[Length+1];
        strncpy (Temp, S+Start, Length);
        Temp[Length]='\0';
        From_Local(Temp);
        delete[] Temp; //Temp=NULL;
    #else
        assign(S+Start, Length);
        if (find(_T('\0'))!=std::string::npos)
            resize(find(_T('\0')));
    #endif
    return *this;
}

Ztring& Ztring::From_UUID (const int128u S)
{
    Ztring S1;
    S1.From_CC2((int16u)((S.hi&0x00000000FFFF0000LL)>>16)); assign(S1);
    S1.From_CC2((int16u)( S.hi&0x000000000000FFFFLL     )); append(S1); append(_T("-"));
    S1.From_CC2((int16u)((S.hi&0x0000FFFF00000000LL)>>32)); append(S1); append(_T("-"));
    S1.From_CC2((int16u)((S.hi&0xFFFF000000000000LL)>>48)); append(S1); append(_T("-"));
    S1.From_CC2((int16u)((S.lo&0xFFFF000000000000LL)>>48)); append(S1); append(_T("-"));
    S1.From_CC2((int16u)((S.lo&0x0000FFFF00000000LL)>>32)); append(S1);
    S1.From_CC2((int16u)((S.lo&0x00000000FFFF0000LL)>>16)); append(S1);
    S1.From_CC2((int16u)( S.lo&0x000000000000FFFFLL     )); append(S1);

    return *this;
}

Ztring& Ztring::From_CC4 (const int32u S)
{
    std::string S1;
    S1.append(1, (char)((S&0xFF000000)>>24));
    S1.append(1, (char)((S&0x00FF0000)>>16));
    S1.append(1, (char)((S&0x0000FF00)>> 8));
    S1.append(1, (char)((S&0x000000FF)>> 0));
    From_Local(S1.c_str());

    //Test
    if (empty())
        assign(_T("(empty)"));

    return *this;
}

Ztring& Ztring::From_CC3 (const int32u S)
{
    std::string S1;
    S1.append(1, (char)((S&0x00FF0000)>>16));
    S1.append(1, (char)((S&0x0000FF00)>> 8));
    S1.append(1, (char)((S&0x000000FF)>> 0));
    From_Local(S1.c_str());

    //Test
    if (empty())
        assign(_T("(empty)"));

    return *this;
}

Ztring& Ztring::From_CC2 (const int16u S)
{
    clear();
    Ztring Pos1; Pos1.From_Number(S, 16);
    resize(4-Pos1.size(), _T('0'));
    append(Pos1);
    MakeUpperCase();

    return *this;
}

Ztring& Ztring::From_CC1 (const int8u S)
{
    clear();
    Ztring Pos1; Pos1.From_Number(S, 16);
    resize(2-Pos1.size(), _T('0'));
    append(Pos1);
    MakeUpperCase();

    return *this;
}

Ztring& Ztring::From_Number (const int8s I, int8u Radix)
{
    #ifdef __MINGW32__
        if (Radix==0)
        {
            clear();
            return *this;
        }
        Char* C1=new Char[33];
        _itot (I, C1, Radix);
        assign (C1);
        delete[] C1; //C1=NULL;
    #else
        toStringStream SS;
        #ifdef UNICODE
            SS << setbase(Radix) << I;
        #else //UNICODE
            SS << setbase(Radix) << (size_t)I; //On linux (at least), (un)signed char is detected as a char
        #endif //UNICODE
        assign(SS.str());
    #endif
    MakeUpperCase();
    return *this;
}

Ztring& Ztring::From_Number (const int8u I, int8u Radix)
{
    #ifdef __MINGW32__
        if (Radix==0)
        {
            clear();
            return *this;
        }
        Char* C1=new Char[33];
        _ultot (I, C1, Radix);
        assign (C1);
        delete[] C1; //C1=NULL;
    #else
        toStringStream SS;
        #ifdef UNICODE
            SS << setbase(Radix) << I;
        #else //UNICODE
            SS << setbase(Radix) << (size_t)I; //On linux (at least), (un)signed char is detected as a char
        #endif //UNICODE
        assign(SS.str());
    #endif
    MakeUpperCase();
    return *this;
}

Ztring& Ztring::From_Number (const int16s I, int8u Radix)
{
    #ifdef __MINGW32__
        if (Radix==0)
        {
            clear();
            return *this;
        }
        Char* C1=new Char[33];
        _itot (I, C1, Radix);
        assign (C1);
        delete[] C1; //C1=NULL;
    #else
        toStringStream SS;
        SS << setbase(Radix) << I;
        assign(SS.str());
    #endif
    MakeUpperCase();
    return *this;
}

Ztring& Ztring::From_Number (const int16u I, int8u Radix)
{
    #ifdef __MINGW32__
        if (Radix==0)
        {
            clear();
            return *this;
        }
        Char* C1=new Char[33];
        _ultot (I, C1, Radix);
        assign (C1);
        delete[] C1; //C1=NULL;
    #else
        toStringStream SS;
        SS << setbase(Radix) << I;
        assign(SS.str());
    #endif
    MakeUpperCase();
    return *this;
}

Ztring& Ztring::From_Number (const int32s I, int8u Radix)
{
    #ifdef __MINGW32__
        if (Radix==0)
        {
            clear();
            return *this;
        }
        Char* C1=new Char[33];
        _itot (I, C1, Radix);
        assign (C1);
        delete[] C1; //C1=NULL;
    #else
        toStringStream SS;
        SS << setbase(Radix) << I;
        assign(SS.str());
    #endif
    MakeUpperCase();
    return *this;
}

Ztring& Ztring::From_Number (const int32u I, int8u Radix)
{
    #ifdef __MINGW32__
        if (Radix==0)
        {
            clear();
            return *this;
        }
        Char* C1=new Char[33];
        _ultot (I, C1, Radix);
        assign (C1);
        delete[] C1; //C1=NULL;
    #else
        toStringStream SS;
        SS << setbase(Radix) << I;
        assign(SS.str());
    #endif
    MakeUpperCase();
    return *this;
}

Ztring& Ztring::From_Number (const int64s I, int8u Radix)
{
    #ifdef __MINGW32__
        if (Radix==0)
        {
            clear();
            return *this;
        }
        Char* C1=new Char[65];
        _i64tot (I, C1, Radix);
        assign (C1);
        delete[] C1; //C1=NULL;
    #else
        toStringStream SS;
        SS << setbase(Radix) << I;
        assign(SS.str());
    #endif
    MakeUpperCase();
    return *this;
}

Ztring& Ztring::From_Number (const int64u I, int8u Radix)
{
    #ifdef __MINGW32__
        if (Radix==0)
        {
            clear();
            return *this;
        }
        Char* C1=new Char[65];
        _ui64tot (I, C1, Radix);
        assign (C1);
        delete[] C1; //C1=NULL;
    #else
        toStringStream SS;
        SS << setbase(Radix) << I;
        assign(SS.str());
    #endif
    MakeUpperCase();
    return *this;
}

Ztring& Ztring::From_Number (const int128u I, int8u Radix)
{
    From_Local(I.toString(Radix));

    return *this;
}

Ztring& Ztring::From_Number (const float32 F, int8u Precision, ztring_t Options)
{
    #ifdef __MINGW32__
        Char C1[100];
        #if defined (_UNICODE)
            snwprintf (C1, 99, (Ztring(_T("%."))+Ztring::ToZtring(Precision)+_T("f")).c_str(), F);
        #else
            snprintf  (C1, 99, (Ztring(_T("%."))+Ztring::ToZtring(Precision)+_T("f")).c_str(), F);
        #endif
        assign(C1);
    #else
        toStringStream SS;
        SS << setprecision(Precision) << fixed << F;
        assign(SS.str());
    #endif

    if ((Options & Ztring_NoZero && size()>0) && find(_T('.'))>0)
    {
        while (size()>0 && ((*this)[size()-1]==_T('0')))
            resize(size()-1);
        if (size()>0 && (*this)[size()-1]==_T('.'))
            resize(size()-1);
    }

    return *this;
}

Ztring& Ztring::From_Number (const float64 F, int8u Precision, ztring_t Options)
{
    #ifdef __MINGW32__
        Char C1[100];
        #if defined (_UNICODE)
            snwprintf (C1, 99, (Ztring(_T("%."))+Ztring::ToZtring(Precision)+_T("f")).c_str(), F);
        #else
            snprintf  (C1, 99, (Ztring(_T("%."))+Ztring::ToZtring(Precision)+_T("f")).c_str(), F);
        #endif
        assign(C1);
    #else
        toStringStream SS;
        SS << setprecision(Precision) << fixed << F;
        assign(SS.str());
    #endif

    if ((Options & Ztring_NoZero && size()>0) && find(_T('.'))>0)
    {
        while (size()>0 && ((*this)[size()-1]==_T('0')))
            resize(size()-1);
        if (size()>0 && (*this)[size()-1]==_T('.'))
            resize(size()-1);
    }

    return *this;
}

Ztring& Ztring::From_Number (const float80 F, int8u Precision, ztring_t Options)
{
    #ifdef __MINGW32__
        Char C1[100];
        #if defined (_UNICODE)
            snwprintf (C1, 99, (Ztring(_T("%."))+Ztring::ToZtring(Precision)+_T("f")).c_str(), F);
        #else
            snprintf  (C1, 99, (Ztring(_T("%."))+Ztring::ToZtring(Precision)+_T("f")).c_str(), F);
        #endif
        assign(C1);
    #else
        toStringStream SS;
        SS << setprecision(Precision) << fixed << F;
        assign(SS.str());
    #endif

    if ((Options & Ztring_NoZero && size()>0) && find(_T('.'))>0)
    {
        while (size()>0 && ((*this)[size()-1]==_T('0')))
            resize(size()-1);
        if (size()>0 && (*this)[size()-1]==_T('.'))
            resize(size()-1);
    }

    return *this;
}

#ifdef NEED_SIZET
Ztring& Ztring::From_Number (const size_t I, int8u Radix)
{
    toStringStream SS;
    SS << setbase(Radix) << I;
    assign(SS.str());
    MakeUpperCase();
    return *this;
}
#endif //NEED_SIZET

//---------------------------------------------------------------------------
Ztring& Ztring::Duration_From_Milliseconds (const int64u Value)
{
    int64u HH=(int8u)(Value/1000/60/60);
    int64u MM=Value/1000/60   -((HH*60));
    int64u SS=Value/1000      -((HH*60+MM)*60);
    int64u MS=Value           -((HH*60+MM)*60+SS)*1000;
    Ztring DateT;
    Ztring Date;
    DateT.From_Number(HH); if (DateT.size()<2){DateT=Ztring(_T("0"))+DateT;}
    Date+=DateT;
    Date+=_T(":");
    DateT.From_Number(MM); if (DateT.size()<2){DateT=Ztring(_T("0"))+DateT;}
    Date+=DateT;
    Date+=_T(":");
    DateT.From_Number(SS); if (DateT.size()<2){DateT=Ztring(_T("0"))+DateT;}
    Date+=DateT;
    Date+=_T(".");
    DateT.From_Number(MS); if (DateT.size()<2){DateT=Ztring(_T("00"))+DateT;} else if (DateT.size()<3){DateT=Ztring(_T("0"))+DateT;}
    Date+=DateT;
    assign (Date.c_str());
    return *this;
}

Ztring& Ztring::Date_From_Milliseconds_1601 (const int64u Value)
{
    if (Value>=11644473600000LL) //Values <1970 are not supported
    {
        Date_From_Seconds_1970((int32u)((Value-11644473600000LL)/1000));
        append(_T("."));
        Ztring Milliseconds; Milliseconds.From_Number(Value%1000);
        while (Milliseconds.size()<3)
            Milliseconds+=_T('0');
        append(Milliseconds);
    }
    else
        clear(); //Not supported

    return *this;
}

Ztring& Ztring::Date_From_Seconds_1601 (const int64u Value)
{
    if (Value>=11644473600LL) //Values <1970 are not supported
        Date_From_Seconds_1970((int32u)(Value-11644473600LL));
    else
        clear(); //Not supported

    return *this;
}

Ztring& Ztring::Date_From_Seconds_1904 (const int64u Value)
{
    #ifdef ZENLIB_USEWX
        /*
        wxDateTime Date;
        Date.SetYear(1904);
        Date.SetMonth(wxDateTime::Jan);
        Date.SetDay(1);
        Date.SetHour(0);
        Date.SetMinute(0);
        Date.SetSecond(0);
        if (Value>=0x80000000)
        {
            //wxTimeSpan doesn't support unsigned int
            int64u Value2=Value;
            while (Value2>0x7FFFFFFF)
            {
                Date+=wxTimeSpan::Seconds(0x7FFFFFFF);
                Value2-=0x7FFFFFFF;
            }
            Date+=wxTimeSpan::Seconds(Value2);
        }
        else
            Date+=wxTimeSpan::Seconds(Value);

        Ztring ToReturn=_T("UTC ");
        ToReturn+=Date.FormatISODate();
        ToReturn+=_T(" ");
        ToReturn+=Date.FormatISOTime();

        assign (ToReturn.c_str());
        */ //WxDateTime is buggy???
        if (Value>=2082844800 && Value<2082844800+0x100000000LL) //Values <1970 and >2038 are not supported
            Date_From_Seconds_1970((int32u)(Value-2082844800));
        else
            clear(); //Not supported

    #else //ZENLIB_USEWX
        if (Value>=2082844800 && Value<2082844800+0x100000000LL) //Values <1970 and >2038 are not supported
            Date_From_Seconds_1970((int32u)(Value-2082844800));
        else
            clear(); //Not supported
    #endif //ZENLIB_USEWX
    return *this;
}

Ztring& Ztring::Date_From_Seconds_1970 (const int32u Value)
{
    time_t Time=(time_t)Value;
    struct tm *Gmt=gmtime(&Time);
    Ztring DateT;
    Ztring Date=_T("UTC ");
    Date+=Ztring::ToZtring((Gmt->tm_year+1900));
    Date+=_T("-");
    DateT.From_Number(Gmt->tm_mon+1); if (DateT.size()<2){DateT=Ztring(_T("0"))+Ztring::ToZtring(Gmt->tm_mon+1);}
    Date+=DateT;
    Date+=_T("-");
    DateT.From_Number(Gmt->tm_mday); if (DateT.size()<2){DateT=Ztring(_T("0"))+Ztring::ToZtring(Gmt->tm_mday);}
    Date+=DateT;
    Date+=_T(" ");
    DateT.From_Number(Gmt->tm_hour); if (DateT.size()<2){DateT=Ztring(_T("0"))+Ztring::ToZtring(Gmt->tm_hour);}
    Date+=DateT;
    Date+=_T(":");
    DateT=Ztring::ToZtring(Gmt->tm_min); if (DateT.size()<2){DateT=Ztring(_T("0"))+Ztring::ToZtring(Gmt->tm_min);}
    Date+=DateT;
    Date+=_T(":");
    DateT.From_Number(Gmt->tm_sec); if (DateT.size()<2){DateT=Ztring(_T("0"))+Ztring::ToZtring(Gmt->tm_sec);}
    Date+=DateT;
    assign (Date.c_str());
    return *this;
}

Ztring& Ztring::Date_From_String (const char* Value, size_t Value_Size)
{
    //Only the year
    if (Value_Size<10)
    {
        From_Local(Value, 0, Value_Size);
        return *this;
    }

    #ifdef ZENLIB_USEWX
        Ztring ToReturn=_T("UTC ");
        wxDateTime Date;
        Ztring DateS;
        DateS.From_Local(Value, Value_Size).c_str();
        if (!DateS.empty() && DateS[DateS.size()-1]==_T('\n'))
            DateS.resize(DateS.size()-1);

        //Some strange formating : exactly 24 bytes (or 25 with 0x0A at the end) and Year is at the end
        if (DateS.size()==24 && DateS[23]>=_T('0') && DateS[23]<=_T('9') && DateS[21]>=_T('0') && DateS[21]<=_T('9') && DateS[19]==_T(' '))
            Date.ParseFormat(DateS.c_str(), _T("%a %b %d %H:%M:%S %Y"));
        //ISO date
        else if (DateS.size()==10 && (DateS[4]<_T('0') || DateS[4]>_T('9')) && (DateS[7]<_T('0') || DateS[7]>_T('9')))
        {
            DateS[4]=_T('-');
            DateS[7]=_T('-');
            ToReturn+=DateS;
        }
        //Default
        else
            Date.ParseDateTime(DateS.c_str());

        if (ToReturn.size()<5)
        {
            ToReturn+=Date.FormatISODate();
            ToReturn+=_T(" ");
            ToReturn+=Date.FormatISOTime();
        }
        else if (ToReturn.size()<5)
            ToReturn+=DateS;

        assign (ToReturn.c_str());
    #else //ZENLIB_USEWX
        Ztring DateS; DateS.From_Local(Value, 0, Value_Size);
        if (DateS.size()==20 && DateS[4]==_T('-') && DateS[7]==_T('-') && DateS[10]==_T('T') && DateS[13]==_T(':') && DateS[16]==_T(':') && DateS[19]==_T('Z'))
        {
            DateS.resize(19);
            DateS[10]=_T(' ');
            assign(_T("UTC "));
            append(DateS);
        }
        else
            From_Local(Value, 0, Value_Size); //Not implemented
    #endif //ZENLIB_USEWX
    return *this;
}

//---------------------------------------------------------------------------
std::wstring Ztring::To_Unicode () const
{
    #ifdef _UNICODE
        return c_str();
    #else //_UNICODE
        #ifdef ZENLIB_USEWX
            return wxConvCurrent->cMB2WC(c_str()).data();
        #else //ZENLIB_USEWX
            return std::wstring(); //Not implemented
        #endif //ZENLIB_USEWX
    #endif //_UNICODE
}

std::string Ztring::To_UTF8 () const
{
    #ifdef _UNICODE
        #ifdef ZENLIB_USEWX
            return wxConvUTF8.cWC2MB(c_str()).data();
        #else //ZENLIB_USEWX
            #ifdef WINDOWS
                if (IsWin9X())
                {
                    const Char*   S                =c_str();
                    const UTF16*  S_Begin          =(const UTF16*)S;
                    size_t        S_Size           =size();
                    const UTF16*  S_End            =(const UTF16*)(S+S_Size+1);
                    char*         AnsiString       =new char[S_Size*4+1];
                                  AnsiString[0]    ='\0';
                    char*         AnsiString_Copy  =AnsiString;
                    UTF8*         AnsiString_Begin =(UTF8*)AnsiString_Copy;
                    UTF8*         AnsiString_End   =(UTF8*)(AnsiString+S_Size*4+1);
                    if (sizeof(wchar_t)==2)
                        ConvertUTF16toUTF8(&S_Begin, S_End, &AnsiString_Begin, AnsiString_End, lenientConversion);
                    else
                        return std::string();
                    size_t Size=strlen(AnsiString);
                    AnsiString[Size]='\0';
                    std::string ToReturn(AnsiString);
                    delete[] AnsiString; //AnsiString=NULL;
                    return ToReturn;
                }
                else
                {
                    int Size=WideCharToMultiByte(CP_UTF8, 0, c_str(), -1, NULL, 0, NULL, NULL);
                    if (Size!=0)
                    {
                        char* AnsiString=new char[Size+1];
                        WideCharToMultiByte(CP_UTF8, 0, c_str(), -1, AnsiString, Size, NULL, NULL);
                        AnsiString[Size]='\0';
                        std::string ToReturn(AnsiString);
                        delete[] AnsiString; //AnsiString=NULL;
                        return ToReturn;
                    }
                    else
                        return std::string();
                }
            #else //WINDOWS
                size_t Size=wcstombs(NULL, c_str(), 0);
                if (Size!=0 && Size!=(size_t)-1)
                {
                    char* AnsiString=new char[Size+1];
                    Size=wcstombs(AnsiString, c_str(), size());
                    AnsiString[Size]='\0';
                    std::string ToReturn(AnsiString, 0, Size);
                    delete[] AnsiString; //AnsiString=NULL;
                    return ToReturn;
                }
                else
                    return std::string();
            #endif
        #endif //ZENLIB_USEWX
    #else
        #ifdef ZENLIB_USEWX
            return wxConvUTF8.cWC2MB(wxConvCurrent->cMB2WC(c_str())).data();
        #else //ZENLIB_USEWX
            return c_str(); //Not implemented
        #endif //ZENLIB_USEWX
    #endif
}

std::string Ztring::To_Local () const
{
    #ifdef _UNICODE
        #ifdef ZENLIB_USEWX
            wxCharBuffer C=wxConvCurrent->cWC2MB(c_str());
            if (C.data())
                return C.data();
            else
                return std::string();
        #else //ZENLIB_USEWX
            #ifdef WINDOWS
                int Size=WideCharToMultiByte(CP_ACP, 0, c_str(), -1, NULL, 0, NULL, NULL);
                if (Size!=0)
                {
                    char* AnsiString=new char[Size+1];
                    WideCharToMultiByte(CP_ACP, 0, c_str(), -1, AnsiString, Size, NULL, NULL);
                    AnsiString[Size]='\0';
                    std::string ToReturn(AnsiString);
                    delete[] AnsiString; //AnsiString=NULL;
                    return ToReturn;
                }
                else
                    return std::string();
            #else //WINDOWS
                size_t Size=wcstombs(NULL, c_str(), 0);
                if (Size!=0 && Size!=(size_t)-1)
                {
                    char* AnsiString=new char[Size+1];
                    Size=wcstombs(AnsiString, c_str(), Size);
                    AnsiString[Size]='\0';
                    std::string ToReturn(AnsiString);
                    delete[] AnsiString; //AnsiString=NULL;
                    return ToReturn;
                }
                else
                    return std::string();
            #endif
        #endif //ZENLIB_USEWX
    #else
        return c_str();
    #endif
}

//---------------------------------------------------------------------------
//Operateur ToInt
int32s Ztring::To_int8s (ztring_t Options) const
{
    //Integrity
    if (empty())
        return 0;

    //Conversion
    int32s I;
    #ifdef __MINGW32__
        I=_ttoi(c_str());
    #else
        tStringStream SS(*this);
        SS >> I;
        if (SS.fail())
            return 0;
    #endif

    //Rounded
    if (Options==Ztring_Rounded && find(_T("."))!=Error)
    {
        float80 F=To_float80();
        F-=I;
        if (F>=0.5)
            return I+1;
    }

    return I;
}

//---------------------------------------------------------------------------
//Operateur ToInt
int32u Ztring::To_int8u (ztring_t Options) const
{
    //Integrity
    if (empty())
        return 0;

    //Conversion
    int32u I;
    #ifdef __MINGW32__
        I=_ttoi64(c_str()); //TODO : I>0x7FFFFFFF - Replaced by i64 version to support, but not good
    #else
        tStringStream SS(*this);
        SS >> I;
        if (SS.fail())
            return 0;
    #endif

    //Rounded
    if (Options==Ztring_Rounded && find(_T("."))!=std::string::npos)
    {
        float32 F=To_float32();
        F-=I;
        if (F>=0.5)
            return I+1;
    }

    return I;
}

//---------------------------------------------------------------------------
//Operateur ToInt
int32s Ztring::To_int16s (ztring_t Options) const
{
    //Integrity
    if (empty())
        return 0;

    //Conversion
    int32s I;
    #ifdef __MINGW32__
        I=_ttoi(c_str());
    #else
        tStringStream SS(*this);
        SS >> I;
        if (SS.fail())
            return 0;
    #endif

    //Rounded
    if (Options==Ztring_Rounded && find(_T("."))!=Error)
    {
        float80 F=To_float80();
        F-=I;
        if (F>=0.5)
            return I+1;
    }

    return I;
}

//---------------------------------------------------------------------------
//Operateur ToInt
int32u Ztring::To_int16u (ztring_t Options) const
{
    //Integrity
    if (empty())
        return 0;

    //Conversion
    int32u I;
    #ifdef __MINGW32__
        I=_ttoi64(c_str()); //TODO : I>0x7FFFFFFF - Replaced by i64 version to support, but not good
    #else
        tStringStream SS(*this);
        SS >> I;
        if (SS.fail())
            return 0;
    #endif

    //Rounded
    if (Options==Ztring_Rounded && find(_T("."))!=std::string::npos)
    {
        float32 F=To_float32();
        F-=I;
        if (F>=0.5)
            return I+1;
    }

    return I;
}

//---------------------------------------------------------------------------
//Operateur ToInt
int32s Ztring::To_int32s (ztring_t Options) const
{
    //Integrity
    if (empty())
        return 0;

    //Conversion
    int32s I;
    #ifdef __MINGW32__
        I=_ttoi(c_str());
    #else
        tStringStream SS(*this);
        SS >> I;
        if (SS.fail())
            return 0;
    #endif

    //Rounded
    if (Options==Ztring_Rounded && find(_T("."))!=Error)
    {
        float80 F=To_float80();
        F-=I;
        if (F>=0.5)
            return I+1;
    }

    return I;
}

//---------------------------------------------------------------------------
//Operateur ToInt
int32u Ztring::To_int32u (ztring_t Options) const
{
    //Integrity
    if (empty())
        return 0;

    //Conversion
    int32u I;
    #ifdef __MINGW32__
        I=_ttoi64(c_str()); //TODO : I>0x7FFFFFFF - Replaced by i64 version to support, but not good
    #else
        tStringStream SS(*this);
        SS >> I;
        if (SS.fail())
            return 0;
    #endif

    //Rounded
    if (Options==Ztring_Rounded && find(_T("."))!=std::string::npos)
    {
        float32 F=To_float32();
        F-=I;
        if (F>=0.5)
            return I+1;
    }

    return I;
}

//---------------------------------------------------------------------------
//Operateur ToInt
int64s Ztring::To_int64s (ztring_t Options) const
{
    //Integrity
    if (empty())
        return 0;

    //Conversion
    int64s I;
    #ifdef __MINGW32__
        I=_ttoi64(c_str());
    #else
        tStringStream SS(*this);
        SS >> I;
        if (SS.fail())
            return 0;
    #endif

    //Rounded
    if (Options==Ztring_Rounded && find(_T("."))!=std::string::npos)
    {
        float32 F=To_float32();
        F-=I;
        if (F>0.5)
            return I+1;
    }

    return I;
}

//---------------------------------------------------------------------------
//Operateur ToInt
int64u Ztring::To_int64u (ztring_t Options) const
{
    //Integrity
    if (empty())
        return 0;

    //Conversion
    int64u I;
    #ifdef __MINGW32__
        I=_ttoi64(c_str()); //TODO : I>0x7FFFFFFFFFFFFFFF
    #else
        tStringStream SS(*this);
        SS >> I;
        if (SS.fail())
            return 0;
    #endif

    //Rounded
    if (Options==Ztring_Rounded && find(_T("."))!=std::string::npos)
    {
        float32 F=To_float32();
        F-=I;
        if (F>=0.5)
            return I+1;
    }

    return I;
}

//---------------------------------------------------------------------------
//Operateur ToFloat
float32 Ztring::To_float32(ztring_t) const
{
    //Integrity
    if (empty())
        return 0;

    //Conversion
    #ifdef __MINGW32__
        #ifdef UNICODE
            return (wcstod(c_str(),NULL));
        #else
            return (strtod(c_str(),NULL));
        #endif
    #else
        float32 F;
        tStringStream SS(*this);
        SS >> F;
        if (SS.fail())
            return 0;

        return F;
    #endif
}

//---------------------------------------------------------------------------
//Operateur ToFloat
float64 Ztring::To_float64(ztring_t) const
{
    //Integrity
    if (empty())
        return 0;

    //Conversion
    #ifdef __MINGW32__
        #ifdef UNICODE
            return (wcstod(c_str(),NULL));
        #else
            return (strtod(c_str(),NULL));
        #endif
    #else
        float64 F;
        tStringStream SS(*this);
        SS >> F;
        if (SS.fail())
            return 0;

        return F;
    #endif
}

//---------------------------------------------------------------------------
//Operateur ToFloat
float80 Ztring::To_float80(ztring_t) const
{
    //Integrity
    if (empty())
        return 0;

    //Conversion
    #ifdef __MINGW32__
        #ifdef UNICODE
            return (wcstod(c_str(),NULL));
        #else
            return (strtod(c_str(),NULL));
        #endif
    #else
        float80 F;
        tStringStream SS(*this);
        SS >> F;
        if (SS.fail())
            return 0;

        return F;
    #endif
}

//***************************************************************************
// Edition
//***************************************************************************

//---------------------------------------------------------------------------
// Retourne une partie de la chaine
Ztring Ztring::SubString (const tstring &Begin, const tstring &End, size_type Pos, ztring_t Options) const
{
    //Recherche D�but
    size_type I_Debut=find(Begin, Pos);
    if (I_Debut==Error)
        return _T("");
    I_Debut+=Begin.size();

    //gestion fin NULL
    if (End==_T(""))
        return substr(I_Debut);

    //Recherche Fin
    size_type I_Fin=find(End, I_Debut);
    if (I_Fin==Error)
    {
        if (Options & Ztring_AddLastItem)
            return substr(I_Debut);
        else
            return _T("");
    }

    return substr(I_Debut, I_Fin-I_Debut);
}

//---------------------------------------------------------------------------
//FindAndReplace
Ztring::size_type Ztring::FindAndReplace (const ZenLib::tstring &ToFind, const ZenLib::tstring &ReplaceBy, size_type Pos, ZenLib::ztring_t Options)
{
   size_type Count=0;
   size_type Middle=Pos;
   while (!(Count==1 && !(Options&Ztring_Recursive)) && (Middle=find(ToFind, Middle))!=npos)
   {
      replace(Middle, ToFind.length(), ReplaceBy);
      Middle += ReplaceBy.length();
      Count++;
   }

    return Count;
}

//---------------------------------------------------------------------------
//test if it is a number
bool Ztring::IsNumber() const
{
    if (empty())
        return false;

    bool OK=true;
    size_t Size=size();
    for (size_t Pos=0; Pos<Size; Pos++)
        if (operator[](Pos)<_T('0') || operator[](Pos)>_T('9'))
        {
            OK=false;
            break;
        }
    return OK;
}

//---------------------------------------------------------------------------
//Mise en minuscules
Ztring &Ztring::MakeLowerCase()
{
    transform(begin(), end(), begin(), (int(*)(int))tolower); //(int(*)(int)) is a patch for unix
    return *this;
}

//---------------------------------------------------------------------------
// Mise en majuscules
Ztring &Ztring::MakeUpperCase()
{
    transform(begin(), end(), begin(), (int(*)(int))toupper); //(int(*)(int)) is a patch for unix
    return *this;
}

//---------------------------------------------------------------------------
// Remove leading whitespaces from a string
Ztring &Ztring::TrimLeft(Char ToTrim)
{
    size_type First=0;
    while (operator[](First)==ToTrim)
        First++;
    assign (c_str()+First);
    return *this;
}

//---------------------------------------------------------------------------
// Remove trailing whitespaces from a string
Ztring &Ztring::TrimRight(Char ToTrim)
{
    size_type Last=size()-1;
    while (operator[](Last)==ToTrim)
        Last--;
    assign (c_str(), Last+1);
    return *this;
}

//---------------------------------------------------------------------------
// Remove leading and trailing whitespaces from a string
Ztring &Ztring::Trim(Char ToTrim)
{
    TrimLeft(ToTrim);
    TrimRight(ToTrim);
    return *this;
}

//---------------------------------------------------------------------------
// Quotes a string
Ztring &Ztring::Quote(Char ToTrim)
{
    assign(tstring(1, ToTrim)+c_str()+ToTrim);
    return *this;
}

//***************************************************************************
// Information
//***************************************************************************

//---------------------------------------------------------------------------
//Count
Ztring::size_type Ztring::Count (const Ztring &ToCount, ztring_t) const
{
    size_type Count=0;
    for (size_type Pos=0; Pos<=size(); Pos++)
        if (find(ToCount, Pos)!=npos)
        {
            Count++;
            Pos+=ToCount.size()-1; //-1 because the loop will add 1
        }
    return Count;
}

//---------------------------------------------------------------------------
//Compare
bool Ztring::Compare (const Ztring &ToCompare, const Ztring &Comparator, ztring_t Options) const
{
    //Integers management
    if (IsNumber() && ToCompare.IsNumber())
    {
        int64s Left=To_int64s();
        int64s Right=ToCompare.To_int64s();
        if (Comparator==_T("==")) return (Left==Right);
        if (Comparator==_T("<"))  return (Left< Right);
        if (Comparator==_T("<=")) return (Left<=Right);
        if (Comparator==_T(">=")) return (Left>=Right);
        if (Comparator==_T(">"))  return (Left> Right);
        if (Comparator==_T("!=")) return (Left!=Right);
        if (Comparator==_T("<>")) return (Left!=Right);
        return false;
    }

    //Case sensitive option
    if (!(Options & Ztring_CaseSensitive))
    {
        //Need to copy strings and make it lowercase
        Ztring Left (c_str());
        Ztring Right (ToCompare.c_str());
        Left.MakeLowerCase();
        Right.MakeLowerCase();

        //string comparasion
        if (Comparator==_T("==")) return (Left==Right);
        if (Comparator==_T("IN")) {if (Left.find(Right)!=string::npos) return true; else return false;}
        if (Comparator==_T("<"))  return (Left< Right);
        if (Comparator==_T("<=")) return (Left<=Right);
        if (Comparator==_T(">=")) return (Left>=Right);
        if (Comparator==_T(">"))  return (Left> Right);
        if (Comparator==_T("!=")) return (Left!=Right);
        if (Comparator==_T("<>")) return (Left!=Right);
        return false;
    }
    else
    {
        //string comparasion
        if (Comparator==_T("==")) return (*this==ToCompare);
        if (Comparator==_T("IN")) {if (this->find(ToCompare)!=string::npos) return true; else return false;}
        if (Comparator==_T("<"))  return (*this< ToCompare);
        if (Comparator==_T("<=")) return (*this<=ToCompare);
        if (Comparator==_T(">=")) return (*this>=ToCompare);
        if (Comparator==_T(">"))  return (*this> ToCompare);
        if (Comparator==_T("!=")) return (*this!=ToCompare);
        if (Comparator==_T("<>")) return (*this!=ToCompare);
        return false;
    }
}

} //namespace
