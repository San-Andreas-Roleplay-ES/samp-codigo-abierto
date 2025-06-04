// SAN ANDREAS ROLEPLAY
// Copyright (c) 2021 - 2025 pigeon
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma warning disable 239 // ignore warning 239: literal array/string passed to a non-const parameter

#include <a_samp>
#include <a_mysql>
#include <a_http>
#include <crashdetect>
#include <colandreas>
#include <streamer>
#include <FCNPC>
#include <YSI_Coding\y_va>
#include <YSI_Data\y_iterate>
#include <YSI_Data\y_bit>
#include <YSI_Game\y_vehicledata>
#include <physics>
#include <Pawn.CMD>
#include <Pawn.Regex>
#include <sscanf2>
#include <samp_bcrypt>
#include <strlib>
#include <math>
#include <a_graphfunc>
#include <easyDialog>
#include <mSelection>
#include <components>
#include <float>
#include <VehiclePartPosition>
#include <weapon-data>

#define TDN_MODE_DEFAULT
#include <td-notification>

//defines
#define SERVER_NAME "San Andreas Roleplay"
#define SERVER_NICK "SA-RP" 
#define SERVER_DISCORD "discord.gg/sa-rp"
#define SERVER_URL "sarp.es"
#define SERVER_LANGUAGE "Espa�ol - Spanish"
#define SERVER_MAP "San Andreas"
#define SERVER_BRAND_COLOR "{ED801A}"
#define COPYRIGHT "Copyright (c) 2021 - 2025 San Andreas Roleplay"

//utils
#include "utils/defines"
#include "utils/open.mp"

//config
#include "config/timers"

//features

main() 
{
	print(""SERVER_NICK"");
	print(""COPYRIGHT"");
	print("  ");
}

public OnGameModeInit()
{
	SetGameModeText("SARP.es Open Source");
	return 1;
}
