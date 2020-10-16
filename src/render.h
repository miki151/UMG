#pragma once

#include "stdafx.h"
#include "canvas.h"

void renderAscii(const LayoutCanvas::Map&, istream& file);
string renderHtml(const LayoutCanvas::Map&, const char* renderer);
