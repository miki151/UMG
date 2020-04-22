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

#include "stdafx.h"
#include "util.h"
#include "shortest_path.h"

const double ShortestPath::infinity = 1000000000;

const int revShortestLimit = 15;

class DistanceTable {
  public:
  DistanceTable(Rectangle bounds) : ddist(bounds), dirty(bounds, 0) {}

  double getDistance(Vec2 v) const {
    return dirty[v] < counter ? ShortestPath::infinity : ddist[v];
  }

  void setDistance(Vec2 v, double d) {
    ddist[v] = d;
    dirty[v] = counter;
  }

  void clear() {
    ++counter;
  }

  private:
  Table<double> ddist;
  Table<int> dirty;
  int counter = 1;
};

const static Rectangle maxBounds = Rectangle(500, 500);

static DistanceTable distanceTable(maxBounds);
static DirtyTable<double> navigationCostCache(maxBounds, 0);

template <typename Fun>
static auto getCached(Fun fun) {
  return [fun] (Vec2 v) {
    if (navigationCostCache.isDirty(v))
      return navigationCostCache.getDirtyValue(v);
    else {
      auto res = fun(v);
      navigationCostCache.setValue(v, res);
      return res;
    }
  };
}

const int margin = 15;

ShortestPath::ShortestPath(Rectangle a, function<double(Vec2)> entryFun, function<double(Vec2)> lengthFun,
    function<vector<Vec2>(Vec2)> directions, Vec2 to, Vec2 from) : ShortestPath(TemplateConstr{},
    std::move(a), std::move(entryFun), std::move(lengthFun), std::move(directions), to, from) {}

template <typename EntryFun, typename LengthFun, typename DirectionsFun>
ShortestPath::ShortestPath(TemplateConstr, Rectangle a, EntryFun entryFun, LengthFun lengthFun,
    DirectionsFun directions, Vec2 to, Vec2 from) : target(to), bounds(a) {
  assert(maxBounds.contains(a));
  navigationCostCache.clear();
  init(getCached(entryFun), lengthFun, directions, target, from);
}

ShortestPath::ShortestPath(Rectangle area, function<double (Vec2)> entryFun, function<double(Vec2)> lengthFun,
    vector<Vec2> directions, Vec2 target, Vec2 from) : ShortestPath(area, entryFun, lengthFun,
    [directions](Vec2) { return directions; }, target, from)
{
}

struct QueueElem {
  Vec2 pos;
  double value;
};

bool inline operator < (const QueueElem& e1, const QueueElem& e2) {
  return e1.value > e2.value || (e1.value == e2.value && e1.pos < e2.pos);
}

template <typename EntryFun, typename LengthFun, typename DirectionsFun>
void ShortestPath::init(EntryFun entryFun, LengthFun lengthFun, DirectionsFun directions,
    Vec2 target, optional<Vec2> from, optional<int> limit) {
  reversed = false;
  distanceTable.clear();
  function<QueueElem(Vec2)> makeElem;
  if (from)
    makeElem = [&](Vec2 pos) ->QueueElem { return {pos, distanceTable.getDistance(pos) + lengthFun(pos)}; };
  else
    makeElem = [&](Vec2 pos) ->QueueElem { return {pos, distanceTable.getDistance(pos)}; };
  priority_queue<QueueElem, vector<QueueElem>> q;
  distanceTable.setDistance(target, 0);
  q.push(makeElem(target));
  int numPopped = 0;
  while (!q.empty()) {
    ++numPopped;
    Vec2 pos = q.top().pos;
    double posDist = distanceTable.getDistance(pos);
   // INFO << "Popping " << pos << " " << distance[pos]  << " " << (from ? (*from - pos).length4() : 0);
    if (from == pos || (limit && distanceTable.getDistance(pos) >= *limit)) {
      constructPath(pos, directions);
      return;
    }
    q.pop();
    for (Vec2 dir : directions(pos)) {
      Vec2 next = pos + dir;
      if (next.inRectangle(bounds)) {
        double nextDist = distanceTable.getDistance(next);
        if (posDist < nextDist) {
          double dist = posDist + entryFun(next);
          assert(dist > posDist);// << "Entry fun non positive " << dist - posDist;
          if (dist < nextDist) {
            distanceTable.setDistance(next, dist);
            {
              q.push(makeElem(next));
            }
          }
        }
      }
    }
  }
}

