#include "generator.h"
#include "canvas.h"
#include "shortest_path.h"

bool make(const Generators::None&, Canvas c, RandomGen&) {
  return true;
}

bool make(const Generators::Set& g, Canvas c, RandomGen&) {
  for (auto v : c.area)
    for (auto& token : g.tokens)
      c.map->elems[v].insert(token);
  return true;
}

bool make(const Generators::Reset& g, Canvas c, RandomGen&) {
  for (auto v : c.area) {
    c.map->elems[v].clear();
    for (auto& token : g.tokens)
      c.map->elems[v].insert(token);
  }
  return true;
}

bool make(const Generators::SetMaybe& g, Canvas c, RandomGen& r) {
  for (auto v : c.area)
    if (g.predicate.apply(c.map, v, r))
      c.map->elems[v].insert(g.token);
  return true;
}

bool make(const Generators::Remove& g, Canvas c, RandomGen&) {
  for (auto v : c.area)
    c.map->elems[v].erase(g.token);
  return true;
}

bool make(const Generators::Margins& g, Canvas c, RandomGen& r) {
  if (!g.inside->make(c.with(c.area.minusMargin(g.width)), r))
    return false;
  if (!g.border->make(c.with(Rectangle(c.area.topLeft(), Vec2(c.area.right(), c.area.top() + g.width))), r))
    return false;
  if (!g.border->make(c.with(Rectangle(
      Vec2(c.area.right() - g.width, c.area.top() + g.width),
      c.area.bottomRight())), r))
    return false;
  if (!g.border->make(c.with(Rectangle(
      Vec2(c.area.left(), c.area.bottom() - g.width),
      Vec2(c.area.right() - g.width, c.area.bottom()))), r))
    return false;
  return g.border->make(c.with(Rectangle(
      Vec2(c.area.left(), c.area.top() + g.width),
      Vec2(c.area.left() + g.width, c.area.bottom() - g.width))), r);
}

bool make(const Generators::Margin& g, Canvas c, RandomGen& r) {
  auto rect = [&]() -> pair<Rectangle, Rectangle> {
    switch (g.type) {
      case MarginType::TOP: return {
            Rectangle(c.area.topLeft(), Vec2(c.area.right(), c.area.top() + g.width)),
            Rectangle(Vec2(c.area.left(), c.area.top() + g.width), c.area.bottomRight())
          };
      case MarginType::BOTTOM: return {
            Rectangle(Vec2(c.area.left(), c.area.bottom() - g.width), c.area.bottomRight()),
            Rectangle(c.area.topLeft(), Vec2(c.area.right(), c.area.bottom() - g.width))
          };
      case MarginType::LEFT: return {
            Rectangle(c.area.topLeft(), Vec2(c.area.left() + g.width, c.area.bottom())),
            Rectangle(Vec2(c.area.left() + g.width, c.area.top()), c.area.bottomRight())
          };
      case MarginType::RIGHT: return {
            Rectangle(Vec2(c.area.right() - g.width, c.area.top()), c.area.bottomRight()),
            Rectangle(c.area.topLeft(), Vec2(c.area.right() - g.width, c.area.bottom()))
          };
    }
    fail();
  }();
  if (!g.border->make(c.with(rect.first), r))
    return false;
  return g.inside->make(c.with(rect.second), r);
}

bool make(const Generators::HRatio& g, Canvas c, RandomGen& r) {
  if (!g.left->make(c.with(Rectangle(
      c.area.topLeft(),
      Vec2(int(c.area.left() + c.area.width() * g.r), c.area.bottom()))), r))
    return false;
  return g.right->make(c.with(Rectangle(
      Vec2(int(c.area.left() + c.area.width() * g.r), c.area.top()),
      c.area.bottomRight())), r);
}

bool make(const Generators::VRatio& g, Canvas c, RandomGen& r) {
  if (!g.top->make(c.with(Rectangle(
      c.area.topLeft(),
      Vec2(c.area.right(), int(c.area.top() + c.area.height() * g.r)))), r))
    return false;
  return g.bottom->make(c.with(Rectangle(
      Vec2(c.area.left(), int(c.area.top() + c.area.height() * g.r)),
      c.area.bottomRight())), r);
}

bool make(const Generators::Place& g, Canvas c, RandomGen& r) {
  vector<char> occupied(c.area.width() * c.area.height(), 0);
  auto check = [&] (Rectangle rect, const Predicate& p) {
    for (auto v : rect)
      if (!p.apply(c.map, v, r) || occupied[(v.x - c.area.left()) + (v.y - c.area.top()) * c.area.width()] != 0)
        return false;
    for (auto v : rect)
      occupied[(v.x - c.area.left()) + (v.y - c.area.top()) * c.area.width()] = 1;
    return true;
  };
  for (int i : All(g.generators)) {
    auto size = g.generators[i].size;
    auto& generator = g.generators[i].generator;
    auto generate = [&] {
      const int numTries = g.generators[i].position ? 1 : 100000;
      for (int iter : Range(numTries)) {
        auto pos = g.generators[i].position
            ? c.area.middle() - size / 2
            : Rectangle(c.area.topLeft(), c.area.bottomRight() - size).random(r);
        auto genArea = Rectangle(pos, pos + size);
        if (!check(genArea, g.generators[i].predicate))
          continue;
        return generator->make(c.with(genArea), r);
      }
      return false;
    };
    for (int j : Range(g.generators[i].count))
      if (!generate())
        return false;
  }
  return true;
}

struct NoiseInit {
  int topLeft;
  int topRight;
  int bottomRight;
  int bottomLeft;
  int middle;
};


