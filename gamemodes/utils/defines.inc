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

#define function%0(%1) forward%0(%1); public%0(%1)

#define PRESSED(%0) (((newkeys & (%0)) == (%0)) && ((oldkeys & (%0)) != (%0)))
#define RELEASED(%0) (((newkeys & (%0)) != (%0)) && ((oldkeys & (%0)) == (%0)))
#define HOLDING(%0) ((newkeys & (%0)) == (%0))	

#define RandomEx(%0,%1)                     (random((%1) - (%0)) + (%0))

stock randarg( ... )
	return getarg( random( numargs( ) ) );

#define Kick(%0) SetTimerEx("promsters_KickTimer", 500, false, "i", %0)
native promsters_Kick(playerid) = Kick;

forward promsters_KickTimer(playerid);
public promsters_KickTimer(playerid) promsters_Kick(playerid);

#define DEFAULT_COLOR           (0xA9C4E4FF)
#define COLOR_GREY              (0xD2D3D5FF)
#define COLOR_GRAD1 		    (0xB4B5B7FF)
#define COLOR_GRAD2 		    (0xBFC0C2FF)
#define COLOR_GRAD3 		    (0xCBCCCEFF)
#define COLOR_GRAD4 		    (0xD8D8D8FF)
#define COLOR_GRAD5 		    (0xE3E3E3FF)
#define COLOR_FADE1             (0xE6E6E6E6)
#define COLOR_FADE2             (0xC8C8C8C8)
#define COLOR_FADE3             (0xAFAFAFFF)
#define COLOR_FADE4             (0x8C8C8C8C)
#define COLOR_FADE5             (0x6E6E6E6E)
#define COLOR_WHITE             (0xFFFFFFFF)
#define COLOR_WHITE2            (0xE2FFFFFF)
#define COLOR_WHITE3            (0xCCE6E6FF)
#define COLOR_RED               (0xFF0000FF)
#define COLOR_LIGHTRED          (0xFF6347AA)
#define COLOR_DARKRED           (0xB33C27AA)
#define COLOR_BROWN             (0xACA179FF) 
#define COLOR_BLUE              (0x5999D1FF)
#define COLOR_LIGHTBLUE         (0x1FBFFFFF) 
#define COLOR_DARKYELLOW	    (0xEBD55DFF)
#define COLOR_YELLOW            (0xFFFF00AA)
#define COLOR_LIGHTYELLOW       (0xF5DEB3AA)  
#define COLOR_ORANGE            (0xFFA500FF) 
#define COLOR_LIGHTORANGE       (0xF0BD60FF)
#define COLOR_GREEN             (0x469146FF)
#define COLOR_BLEETER			(0x33AA74AA)
#define COLOR_LIGHTGREEN        (0x9EC73DFF)
#define COLOR_LIGHTGREEN2       (0xA0AD3EFF)
#define COLOR_PURPLE            (0xD0AEEBFF)
#define COLOR_LIGHTPURPLE       (0xDDAFAFAA)
#define COLOR_LIGHTCYAN1        (0xE2FFFFFF)
#define COLOR_LIGHTCYAN2        (0xCCE6E6FF)
#define COLOR_RADIO             (0xFFEC8BFF)
#define COLOR_RADIO2            (0xB4AF90FF)                      
#define COLOR_OOC               (0xCAE1FFFF)
#define COLOR_HELP              (0xFFEC8BFF)
#define COLOR_HELP_1            (0x5FB9D1FF)
#define COLOR_HELP_2            (0x58dd85FF)
#define COLOR_HELP_3            (0xf87070FF)
#define COLOR_HELP_4            (0xffcc68FF)
#define COL_HELP_2	            "{b2ffcd}"
#define COL_HELP_3	            "{fdbcbc}"
#define COL_HELP_4	            "{ffe1a8}"
#define COLOR_NEWS              (0x00DC76FF)
#define ANNOUNCEMENT_COLOR      (0x00D900FF)
#define TEAM_LSPD_COLOR         (0x8D8DFFFF)
#define TEAM_SASD_COLOR         (0x009966FF)
#define TEAM_LSFD_COLOR         (0xFF8282FF) 
#define POLICEDUTY              (0x8D8DFF00)
#define MEDDUTY                 (0xFF828200)
#define SADCRDUTY               (0x37673800)
#define TAXIDUTY                (0xFBA16C00)
#define ADMINDUTY               (0x1F854100)
#define TESTERDUTY              (0xA52A2A00)    
#define DEVDUTY                 (0x13131300)
#define REINFORCEMENT_COLOR     (0xFFFF00FF)
#define LABEL_COLOR             (0xFFFFFF99)
#define WOUNDED_LABEL_COLOR     (0xB62828FF)
#define INMATE_COLOR            (0x1B344B00)
#define MOST_WANTED_COLOR       (0xB6282800)
#define INVISIBLE               (0xFFFFFF00)