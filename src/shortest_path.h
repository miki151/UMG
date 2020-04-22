/* Copyright (C) 2013-2014 Michal Brzozowski (rusolis@poczta.fm)

   This file is part of KeeperRL.

   KeeperRL is free software; you can redistribute it and/or modify it under the terms of the
   GNU General Public License as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   KeeperRL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
   even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along with this program.
   If not, see http://www.gnu.org/licenses/ . */

#pragma once

#include <functional>

#include "util.h"

class Creature;
class Level;

class ShortestPath {
  public:
  ShortestPath(
      Rectangle area,
      function<double(Vec2)> entryFun,
      function<double(Vec2)> lengthFun,
      function<vector<Vec2>(Vec2)> directions,
      Vec2 target,
      Vec2 from);

  struct TemplateConstr {};
  template <typename EntryFun, typename LengthFun, typename DirectionsFun>
  ShortestPath(TemplateConstr, Rectangle area, EntryFun entryFun, LengthFun lengthFun, DirectionsFun directions,
      Vec2 target, Vec2 from);

  ShortestPath(
      Rectangle area,
      function<double(Vec2)> entryFun,
      function<double(Vec2)> lengthFun,
      vector<Vec2> directions,
      Vec2 target,
      Vec2 from);
  bool isReachable(Vec2 pos) const;
  Vec2 getNextMove(Vec2 pos);
  optional<Vec2> getNextNextMove(Vec2 pos);
  Vec2 getTarget() const;
  const vector<Vec2>& getPath() const;

  static const double infinity;

  private:
  template <typename EntryFun, typename LengthFun, typename DirectionsFun>
  void init(EntryFun entryFun, LengthFun lengthFun, DirectionsFun directions,
      Vec2 target, optional<Vec2> from, optional<int> limit = none);
  void constructPath(Vec2 start, function<vector<Vec2>(Vec2)> directions, bool reversed = false);
  vector<Vec2> SERIAL(path);
  Vec2 SERIAL(target);
  Rectangle SERIAL(bounds);
  bool SERIAL(reversed);
};

class Dijkstra {
  public:
  Dijkstra(Rectangle bounds, vector<Vec2> from, int maxDist, function<double(Vec2)> entryFun,
      vector<Vec2> directions = Vec2::directions8());
  bool isReachable(Vec2) const;
  double getDist(Vec2) const;
  const map<Vec2, double>& getAllReachable() const;

  private:
  map<Vec2, double> reachable;
};

class BfSearch {
  public:
  BfSearch(Rectangle bounds, Vec2 from, function<bool(Vec2)> entryFun, vector<Vec2> directions = Vec2::directions8());
  bool isReachable(Vec2) const;
  const set<Vec2>& getAllReachable() const;

  private:
  set<Vec2> reachable;
};