void ShortestPath::constructPath(Vec2 pos, function<vector<Vec2>(Vec2)> directions, bool reversed) {
  vector<Vec2> ret;
  //auto origPos = pos;
  while (pos != target) {
    Vec2 next;
    double lowest = distanceTable.getDistance(pos);
    assert(lowest < infinity);
    for (Vec2 dir : directions(pos)) {
      double dist = 0;
      if ((pos + dir).inRectangle(bounds) && (dist = distanceTable.getDistance(pos + dir)) < lowest) {
        lowest = dist;
        next = pos + dir;
      }
    }
    if (lowest >= distanceTable.getDistance(pos)) {
      if (reversed)
        break;
      else
        fail();
        //FATAL << "can't track path " << lowest << " " << distanceTable.getDistance(pos) << " " << origPos
        //    << " " << target << " " << pos << " " << next;
    }
    ret.push_back(pos);
    pos = next;
  }
  if (!reversed)
    ret.push_back(target);
  path = ret.reverse();
}

const vector<Vec2>& ShortestPath::getPath() const {
  return path;
}

bool ShortestPath::isReachable(Vec2 pos) const {
  return (path.size() >= 2 && path.back() == pos) || (path.size() >= 3 && path[path.size() - 2] == pos);
}

Vec2 ShortestPath::getNextMove(Vec2 pos) {
  assert(isReachable(pos));
  if (pos != path.back())
    path.pop_back();
  return path[path.size() - 2];
}

optional<Vec2> ShortestPath::getNextNextMove(Vec2 pos) {
  assert(isReachable(pos));
  if (pos != path.back())
    path.pop_back();
  if (path.size() > 2)
    return path[path.size() - 3];
  else
    return path[path.size() - 2];
}

Vec2 ShortestPath::getTarget() const {
  return target;
}

Dijkstra::Dijkstra(Rectangle bounds, vector<Vec2> from, int maxDist, function<double(Vec2)> entryFun,
      vector<Vec2> directions) {
  distanceTable.clear();
  auto comparator = [](Vec2 pos1, Vec2 pos2) {
      double diff = distanceTable.getDistance(pos1) - distanceTable.getDistance(pos2);
      if (diff > 0 || (diff == 0 && pos1 < pos2))
        return 1;
      else
        return 0;};
  priority_queue<Vec2, vector<Vec2>, decltype(comparator)> q(comparator) ;
  for (auto& v : from) {
    distanceTable.setDistance(v, 0);
    q.push(v);
  }
  int numPopped = 0;
  while (!q.empty()) {
    ++numPopped;
    Vec2 pos = q.top();
    double cdist = distanceTable.getDistance(pos);
    if (cdist > maxDist)
      return;
    q.pop();
    assert(!reachable.count(pos));
    reachable[pos] = cdist;
    for (Vec2 dir : directions) {
      Vec2 next = pos + dir;
      if (next.inRectangle(bounds)) {
        double ndist = distanceTable.getDistance(next);
        if (cdist < ndist) {
          double dist = cdist + entryFun(next);
          assert(dist > cdist);// << "Entry fun non positive " << dist - cdist;
          if (dist < ndist && dist <= maxDist) {
            distanceTable.setDistance(next, dist);
            q.push(next);
          }
        }
      }
    }
  }

}

bool Dijkstra::isReachable(Vec2 pos) const {
  return reachable.count(pos);
}

double Dijkstra::getDist(Vec2 v) const {
  return reachable.at(v);
}

const map<Vec2, double>& Dijkstra::getAllReachable() const {
  return reachable;
}

BfSearch::BfSearch(Rectangle bounds, Vec2 from, function<bool(Vec2)> entryFun, vector<Vec2> directions) {
  distanceTable.clear();
  queue<Vec2> q;
  distanceTable.setDistance(from, 0);
  q.push(from);
  int numPopped = 0;
  while (!q.empty()) {
    ++numPopped;
    Vec2 pos = q.front();
    q.pop();
    assert(!reachable.count(pos));
    reachable.insert(pos);
    for (Vec2 dir : directions) {
      Vec2 next = pos + dir;
      if (next.inRectangle(bounds) && distanceTable.getDistance(next) == ShortestPath::infinity && entryFun(next)) {
        distanceTable.setDistance(next, 0);
        q.push(next);
      }
    }
  }

}

bool BfSearch::isReachable(Vec2 pos) const {
  return reachable.count(pos);
}

const set<Vec2>& BfSearch::getAllReachable() const {
  return reachable;
}

