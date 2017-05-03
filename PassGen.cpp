/*

A simple password generator.

Requirement: Visual Studio

License: MIT LICENSE

Copyright 2017 MaxBytes

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#include <Windows.h>
#include <Wincrypt.h>
#include <Windowsx.h>
#include <Commctrl.h>
#include <Tchar.h>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <ctime>
#include <random>
#include <string>
#include <vector>
#include "resource.h"

#ifdef UNICODE
typedef std::wstring tstring;
#else
typedef std::string tstring;
#endif

HCRYPTPROV prov;

std::mt19937 mersenne_twister;
unsigned int last_seeded_time;

// char_list contains characters which is to be used for generating password

tstring generate_random_string(std::vector<TCHAR> &char_list,int len,int RNG)
{
	unsigned char *buffer = 0;
	int n = char_list.size();
	tstring phrase;

	if (!n) return std::move(phrase); // return empty string

	switch (RNG)
	{
	case 0: /* Use CryptGenRandom */
		buffer = new unsigned char [len]();
		for(int i = 0;i < len;++i) *(buffer + i) = rand() & 0x7f;
		CryptGenRandom(prov,len,buffer);
		for(int i = 0;i < len;++i)
		{
			phrase += char_list[*(buffer + i) % n];
		}
		delete [] buffer;
		break;
	case 1:
		for(int i = 0;i < len;++i)
		{
			phrase += char_list[mersenne_twister() % n];
		}
		break;
	default:
		for(int i = 0;i < len;++i)
		{
			phrase += char_list[rand() % n];
		}
		break;
	}
	return phrase;
}

std::vector<TCHAR>& build_char_set(std::vector<TCHAR>& o,bool num,bool lower,bool capital,bool symbols,TCHAR *inclusion,TCHAR *exclusion)
{
	o.clear();
	if (num)
	{
		for(TCHAR c = 0x30;c < 0x3a;++c)
		{
			o.push_back(c);
		}
	}
	if (capital)
	{
		for(TCHAR c = 0x41;c < 0x5b;++c)
		{
			o.push_back(c);
		}
	}
	if (lower)
	{
		for(TCHAR c = 0x61;c < 0x7b;++c)
		{
			o.push_back(c);
		}
	}
	if (symbols)
	{
		for(TCHAR c = 0x21;c < 0x30;++c)
		{
			o.push_back(c);
		}
		for(TCHAR c = 0x3a;c < 0x41;++c)
		{
			o.push_back(c);
		}
		for(TCHAR c = 0x5b;c < 0x61;++c)
		{
			o.push_back(c);
		}
		for(TCHAR c = 0x7b;c < 0x7f;++c)
		{
			o.push_back(c);
		}
	}
	while(*inclusion)
	{
		o.push_back(*inclusion++);
	}
	auto it = std::remove_if(o.begin(),o.end(),[&](TCHAR c) -> bool { return NULL != _tcschr(exclusion,c); });
	o.erase(it,o.end());
	return o;
}

int calc_strength(tstring &s)
{
	double strength = 0.0;
	for(auto itrtr = s.begin();itrtr != s.end();++itrtr)
	{
		if (_istdigit(*itrtr))
		{
			strength += 3.321928;
		}
		else if (_istalpha(*itrtr))
		{
			strength += 4.7004397181410;
		}
		else strength += 5.0;
	}
	return static_cast<int>(strength);
}

