#pragma once

#include "stdafx.h"
#include "token.h"
#include "util.h"

struct Map {
  Table<unordered_set<Token>> elems;
};

struct Canvas {
  Canvas with(Rectangle area) const { return Canvas{area, map}; }
  Rectangle area;
  Map* map;
};
