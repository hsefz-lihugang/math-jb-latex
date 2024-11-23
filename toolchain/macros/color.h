#pragma once

#ifndef _INC_COLOR
#define _INC_COLOR

#define FCOLOR_BLACK "30"
#define FCOLOR_RED "31"
#define FCOLOR_GREEN "32"
#define FCOLOR_YELLOW "33"
#define FCOLOR_BLUE "34"
#define FCOLOR_MAGENTA "35"
#define FCOLOR_CYAN "36"
#define FCOLOR_WHITE "37"

#define BCOLOR_BLACK "40"
#define BCOLOR_RED "41"
#define BCOLOR_GREEN "42"
#define BCOLOR_YELLOW "43"
#define BCOLOR_BLUE "44"
#define BCOLOR_MAGENTA "45"
#define BCOLOR_CYAN "46"
#define BCOLOR_WHITE "47"

#ifndef NO_COLOR
#define USE_COLOR(frontColor) ("\033[0;" frontColor "m")
#define USE_COLOR_WITH_BKG(frontColor, backgroundColor) ("\033[0;" frontColor ";" backgroundColor "m")
#else
#define USE_COLOR_WITH_BKG(frontColor, backgroundColor) ("")
#define USE_COLOR(frontColor) ("")
#endif

#endif