#pragma once

#include "stdafx.h"
#include "util.h"
#include "token.h"
#include "pretty_archive.h"
#include "predicate.h"

struct Generator;
struct Canvas;

RICH_ENUM(MarginType, TOP, BOTTOM, LEFT, RIGHT);
RICH_ENUM(PlacementPos, MIDDLE);

namespace Generators {

struct None {
  SERIALIZE_EMPTY()
};

struct Set {
  vector<Token> SERIAL(tokens);
  SERIALIZE_ALL(tokens)
};

struct Reset {
  vector<Token> SERIAL(tokens);
  SERIALIZE_ALL(tokens)
};

struct SetMaybe {
  Predicate SERIAL(predicate);
  vector<Token> SERIAL(tokens);
  SERIALIZE_ALL(predicate, tokens)
};

struct Remove {
  vector<Token> SERIAL(tokens);
  SERIALIZE_ALL(tokens)
};

struct Margin {
  MarginType SERIAL(type);
  int SERIAL(width);
  HeapAllocated<Generator> SERIAL(border);
  HeapAllocated<Generator> SERIAL(inside);
  SERIALIZE_ALL(type, width, border, inside)
};

struct Margins {
  int SERIAL(width);
  HeapAllocated<Generator> SERIAL(border);
  HeapAllocated<Generator> SERIAL(inside);
  SERIALIZE_ALL(NAMED(width), NAMED(border), NAMED(inside))
};

struct HRatio {
  double SERIAL(r);
  HeapAllocated<Generator> SERIAL(left);
  HeapAllocated<Generator> SERIAL(right);
  SERIALIZE_ALL(r, left, right)
};

struct VRatio {
  double SERIAL(r);
  HeapAllocated<Generator> SERIAL(top);
  HeapAllocated<Generator> SERIAL(bottom);
  SERIALIZE_ALL(r, top, bottom)
};

struct Place {
  struct Elem {
    Vec2 SERIAL(size);
    HeapAllocated<Generator> SERIAL(generator);
    int SERIAL(count) = 1;
    Predicate SERIAL(predicate) = Predicates::True{};
    optional<PlacementPos> SERIAL(position);
    SERIALIZE_ALL(NAMED(size), NAMED(generator), OPTION(count), OPTION(predicate), OPTION(position))
  };
  vector<Elem> SERIAL(generators);
  SERIALIZE_ALL(generators)
};

struct NoiseMap {
  struct Elem {
    double SERIAL(lower);
    double SERIAL(upper);
    HeapAllocated<Generator> SERIAL(generator);
    SERIALIZE_ALL(lower, upper, generator)
  };
  vector<Elem> SERIAL(generators);
  SERIALIZE_ALL(generators)
};

struct Chain {
  vector<Generator> SERIAL(generators);
  SERIALIZE_ALL(generators)
};

struct Call {
  string SERIAL(name);
  SERIALIZE_ALL(name)
};

struct Connect {
  struct Elem {
    optional<double> SERIAL(cost);
    Predicate SERIAL(predicate);
    HeapAllocated<Generator> SERIAL(generator);
    SERIALIZE_ALL(cost, predicate, generator)
  };
  Predicate SERIAL(toConnect);
  vector<Elem> SERIAL(elems);
  SERIALIZE_ALL(toConnect, elems)
};

#define VARIANT_TYPES_LIST\
  X(None, 0)\
  X(Set, 1)\
  X(Reset, 2)\
  X(SetMaybe, 3)\
  X(Remove, 4)\
  X(Margin, 5)\
  X(Margins, 6)\
  X(HRatio, 7)\
  X(VRatio, 8)\
  X(Place, 9)\
  X(NoiseMap, 10)\
  X(Chain, 11)\
  X(Connect, 12)

#define VARIANT_NAME GeneratorImpl

#include "gen_variant.h"
inline
#include "gen_variant_serialize_pretty.h"

#undef VARIANT_TYPES_LIST
#undef VARIANT_NAME

}

struct Generator : Generators::GeneratorImpl {
  using GeneratorImpl::GeneratorImpl;
  [[nodiscard]] bool make(Canvas, RandomGen&) const;
};
