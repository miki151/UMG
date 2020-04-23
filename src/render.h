#pragma once

#include "stdafx.h"

struct Map;
void renderAscii(const Map&, istream& file);
string renderHtml(const Map&, const char* renderer);
