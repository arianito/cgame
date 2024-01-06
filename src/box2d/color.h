// SPDX-FileCopyrightText: 2022 Erin Catto
// SPDX-License-Identifier: MIT

#pragma once

#include "math/color.h"

/// All the colors! Credit to wherever I got this from, I forget.
typedef enum b2HexColor
{
	b2_colorAliceBlue = 0xf0f8ff,
	b2_colorAntiqueWhite = 0xfaebd7,
	b2_colorAntiqueWhite1 = 0xffefdb,
	b2_colorAntiqueWhite2 = 0xeedfcc,
	b2_colorAntiqueWhite3 = 0xcdc0b0,
	b2_colorAntiqueWhite4 = 0x8b8378,
	b2_colorAqua = 0x00ffff,
	b2_colorAquamarine = 0x7fffd4,
	b2_colorAquamarine1 = 0x7fffd4,
	b2_colorAquamarine2 = 0x76eec6,
	b2_colorAquamarine3 = 0x66cdaa,
	b2_colorAquamarine4 = 0x458b74,
	b2_colorAzure = 0xf0ffff,
	b2_colorAzure1 = 0xf0ffff,
	b2_colorAzure2 = 0xe0eeee,
	b2_colorAzure3 = 0xc1cdcd,
	b2_colorAzure4 = 0x838b8b,
	b2_colorBeige = 0xf5f5dc,
	b2_colorBisque = 0xffe4c4,
	b2_colorBisque1 = 0xffe4c4,
	b2_colorBisque2 = 0xeed5b7,
	b2_colorBisque3 = 0xcdb79e,
	b2_colorBisque4 = 0x8b7d6b,
	b2_colorBlack = 0x000000,
	b2_colorBlanchedAlmond = 0xffebcd,
	b2_colorBlue = 0x0000ff,
	b2_colorBlue1 = 0x0000ff,
	b2_colorBlue2 = 0x0000ee,
	b2_colorBlue3 = 0x0000cd,
	b2_colorBlue4 = 0x00008b,
	b2_colorBlueViolet = 0x8a2be2,
	b2_colorBrown = 0xa52a2a,
	b2_colorBrown1 = 0xff4040,
	b2_colorBrown2 = 0xee3b3b,
	b2_colorBrown3 = 0xcd3333,
	b2_colorBrown4 = 0x8b2323,
	b2_colorBurlywood = 0xdeb887,
	b2_colorBurlywood1 = 0xffd39b,
	b2_colorBurlywood2 = 0xeec591,
	b2_colorBurlywood3 = 0xcdaa7d,
	b2_colorBurlywood4 = 0x8b7355,
	b2_colorCadetBlue = 0x5f9ea0,
	b2_colorCadetBlue1 = 0x98f5ff,
	b2_colorCadetBlue2 = 0x8ee5ee,
	b2_colorCadetBlue3 = 0x7ac5cd,
	b2_colorCadetBlue4 = 0x53868b,
	b2_colorChartreuse = 0x7fff00,
	b2_colorChartreuse1 = 0x7fff00,
	b2_colorChartreuse2 = 0x76ee00,
	b2_colorChartreuse3 = 0x66cd00,
	b2_colorChartreuse4 = 0x458b00,
	b2_colorChocolate = 0xd2691e,
	b2_colorChocolate1 = 0xff7f24,
	b2_colorChocolate2 = 0xee7621,
	b2_colorChocolate3 = 0xcd661d,
	b2_colorChocolate4 = 0x8b4513,
	b2_colorCoral = 0xff7f50,
	b2_colorCoral1 = 0xff7256,
	b2_colorCoral2 = 0xee6a50,
	b2_colorCoral3 = 0xcd5b45,
	b2_colorCoral4 = 0x8b3e2f,
	b2_colorCornflowerBlue = 0x6495ed,
	b2_colorCornsilk = 0xfff8dc,
	b2_colorCornsilk1 = 0xfff8dc,
	b2_colorCornsilk2 = 0xeee8cd,
	b2_colorCornsilk3 = 0xcdc8b1,
	b2_colorCornsilk4 = 0x8b8878,
	b2_colorCrimson = 0xdc143c,
	b2_colorCyan = 0x00ffff,
	b2_colorCyan1 = 0x00ffff,
	b2_colorCyan2 = 0x00eeee,
	b2_colorCyan3 = 0x00cdcd,
	b2_colorCyan4 = 0x008b8b,
	b2_colorDarkBlue = 0x00008b,
	b2_colorDarkCyan = 0x008b8b,
	b2_colorDarkGoldenrod = 0xb8860b,
	b2_colorDarkGoldenrod1 = 0xffb90f,
	b2_colorDarkGoldenrod2 = 0xeead0e,
	b2_colorDarkGoldenrod3 = 0xcd950c,
	b2_colorDarkGoldenrod4 = 0x8b6508,
	b2_colorDarkGray = 0xa9a9a9,
	b2_colorDarkGreen = 0x006400,
	b2_colorDarkKhaki = 0xbdb76b,
	b2_colorDarkMagenta = 0x8b008b,
	b2_colorDarkOliveGreen = 0x556b2f,
	b2_colorDarkOliveGreen1 = 0xcaff70,
	b2_colorDarkOliveGreen2 = 0xbcee68,
	b2_colorDarkOliveGreen3 = 0xa2cd5a,
	b2_colorDarkOliveGreen4 = 0x6e8b3d,
	b2_colorDarkOrange = 0xff8c00,
	b2_colorDarkOrange1 = 0xff7f00,
	b2_colorDarkOrange2 = 0xee7600,
	b2_colorDarkOrange3 = 0xcd6600,
	b2_colorDarkOrange4 = 0x8b4500,
	b2_colorDarkOrchid = 0x9932cc,
	b2_colorDarkOrchid1 = 0xbf3eff,
	b2_colorDarkOrchid2 = 0xb23aee,
	b2_colorDarkOrchid3 = 0x9a32cd,
	b2_colorDarkOrchid4 = 0x68228b,
	b2_colorDarkRed = 0x8b0000,
	b2_colorDarkSalmon = 0xe9967a,
	b2_colorDarkSeaGreen = 0x8fbc8f,
	b2_colorDarkSeaGreen1 = 0xc1ffc1,
	b2_colorDarkSeaGreen2 = 0xb4eeb4,
	b2_colorDarkSeaGreen3 = 0x9bcd9b,
	b2_colorDarkSeaGreen4 = 0x698b69,
	b2_colorDarkSlateBlue = 0x483d8b,
	b2_colorDarkSlateGray = 0x2f4f4f,
	b2_colorDarkSlateGray1 = 0x97ffff,
	b2_colorDarkSlateGray2 = 0x8deeee,
	b2_colorDarkSlateGray3 = 0x79cdcd,
	b2_colorDarkSlateGray4 = 0x528b8b,
	b2_colorDarkTurquoise = 0x00ced1,
	b2_colorDarkViolet = 0x9400d3,
	b2_colorDeepPink = 0xff1493,
	b2_colorDeepPink1 = 0xff1493,
	b2_colorDeepPink2 = 0xee1289,
	b2_colorDeepPink3 = 0xcd1076,
	b2_colorDeepPink4 = 0x8b0a50,
	b2_colorDeepSkyBlue = 0x00bfff,
	b2_colorDeepSkyBlue1 = 0x00bfff,
	b2_colorDeepSkyBlue2 = 0x00b2ee,
	b2_colorDeepSkyBlue3 = 0x009acd,
	b2_colorDeepSkyBlue4 = 0x00688b,
	b2_colorDimGray = 0x696969,
	b2_colorDodgerBlue = 0x1e90ff,
	b2_colorDodgerBlue1 = 0x1e90ff,
	b2_colorDodgerBlue2 = 0x1c86ee,
	b2_colorDodgerBlue3 = 0x1874cd,
	b2_colorDodgerBlue4 = 0x104e8b,
	b2_colorFirebrick = 0xb22222,
	b2_colorFirebrick1 = 0xff3030,
	b2_colorFirebrick2 = 0xee2c2c,
	b2_colorFirebrick3 = 0xcd2626,
	b2_colorFirebrick4 = 0x8b1a1a,
	b2_colorFloralWhite = 0xfffaf0,
	b2_colorForestGreen = 0x228b22,
	b2_colorFuchsia = 0xff00ff,
	b2_colorGainsboro = 0xdcdcdc,
	b2_colorGhostWhite = 0xf8f8ff,
	b2_colorGold = 0xffd700,
	b2_colorGold1 = 0xffd700,
	b2_colorGold2 = 0xeec900,
	b2_colorGold3 = 0xcdad00,
	b2_colorGold4 = 0x8b7500,
	b2_colorGoldenrod = 0xdaa520,
	b2_colorGoldenrod1 = 0xffc125,
	b2_colorGoldenrod2 = 0xeeb422,
	b2_colorGoldenrod3 = 0xcd9b1d,
	b2_colorGoldenrod4 = 0x8b6914,
	b2_colorGray = 0xbebebe,
	b2_colorGray0 = 0x000000,
	b2_colorGray1 = 0x030303,
	b2_colorGray10 = 0x1a1a1a,
	b2_colorGray100 = 0xffffff,
	b2_colorGray11 = 0x1c1c1c,
	b2_colorGray12 = 0x1f1f1f,
	b2_colorGray13 = 0x212121,
	b2_colorGray14 = 0x242424,
	b2_colorGray15 = 0x262626,
	b2_colorGray16 = 0x292929,
	b2_colorGray17 = 0x2b2b2b,
	b2_colorGray18 = 0x2e2e2e,
	b2_colorGray19 = 0x303030,
	b2_colorGray2 = 0x050505,
	b2_colorGray20 = 0x333333,
	b2_colorGray21 = 0x363636,
	b2_colorGray22 = 0x383838,
	b2_colorGray23 = 0x3b3b3b,
	b2_colorGray24 = 0x3d3d3d,
	b2_colorGray25 = 0x404040,
	b2_colorGray26 = 0x424242,
	b2_colorGray27 = 0x454545,
	b2_colorGray28 = 0x474747,
	b2_colorGray29 = 0x4a4a4a,
	b2_colorGray3 = 0x080808,
	b2_colorGray30 = 0x4d4d4d,
	b2_colorGray31 = 0x4f4f4f,
	b2_colorGray32 = 0x525252,
	b2_colorGray33 = 0x545454,
	b2_colorGray34 = 0x575757,
	b2_colorGray35 = 0x595959,
	b2_colorGray36 = 0x5c5c5c,
	b2_colorGray37 = 0x5e5e5e,
	b2_colorGray38 = 0x616161,
	b2_colorGray39 = 0x636363,
	b2_colorGray4 = 0x0a0a0a,
	b2_colorGray40 = 0x666666,
	b2_colorGray41 = 0x696969,
	b2_colorGray42 = 0x6b6b6b,
	b2_colorGray43 = 0x6e6e6e,
	b2_colorGray44 = 0x707070,
	b2_colorGray45 = 0x737373,
	b2_colorGray46 = 0x757575,
	b2_colorGray47 = 0x787878,
	b2_colorGray48 = 0x7a7a7a,
	b2_colorGray49 = 0x7d7d7d,
	b2_colorGray5 = 0x0d0d0d,
	b2_colorGray50 = 0x7f7f7f,
	b2_colorGray51 = 0x828282,
	b2_colorGray52 = 0x858585,
	b2_colorGray53 = 0x878787,
	b2_colorGray54 = 0x8a8a8a,
	b2_colorGray55 = 0x8c8c8c,
	b2_colorGray56 = 0x8f8f8f,
	b2_colorGray57 = 0x919191,
	b2_colorGray58 = 0x949494,
	b2_colorGray59 = 0x969696,
	b2_colorGray6 = 0x0f0f0f,
	b2_colorGray60 = 0x999999,
	b2_colorGray61 = 0x9c9c9c,
	b2_colorGray62 = 0x9e9e9e,
	b2_colorGray63 = 0xa1a1a1,
	b2_colorGray64 = 0xa3a3a3,
	b2_colorGray65 = 0xa6a6a6,
	b2_colorGray66 = 0xa8a8a8,
	b2_colorGray67 = 0xababab,
	b2_colorGray68 = 0xadadad,
	b2_colorGray69 = 0xb0b0b0,
	b2_colorGray7 = 0x121212,
	b2_colorGray70 = 0xb3b3b3,
	b2_colorGray71 = 0xb5b5b5,
	b2_colorGray72 = 0xb8b8b8,
	b2_colorGray73 = 0xbababa,
	b2_colorGray74 = 0xbdbdbd,
	b2_colorGray75 = 0xbfbfbf,
	b2_colorGray76 = 0xc2c2c2,
	b2_colorGray77 = 0xc4c4c4,
	b2_colorGray78 = 0xc7c7c7,
	b2_colorGray79 = 0xc9c9c9,
	b2_colorGray8 = 0x141414,
	b2_colorGray80 = 0xcccccc,
	b2_colorGray81 = 0xcfcfcf,
	b2_colorGray82 = 0xd1d1d1,
	b2_colorGray83 = 0xd4d4d4,
	b2_colorGray84 = 0xd6d6d6,
	b2_colorGray85 = 0xd9d9d9,
	b2_colorGray86 = 0xdbdbdb,
	b2_colorGray87 = 0xdedede,
	b2_colorGray88 = 0xe0e0e0,
	b2_colorGray89 = 0xe3e3e3,
	b2_colorGray9 = 0x171717,
	b2_colorGray90 = 0xe5e5e5,
	b2_colorGray91 = 0xe8e8e8,
	b2_colorGray92 = 0xebebeb,
	b2_colorGray93 = 0xededed,
	b2_colorGray94 = 0xf0f0f0,
	b2_colorGray95 = 0xf2f2f2,
	b2_colorGray96 = 0xf5f5f5,
	b2_colorGray97 = 0xf7f7f7,
	b2_colorGray98 = 0xfafafa,
	b2_colorGray99 = 0xfcfcfc,
	b2_colorGreen = 0x00ff00,
	b2_colorGreen1 = 0x00ff00,
	b2_colorGreen2 = 0x00ee00,
	b2_colorGreen3 = 0x00cd00,
	b2_colorGreen4 = 0x008b00,
	b2_colorGreenYellow = 0xadff2f,
	b2_colorHoneydew = 0xf0fff0,
	b2_colorHoneydew1 = 0xf0fff0,
	b2_colorHoneydew2 = 0xe0eee0,
	b2_colorHoneydew3 = 0xc1cdc1,
	b2_colorHoneydew4 = 0x838b83,
	b2_colorHotPink = 0xff69b4,
	b2_colorHotPink1 = 0xff6eb4,
	b2_colorHotPink2 = 0xee6aa7,
	b2_colorHotPink3 = 0xcd6090,
	b2_colorHotPink4 = 0x8b3a62,
	b2_colorIndianRed = 0xcd5c5c,
	b2_colorIndianRed1 = 0xff6a6a,
	b2_colorIndianRed2 = 0xee6363,
	b2_colorIndianRed3 = 0xcd5555,
	b2_colorIndianRed4 = 0x8b3a3a,
	b2_colorIndigo = 0x4b0082,
	b2_colorIvory = 0xfffff0,
	b2_colorIvory1 = 0xfffff0,
	b2_colorIvory2 = 0xeeeee0,
	b2_colorIvory3 = 0xcdcdc1,
	b2_colorIvory4 = 0x8b8b83,
	b2_colorKhaki = 0xf0e68c,
	b2_colorKhaki1 = 0xfff68f,
	b2_colorKhaki2 = 0xeee685,
	b2_colorKhaki3 = 0xcdc673,
	b2_colorKhaki4 = 0x8b864e,
	b2_colorLavender = 0xe6e6fa,
	b2_colorLavenderBlush = 0xfff0f5,
	b2_colorLavenderBlush1 = 0xfff0f5,
	b2_colorLavenderBlush2 = 0xeee0e5,
	b2_colorLavenderBlush3 = 0xcdc1c5,
	b2_colorLavenderBlush4 = 0x8b8386,
	b2_colorLawnGreen = 0x7cfc00,
	b2_colorLemonChiffon = 0xfffacd,
	b2_colorLemonChiffon1 = 0xfffacd,
	b2_colorLemonChiffon2 = 0xeee9bf,
	b2_colorLemonChiffon3 = 0xcdc9a5,
	b2_colorLemonChiffon4 = 0x8b8970,
	b2_colorLightBlue = 0xadd8e6,
	b2_colorLightBlue1 = 0xbfefff,
	b2_colorLightBlue2 = 0xb2dfee,
	b2_colorLightBlue3 = 0x9ac0cd,
	b2_colorLightBlue4 = 0x68838b,
	b2_colorLightCoral = 0xf08080,
	b2_colorLightCyan = 0xe0ffff,
	b2_colorLightCyan1 = 0xe0ffff,
	b2_colorLightCyan2 = 0xd1eeee,
	b2_colorLightCyan3 = 0xb4cdcd,
	b2_colorLightCyan4 = 0x7a8b8b,
	b2_colorLightGoldenrod = 0xeedd82,
	b2_colorLightGoldenrod1 = 0xffec8b,
	b2_colorLightGoldenrod2 = 0xeedc82,
	b2_colorLightGoldenrod3 = 0xcdbe70,
	b2_colorLightGoldenrod4 = 0x8b814c,
	b2_colorLightGoldenrodYellow = 0xfafad2,
	b2_colorLightGray = 0xd3d3d3,
	b2_colorLightGreen = 0x90ee90,
	b2_colorLightPink = 0xffb6c1,
	b2_colorLightPink1 = 0xffaeb9,
	b2_colorLightPink2 = 0xeea2ad,
	b2_colorLightPink3 = 0xcd8c95,
	b2_colorLightPink4 = 0x8b5f65,
	b2_colorLightSalmon = 0xffa07a,
	b2_colorLightSalmon1 = 0xffa07a,
	b2_colorLightSalmon2 = 0xee9572,
	b2_colorLightSalmon3 = 0xcd8162,
	b2_colorLightSalmon4 = 0x8b5742,
	b2_colorLightSeaGreen = 0x20b2aa,
	b2_colorLightSkyBlue = 0x87cefa,
	b2_colorLightSkyBlue1 = 0xb0e2ff,
	b2_colorLightSkyBlue2 = 0xa4d3ee,
	b2_colorLightSkyBlue3 = 0x8db6cd,
	b2_colorLightSkyBlue4 = 0x607b8b,
	b2_colorLightSlateBlue = 0x8470ff,
	b2_colorLightSlateGray = 0x778899,
	b2_colorLightSteelBlue = 0xb0c4de,
	b2_colorLightSteelBlue1 = 0xcae1ff,
	b2_colorLightSteelBlue2 = 0xbcd2ee,
	b2_colorLightSteelBlue3 = 0xa2b5cd,
	b2_colorLightSteelBlue4 = 0x6e7b8b,
	b2_colorLightYellow = 0xffffe0,
	b2_colorLightYellow1 = 0xffffe0,
	b2_colorLightYellow2 = 0xeeeed1,
	b2_colorLightYellow3 = 0xcdcdb4,
	b2_colorLightYellow4 = 0x8b8b7a,
	b2_colorLime = 0x00ff00,
	b2_colorLimeGreen = 0x32cd32,
	b2_colorLinen = 0xfaf0e6,
	b2_colorMagenta = 0xff00ff,
	b2_colorMagenta1 = 0xff00ff,
	b2_colorMagenta2 = 0xee00ee,
	b2_colorMagenta3 = 0xcd00cd,
	b2_colorMagenta4 = 0x8b008b,
	b2_colorMaroon = 0xb03060,
	b2_colorMaroon1 = 0xff34b3,
	b2_colorMaroon2 = 0xee30a7,
	b2_colorMaroon3 = 0xcd2990,
	b2_colorMaroon4 = 0x8b1c62,
	b2_colorMediumAquamarine = 0x66cdaa,
	b2_colorMediumBlue = 0x0000cd,
	b2_colorMediumOrchid = 0xba55d3,
	b2_colorMediumOrchid1 = 0xe066ff,
	b2_colorMediumOrchid2 = 0xd15fee,
	b2_colorMediumOrchid3 = 0xb452cd,
	b2_colorMediumOrchid4 = 0x7a378b,
	b2_colorMediumPurple = 0x9370db,
	b2_colorMediumPurple1 = 0xab82ff,
	b2_colorMediumPurple2 = 0x9f79ee,
	b2_colorMediumPurple3 = 0x8968cd,
	b2_colorMediumPurple4 = 0x5d478b,
	b2_colorMediumSeaGreen = 0x3cb371,
	b2_colorMediumSlateBlue = 0x7b68ee,
	b2_colorMediumSpringGreen = 0x00fa9a,
	b2_colorMediumTurquoise = 0x48d1cc,
	b2_colorMediumVioletRed = 0xc71585,
	b2_colorMidnightBlue = 0x191970,
	b2_colorMintCream = 0xf5fffa,
	b2_colorMistyRose = 0xffe4e1,
	b2_colorMistyRose1 = 0xffe4e1,
	b2_colorMistyRose2 = 0xeed5d2,
	b2_colorMistyRose3 = 0xcdb7b5,
	b2_colorMistyRose4 = 0x8b7d7b,
	b2_colorMoccasin = 0xffe4b5,
	b2_colorNavajoWhite = 0xffdead,
	b2_colorNavajoWhite1 = 0xffdead,
	b2_colorNavajoWhite2 = 0xeecfa1,
	b2_colorNavajoWhite3 = 0xcdb38b,
	b2_colorNavajoWhite4 = 0x8b795e,
	b2_colorNavy = 0x000080,
	b2_colorNavyBlue = 0x000080,
	b2_colorOldLace = 0xfdf5e6,
	b2_colorOlive = 0x808000,
	b2_colorOliveDrab = 0x6b8e23,
	b2_colorOliveDrab1 = 0xc0ff3e,
	b2_colorOliveDrab2 = 0xb3ee3a,
	b2_colorOliveDrab3 = 0x9acd32,
	b2_colorOliveDrab4 = 0x698b22,
	b2_colorOrange = 0xffa500,
	b2_colorOrange1 = 0xffa500,
	b2_colorOrange2 = 0xee9a00,
	b2_colorOrange3 = 0xcd8500,
	b2_colorOrange4 = 0x8b5a00,
	b2_colorOrangeRed = 0xff4500,
	b2_colorOrangeRed1 = 0xff4500,
	b2_colorOrangeRed2 = 0xee4000,
	b2_colorOrangeRed3 = 0xcd3700,
	b2_colorOrangeRed4 = 0x8b2500,
	b2_colorOrchid = 0xda70d6,
	b2_colorOrchid1 = 0xff83fa,
	b2_colorOrchid2 = 0xee7ae9,
	b2_colorOrchid3 = 0xcd69c9,
	b2_colorOrchid4 = 0x8b4789,
	b2_colorPaleGoldenrod = 0xeee8aa,
	b2_colorPaleGreen = 0x98fb98,
	b2_colorPaleGreen1 = 0x9aff9a,
	b2_colorPaleGreen2 = 0x90ee90,
	b2_colorPaleGreen3 = 0x7ccd7c,
	b2_colorPaleGreen4 = 0x548b54,
	b2_colorPaleTurquoise = 0xafeeee,
	b2_colorPaleTurquoise1 = 0xbbffff,
	b2_colorPaleTurquoise2 = 0xaeeeee,
	b2_colorPaleTurquoise3 = 0x96cdcd,
	b2_colorPaleTurquoise4 = 0x668b8b,
	b2_colorPaleVioletRed = 0xdb7093,
	b2_colorPaleVioletRed1 = 0xff82ab,
	b2_colorPaleVioletRed2 = 0xee799f,
	b2_colorPaleVioletRed3 = 0xcd6889,
	b2_colorPaleVioletRed4 = 0x8b475d,
	b2_colorPapayaWhip = 0xffefd5,
	b2_colorPeachPuff = 0xffdab9,
	b2_colorPeachPuff1 = 0xffdab9,
	b2_colorPeachPuff2 = 0xeecbad,
	b2_colorPeachPuff3 = 0xcdaf95,
	b2_colorPeachPuff4 = 0x8b7765,
	b2_colorPeru = 0xcd853f,
	b2_colorPink = 0xffc0cb,
	b2_colorPink1 = 0xffb5c5,
	b2_colorPink2 = 0xeea9b8,
	b2_colorPink3 = 0xcd919e,
	b2_colorPink4 = 0x8b636c,
	b2_colorPlum = 0xdda0dd,
	b2_colorPlum1 = 0xffbbff,
	b2_colorPlum2 = 0xeeaeee,
	b2_colorPlum3 = 0xcd96cd,
	b2_colorPlum4 = 0x8b668b,
	b2_colorPowderBlue = 0xb0e0e6,
	b2_colorPurple = 0xa020f0,
	b2_colorPurple1 = 0x9b30ff,
	b2_colorPurple2 = 0x912cee,
	b2_colorPurple3 = 0x7d26cd,
	b2_colorPurple4 = 0x551a8b,
	b2_colorRebeccaPurple = 0x663399,
	b2_colorRed = 0xff0000,
	b2_colorRed1 = 0xff0000,
	b2_colorRed2 = 0xee0000,
	b2_colorRed3 = 0xcd0000,
	b2_colorRed4 = 0x8b0000,
	b2_colorRosyBrown = 0xbc8f8f,
	b2_colorRosyBrown1 = 0xffc1c1,
	b2_colorRosyBrown2 = 0xeeb4b4,
	b2_colorRosyBrown3 = 0xcd9b9b,
	b2_colorRosyBrown4 = 0x8b6969,
	b2_colorRoyalBlue = 0x4169e1,
	b2_colorRoyalBlue1 = 0x4876ff,
	b2_colorRoyalBlue2 = 0x436eee,
	b2_colorRoyalBlue3 = 0x3a5fcd,
	b2_colorRoyalBlue4 = 0x27408b,
	b2_colorSaddleBrown = 0x8b4513,
	b2_colorSalmon = 0xfa8072,
	b2_colorSalmon1 = 0xff8c69,
	b2_colorSalmon2 = 0xee8262,
	b2_colorSalmon3 = 0xcd7054,
	b2_colorSalmon4 = 0x8b4c39,
	b2_colorSandyBrown = 0xf4a460,
	b2_colorSeaGreen = 0x2e8b57,
	b2_colorSeaGreen1 = 0x54ff9f,
	b2_colorSeaGreen2 = 0x4eee94,
	b2_colorSeaGreen3 = 0x43cd80,
	b2_colorSeaGreen4 = 0x2e8b57,
	b2_colorSeashell = 0xfff5ee,
	b2_colorSeashell1 = 0xfff5ee,
	b2_colorSeashell2 = 0xeee5de,
	b2_colorSeashell3 = 0xcdc5bf,
	b2_colorSeashell4 = 0x8b8682,
	b2_colorSienna = 0xa0522d,
	b2_colorSienna1 = 0xff8247,
	b2_colorSienna2 = 0xee7942,
	b2_colorSienna3 = 0xcd6839,
	b2_colorSienna4 = 0x8b4726,
	b2_colorSilver = 0xc0c0c0,
	b2_colorSkyBlue = 0x87ceeb,
	b2_colorSkyBlue1 = 0x87ceff,
	b2_colorSkyBlue2 = 0x7ec0ee,
	b2_colorSkyBlue3 = 0x6ca6cd,
	b2_colorSkyBlue4 = 0x4a708b,
	b2_colorSlateBlue = 0x6a5acd,
	b2_colorSlateBlue1 = 0x836fff,
	b2_colorSlateBlue2 = 0x7a67ee,
	b2_colorSlateBlue3 = 0x6959cd,
	b2_colorSlateBlue4 = 0x473c8b,
	b2_colorSlateGray = 0x708090,
	b2_colorSlateGray1 = 0xc6e2ff,
	b2_colorSlateGray2 = 0xb9d3ee,
	b2_colorSlateGray3 = 0x9fb6cd,
	b2_colorSlateGray4 = 0x6c7b8b,
	b2_colorSnow = 0xfffafa,
	b2_colorSnow1 = 0xfffafa,
	b2_colorSnow2 = 0xeee9e9,
	b2_colorSnow3 = 0xcdc9c9,
	b2_colorSnow4 = 0x8b8989,
	b2_colorSpringGreen = 0x00ff7f,
	b2_colorSpringGreen1 = 0x00ff7f,
	b2_colorSpringGreen2 = 0x00ee76,
	b2_colorSpringGreen3 = 0x00cd66,
	b2_colorSpringGreen4 = 0x008b45,
	b2_colorSteelBlue = 0x4682b4,
	b2_colorSteelBlue1 = 0x63b8ff,
	b2_colorSteelBlue2 = 0x5cacee,
	b2_colorSteelBlue3 = 0x4f94cd,
	b2_colorSteelBlue4 = 0x36648b,
	b2_colorTan = 0xd2b48c,
	b2_colorTan1 = 0xffa54f,
	b2_colorTan2 = 0xee9a49,
	b2_colorTan3 = 0xcd853f,
	b2_colorTan4 = 0x8b5a2b,
	b2_colorTeal = 0x008080,
	b2_colorThistle = 0xd8bfd8,
	b2_colorThistle1 = 0xffe1ff,
	b2_colorThistle2 = 0xeed2ee,
	b2_colorThistle3 = 0xcdb5cd,
	b2_colorThistle4 = 0x8b7b8b,
	b2_colorTomato = 0xff6347,
	b2_colorTomato1 = 0xff6347,
	b2_colorTomato2 = 0xee5c42,
	b2_colorTomato3 = 0xcd4f39,
	b2_colorTomato4 = 0x8b3626,
	b2_colorTurquoise = 0x40e0d0,
	b2_colorTurquoise1 = 0x00f5ff,
	b2_colorTurquoise2 = 0x00e5ee,
	b2_colorTurquoise3 = 0x00c5cd,
	b2_colorTurquoise4 = 0x00868b,
	b2_colorViolet = 0xee82ee,
	b2_colorVioletRed = 0xd02090,
	b2_colorVioletRed1 = 0xff3e96,
	b2_colorVioletRed2 = 0xee3a8c,
	b2_colorVioletRed3 = 0xcd3278,
	b2_colorVioletRed4 = 0x8b2252,
	b2_colorWebGray = 0x808080,
	b2_colorWebGreen = 0x008000,
	b2_colorWebMaroon = 0x800000,
	b2_colorWebPurple = 0x800080,
	b2_colorWheat = 0xf5deb3,
	b2_colorWheat1 = 0xffe7ba,
	b2_colorWheat2 = 0xeed8ae,
	b2_colorWheat3 = 0xcdba96,
	b2_colorWheat4 = 0x8b7e66,
	b2_colorWhite = 0xffffff,
	b2_colorWhiteSmoke = 0xf5f5f5,
	b2_colorX11Gray = 0xbebebe,
	b2_colorX11Green = 0x00ff00,
	b2_colorX11Maroon = 0xb03060,
	b2_colorX11Purple = 0xa020f0,
	b2_colorYellow = 0xffff00,
	b2_colorYellow1 = 0xffff00,
	b2_colorYellow2 = 0xeeee00,
	b2_colorYellow3 = 0xcdcd00,
	b2_colorYellow4 = 0x8b8b00,
	b2_colorYellowGreen = 0x9acd32,
} b2HexColor;

/// Make a color from a hex code
static inline Color b2MakeColor(enum b2HexColor hexCode, float alpha)
{
	Color color;
	color.p0 = ((hexCode >> 16) & 0xFF) / 255.0f;
	color.p1 = ((hexCode >> 8) & 0xFF) / 255.0f;
	color.p2 = (hexCode & 0xFF) / 255.0f;
	color.alpha = alpha;
	return color;
}