void GenerateButtonPressed(HWND hWnd)
{
	std::vector<TCHAR> char_list;
	bool num,lower,capital,symbols;
	TCHAR *inclusion = 0;
	TCHAR *exclusion = 0;
	int len = 1,strength = 0;
	int rng = 0;
	TCHAR len_text[4];

	num = (((BST_CHECKED & Button_GetState(GetDlgItem(hWnd,IDC_DIGITS))) == BST_CHECKED) ? true : false);
	lower = (((BST_CHECKED  & Button_GetState(GetDlgItem(hWnd,IDC_LOWER))) == BST_CHECKED) ? true : false);
	capital = (((BST_CHECKED  & Button_GetState(GetDlgItem(hWnd,IDC_CAPITAL))) == BST_CHECKED) ? true : false);
	symbols = (((BST_CHECKED  & Button_GetState(GetDlgItem(hWnd,IDC_SYMBOLS))) == BST_CHECKED) ? true : false);
	inclusion = new TCHAR [GetWindowTextLength(GetDlgItem(hWnd,IDC_EDIT2)) + 1]();
	exclusion = new TCHAR [GetWindowTextLength(GetDlgItem(hWnd,IDC_EDIT3)) + 1]();
	GetWindowText(GetDlgItem(hWnd,IDC_EDIT2),inclusion,GetWindowTextLength(GetDlgItem(hWnd,IDC_EDIT2)) + 1);
	GetWindowText(GetDlgItem(hWnd,IDC_EDIT3),exclusion,GetWindowTextLength(GetDlgItem(hWnd,IDC_EDIT3)) + 1);
	build_char_set(char_list,num,lower,capital,symbols,inclusion,exclusion);

	if (BST_CHECKED == (BST_CHECKED & Button_GetState(GetDlgItem(hWnd,IDC_ONE))))
	{
		auto it = std::remove_if(char_list.begin(),char_list.end(),[=](TCHAR c) -> bool { return c == _TEXT('1'); });
		char_list.erase(it,char_list.end());
	}
	if (BST_CHECKED == (BST_CHECKED & Button_GetState(GetDlgItem(hWnd,IDC_L))))
	{
		auto it = std::remove_if(char_list.begin(),char_list.end(),[](TCHAR c) -> bool { return c == _TEXT('l'); });
		char_list.erase(it,char_list.end());
	}
	if (BST_CHECKED == (BST_CHECKED & Button_GetState(GetDlgItem(hWnd,IDC_I))))
	{
		auto it = std::remove_if(char_list.begin(),char_list.end(),[](TCHAR c) -> bool { return c == _TEXT('I'); });
		char_list.erase(it,char_list.end());
	}
	if (BST_CHECKED == (BST_CHECKED & Button_GetState(GetDlgItem(hWnd,IDC_VERTICAL))))
	{
		auto it = std::remove_if(char_list.begin(),char_list.end(),[](TCHAR c) -> bool { return c == _TEXT('|'); });
		char_list.erase(it,char_list.end());
	}
	if (BST_CHECKED == (BST_CHECKED & Button_GetState(GetDlgItem(hWnd,IDC_O))))
	{
		auto it = std::remove_if(char_list.begin(),char_list.end(),[](TCHAR c) -> bool { return c == _TEXT('O'); });
		char_list.erase(it,char_list.end());
	}
	if (BST_CHECKED == (BST_CHECKED & Button_GetState(GetDlgItem(hWnd,IDC_ZERO))))
	{
		auto it = std::remove_if(char_list.begin(),char_list.end(),[](TCHAR c) -> bool { return c == _TEXT('0'); });
		char_list.erase(it,char_list.end());
	}

	GetWindowText(GetDlgItem(hWnd,IDC_EDIT4),len_text,4);
	len = _tstoi(len_text);
	rng = ComboBox_GetCurSel(GetDlgItem(hWnd,IDC_ALGO));
	tstring phrase = generate_random_string(char_list,len,rng);
	strength = calc_strength(phrase);

	SetWindowText(GetDlgItem(hWnd,IDC_EDIT1),phrase.c_str());
	#ifdef UNICODE
	SetWindowText(GetDlgItem(hWnd,IDC_STRENGTH_TEXT),std::to_wstring((long long)strength).c_str());
	#else
	SetWindowText(GetDlgItem(hWnd,IDC_STRENGTH_TEXT),std::to_string((long long)strength).c_str());
	#endif

	if (strength < 32)
	{
		SendMessage(GetDlgItem(hWnd,IDC_PROGRESS1),PBM_SETBARCOLOR,0,RGB(255,0,0));
	}
	else if (strength < 64)
	{
		SendMessage(GetDlgItem(hWnd,IDC_PROGRESS1),PBM_SETBARCOLOR,0,RGB(255,128,0));
	}
	else if (strength < 96)
	{
		SendMessage(GetDlgItem(hWnd,IDC_PROGRESS1),PBM_SETBARCOLOR,0,RGB(255,255,0));
	}
	else if (strength < 128)
	{
		SendMessage(GetDlgItem(hWnd,IDC_PROGRESS1),PBM_SETBARCOLOR,0,RGB(0,255,0));
	}
	else
	{
		SendMessage(GetDlgItem(hWnd,IDC_PROGRESS1),PBM_SETBARCOLOR,0,RGB(0,0,255));
	}
	SendMessage(GetDlgItem(hWnd,IDC_PROGRESS1),PBM_SETPOS,strength,0);
	delete [] inclusion;
	delete [] exclusion;
}

