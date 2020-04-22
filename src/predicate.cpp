#include "predicate.h"
#include "canvas.h"


static bool apply(const Predicates::On& p, Map* map, Vec2 v, RandomGen& r) {
  return map->elems[v].count(p.token);
}

static bool apply(const Predicates::Not& p, Map* map, Vec2 v, RandomGen& r) {
  return !p.predicate->apply(map, v, r);
}

static bool apply(const Predicates::True& p, Map* map, Vec2 v, RandomGen& r) {
  return true;
}

static bool apply(const Predicates::And& p, Map* map, Vec2 v, RandomGen& r) {
  for (auto& pred : p.predicates)
    if (!pred.apply(map, v, r))
      return false;
  return true;
}

static bool apply(const Predicates::Or& p, Map* map, Vec2 v, RandomGen& r) {
  for (auto& pred : p.predicates)
    if (pred.apply(map, v, r))
      return true;
  return false;
}

static bool apply(const Predicates::Chance& p, Map* map, Vec2 v, RandomGen& r) {
  return r.chance(p.value);
}

bool Predicate::apply(Map* map, Vec2 v, RandomGen& r) const {
  return visit<bool>([&](const auto& p) { return ::apply(p, map, v, r); });
}
