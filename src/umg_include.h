#pragma once


constexpr auto umgInclude =
"    Def Margin(Size, Gen1, Gen2)\n"
"      Margins(Size, Gen1, Gen2)\n"
"    End\n"
"    \n"
"    Def Margin(Position, Size, Gen1, Gen2)\n"
"      MarginImpl(Position, Size, Gen1, Gen2)\n"
"    End\n"
"    \n"
"    Def Inside(Size, Gen)\n"
"      Margin(Size, {}, Gen)\n"
"    End\n"
"    \n"
"    Def Inside(Pos, Size, Gen)\n"
"      Margin(Pos, Size, {}, Gen)\n"
"    End\n"
"    \n"
"    Def Border(Size, Gen)\n"
"      Margin(Size, Gen, {})\n"
"    End\n"
"    \n"
"    Def Border(Pos, Size, Gen)\n"
"      Margin(Pos, Size, Gen, {})\n"
"    End\n"
;