void reseed(void)
{
	std::vector<unsigned> v;
	for(int i = 0;i < 32;++i) v.push_back(mersenne_twister());
	std::seed_seq seed(v.begin(),v.end());
	srand(last_seeded_time = GetTickCount());
	mersenne_twister.seed(seed);
}

unsigned timerID;


INT_PTR CALLBACK DialogProcedure(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	INT_PTR r = FALSE;
	switch(uMsg)
	{
	case WM_CLOSE:
		if (BST_CHECKED == (BST_CHECKED & Button_GetState(GetDlgItem(hWnd,IDC_CLEAR))))
		{
			if (OpenClipboard(hWnd))
			{
				EmptyClipboard();
				CloseClipboard();
			}
		}
		EndDialog(hWnd,0);
		r = TRUE;
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_ABOUT:
			MessageBox(hWnd,_TEXT("This is a simple password generator."),_TEXT("about"),MB_OK);
			r = TRUE;
			break;
		case IDC_GENERATE:
			GenerateButtonPressed(hWnd);
			r = TRUE;
			break;
		case IDC_PERIOD:
			if (EN_CHANGE != HIWORD(wParam)) break;
		case IDC_RESEED:
			if (BST_CHECKED != (BST_CHECKED & Button_GetState(GetDlgItem(hWnd,IDC_RESEED))))
			{
				KillTimer(hWnd,timerID);
			}
			else
			{
				TCHAR p[32];
				unsigned period = 0;
				GetWindowText(GetDlgItem(hWnd,IDC_PERIOD),p,32);
				period = _tstoi(p);
				timerID = SetTimer(hWnd,1,period*60000,0);
			}
			r = TRUE;
			break;
		}
		break;
	case WM_INITDIALOG:
		{
			if (!CryptAcquireContext(&prov,0,0,PROV_RSA_AES,0))
			{
				// error handling goes here
			}
			// initial seeding
			srand(last_seeded_time = time(0));
			std::seed_seq seq1(&last_seeded_time,&last_seeded_time + 1);
			mersenne_twister.seed(seq1);

			SendMessage(GetDlgItem(hWnd,IDC_PROGRESS1),PBM_SETRANGE32,0,256);

			SendMessage(GetDlgItem(hWnd,IDC_SPIN1),UDM_SETRANGE,0,MAKELPARAM(999,1));
			SetWindowText(GetDlgItem(hWnd,IDC_EDIT4),_TEXT("1"));

			SetWindowText(GetDlgItem(hWnd,IDC_PERIOD),_TEXT("10"));

			ComboBox_AddString(GetDlgItem(hWnd,IDC_ALGO),_TEXT("CryptGenRandom"));
			ComboBox_AddString(GetDlgItem(hWnd,IDC_ALGO),_TEXT("mt19937"));
			ComboBox_SetCurSel(GetDlgItem(hWnd,IDC_ALGO),1);

			SendMessage(hWnd,WM_SETICON,static_cast<WPARAM>(ICON_SMALL),reinterpret_cast<LPARAM>(LoadIcon(GetModuleHandle(0),MAKEINTRESOURCE(IDI_ICON1))));
		}
		break;
	case WM_TIMER:
		reseed();
		r = TRUE;
		break;
	default:
		break;
	}
	return r;
}

int CALLBACK _tWinMain(HINSTANCE hInst,HINSTANCE hPrevInst,LPWSTR lpCmdLine,int nShowCmd)
{
	DialogBox(hInst,MAKEINTRESOURCE(IDD_DIALOG1),0,DialogProcedure);
}

