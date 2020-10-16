#pragma once

#include "stdafx.h"
#include "token.h"
#include "util.h"

using Token = string;

struct LayoutCanvas {
  struct Map {
    Table<vector<Token>> elems;
  };
  LayoutCanvas with(Rectangle area) const {
    //if (map->elems.getBounds().contains(area));
    return LayoutCanvas{area, map};
  }
  Rectangle area;
  Map* map;
};