void addAvg(int x, int y, const Table<double>& wys, double& avg, int& num) {
  Vec2 pos(x, y);
  if (pos.inRectangle(wys.getBounds())) {
    avg += wys[pos];
    ++num;
  }
}

Table<double> genNoiseMap(RandomGen& random, Rectangle area, NoiseInit init, double varianceMult) {
  int width = 1;
  while (width < area.width() - 1 || width < area.height() - 1)
    width *= 2;
  width /= 2;
  ++width;
  Table<double> wys(width, width);
  wys[0][0] = init.topLeft;
  wys[width - 1][0] = init.topRight;
  wys[width - 1][width - 1] = init.bottomRight;
  wys[0][width - 1] = init.bottomLeft;
  wys[(width - 1) / 2][(width - 1) / 2] = init.middle;

  double variance = 0.5;
  for (int a = width - 1; a >= 2; a /= 2) {
    if (a < width - 1)
      for (Vec2 pos1 : Rectangle((width - 1) / a, (width - 1) / a)) {
        Vec2 pos = pos1 * a;
        double avg = (wys[pos] + wys[pos.x + a][pos.y] + wys[pos.x][pos.y + a] + wys[pos.x + a][pos.y + a]) / 4;
        wys[pos.x + a / 2][pos.y + a / 2] =
            avg + variance * (random.getDouble() * 2 - 1);
      }
    for (Vec2 pos1 : Rectangle((width - 1) / a, (width - 1) / a + 1)) {
      Vec2 pos = pos1 * a;
      double avg = 0;
      int num = 0;
      addAvg(pos.x + a / 2, pos.y - a / 2, wys, avg, num);
      addAvg(pos.x, pos.y, wys, avg, num);
      addAvg(pos.x + a, pos.y, wys, avg, num);
      addAvg(pos.x + a / 2, pos.y + a / 2, wys, avg, num);
      wys[pos.x + a / 2][pos.y] =
          avg / num + variance * (random.getDouble() * 2 - 1);
    }
    for (Vec2 pos1 : Rectangle((width - 1) / a + 1, (width - 1) / a)) {
      Vec2 pos = pos1 * a;
      double avg = 0;
      int num = 0;
      addAvg(pos.x - a / 2, pos.y + a / 2, wys, avg, num);
      addAvg(pos.x, pos.y, wys, avg, num);
      addAvg(pos.x, pos.y + a , wys, avg, num);
      addAvg(pos.x + a / 2, pos.y + a / 2, wys, avg, num);
      wys[pos.x][pos.y + a / 2] =
          avg / num + variance * (random.getDouble() * 2 - 1);
    }
    variance *= varianceMult;
  }
  Table<double> ret(area);
  Vec2 offset(area.left(), area.top());
  for (Vec2 v : area) {
    Vec2 lv((v.x - offset.x) * width / area.width(), (v.y - offset.y) * width / area.height());
    ret[v] = wys[lv];
  }
  return ret;
}

bool make(const Generators::NoiseMap& g, Canvas c, RandomGen& r) {
  if (c.area.empty())
    return true;
  auto map = genNoiseMap(r, c.area, NoiseInit { 1, 1, 1, 1, 0 }, 0.45);
  vector<double> all;
  auto getValue = [&all](double r) {
    int index = max(0, int(r * all.size()));
    if (index >= all.size())
      return all.back() + 1;
    return all[index];
  };
  for (auto v : map.getBounds())
    all.push_back(map[v]);
  sort(all.begin(), all.end());
  for (auto& generator : g.generators) {
    auto lower = getValue(generator.lower);
    auto upper = getValue(generator.upper);
    for (auto v : c.area)
      if (map[v] >= lower && map[v] < upper)
        if (!generator.generator->make(c.with(Rectangle(v, v + Vec2(1, 1))), r))
          return false;
  }
  return true;
}

bool make(const Generators::Chain& g, Canvas c, RandomGen& r) {
  for (auto& gen : g.generators)
    if (!gen.make(c, r))
      return false;
  return true;
}

const Generators::Connect::Elem* getConnectorElem(const Generators::Connect& g, Canvas c, RandomGen& r, Vec2 p1) {
  const Generators::Connect::Elem* ret = nullptr;
  for (auto& elem : g.elems)
    if (elem.predicate.apply(c.map, p1, r) && (!ret || !ret->cost || (elem.cost && *ret->cost > *elem.cost)))
      ret = &elem;
  return ret;
}

bool connect(const Generators::Connect& g, Canvas c, RandomGen& r, Vec2 p1, Vec2 p2) {
  ShortestPath path(c.area,
      [&](Vec2 pos) {
        auto elem = getConnectorElem(g, c, r, pos);
        return !elem ? 1 : elem->cost.value_or(ShortestPath::infinity); },
      [p2] (Vec2 to) { return p2.dist4(to); },
      Vec2::directions4(), p1, p2);
  for (Vec2 v = p2; v != p1; v = path.getNextMove(v)) {
    if (auto elem = getConnectorElem(g, c, r, v)) {
      assert(!!elem->cost);
      if (!elem->generator->make(c.with(Rectangle(v, v + Vec2(1, 1))), r))
        return false;
    }
  }
  return true;
}

bool make(const Generators::Connect& g, Canvas c, RandomGen& r) {
  vector<Vec2> points = c.area.getAllSquares().filter(
      [&](Vec2 v) { return g.toConnect.apply(c.map, v, r); });
  Vec2 p1;
  for (int i : Range(30)) {
    p1 = r.choose(points);
    auto p2 = r.choose(points);
    if (p1 != p2 && !connect(g, c, r, p1, p2))
      return false;
  }
  return true;
}

bool Generator::make(Canvas c, RandomGen& r) const {
  return visit<bool>([&c, &r] (const auto& g) { return ::make(g, c, r); } );
}
