#pragma once

#include "stdafx.h"
#include "serialization.h"

[[noreturn]] inline void fail() {
  throw std::runtime_error{"error"};
}

template <class T>
class HeapAllocated {
  public:
  template <typename... Args>
  HeapAllocated(Args... a) : elem(new T(a...)) {}

  HeapAllocated(T&& o) : elem(new T(std::move(o))) {}

  HeapAllocated(const HeapAllocated& o) noexcept : elem(new T(*o)) {}
  HeapAllocated(HeapAllocated&& o) noexcept : elem(std::move(o.elem)) {}

  T* operator -> () {
    return elem.get();
  }

  const T* operator -> () const {
    return elem.get();
  }

  T& operator * () {
    return *elem.get();
  }

  const T& operator * () const {
    return *elem.get();
  }

  const T* get() const {
    return elem.get();
  }

  T* get() {
    return elem.get();
  }

  void reset(T&& t) {
    elem.reset(new T(std::move(t)));
  }

  HeapAllocated& operator = (const HeapAllocated& t) {
    *elem.get() = *t;
    return *this;
  }

  HeapAllocated& operator = (HeapAllocated&& t) noexcept {
    elem = std::move(t.elem);
    return *this;
  }

  bool operator == (const HeapAllocated& o) const {
    return *elem == *o.elem;
  }

  template <class Archive>
  void serialize(Archive& ar1) {
    if (!elem && Archive::is_loading::value) {
      elem = unique<T>();
    }
    assert(!!elem);
    ar1(*elem);
  }

#ifdef MEM_USAGE_TEST
  void serialize(MemUsageArchive& ar1) {
    ar1.addUsage(sizeof(T));
    CHECK(!!elem);
    ar1(*elem);
  }
#endif

  protected:
  unique_ptr<T> elem;
};

template <class T> constexpr bool isOneOf(const T& value) {
  return false;
}
template <class T, class Arg1, class... Args>
constexpr bool isOneOf(const T& value, const Arg1& arg1, const Args&... args) {
  return value == arg1 || isOneOf(value, args...);
}

inline std::string operator "" _s(const char* str, size_t) { return string(str); }

class Range {
  public:
  Range(int start, int end);
  explicit Range(int end);
  static Range singleElem(int);

  bool isEmpty() const;
  Range reverse();
  Range shorten(int r);

  int getStart() const;
  int getEnd() const;
  int getLength() const;
  bool contains(int) const;
  bool intersects(Range) const;
  Range intersection(Range) const;

  bool operator == (const Range&) const;
  Range operator + (int) const;
  Range operator - (int) const;

  class Iter {
    public:
    Iter(int ind, int min, int max, int increment);

    int operator* () const;
    bool operator != (const Iter& other) const;

    const Iter& operator++ ();

    private:
    int ind;
    //int min;
    //int max;
    int increment;
  };

  Iter begin();
  Iter end();

  private:
  Range(int start, int end, int increment);
  int SERIAL(start) = 0; // HASH(start)
  int SERIAL(finish) = 0; // HASH(finish)
  int SERIAL(increment) = 1; // HASH(increment)
};

template <class T>
Range All(const T& container) {
  return Range(container.size());
}

class Rectangle;
class RandomGen;
class PrettyInputArchive;

class Vec2 {
  public:
  int SERIAL(x); // HASH(x)
  int SERIAL(y); // HASH(y)
  Vec2() : x(0), y(0) {}
  Vec2(int x, int y);
  bool inRectangle(int px, int py, int kx, int ky) const;
  bool inRectangle(const Rectangle&) const;
  bool operator == (const Vec2& v) const;
  bool operator != (const Vec2& v) const;
  Vec2 operator + (const Vec2& v) const;
  Vec2 operator * (int) const;
  Vec2 operator * (double) const;
  Vec2 operator / (int) const;
  Vec2& operator += (const Vec2& v);
  Vec2 operator - (const Vec2& v) const;
  Vec2& operator -= (const Vec2& v);
  Vec2 operator - () const;
  bool operator < (Vec2) const;
  Vec2 mult(const Vec2& v) const;
  Vec2 div(const Vec2& v) const;
  static int dotProduct(Vec2 a, Vec2 b);
  int length8() const;
  int length4() const;
  int dist8(Vec2) const;
  int dist4(Vec2) const;
  double distD(Vec2) const;
  double lengthD() const;
  Vec2 shorten() const;
  pair<Vec2, Vec2> approxL1() const;
  Vec2 getBearing() const;
  bool isCardinal4() const;
  bool isCardinal8() const;
  static Vec2 getCenterOfWeight(vector<Vec2>);

  static const vector<Vec2>& directions8();
  vector<Vec2> neighbors8() const;
  static const vector<Vec2>& directions4();
  vector<Vec2> neighbors4() const;
  static vector<Vec2> corners();
  static vector<set<Vec2>> calculateLayers(set<Vec2>);

  typedef function<Vec2(Vec2)> LinearMap;

  void serialize(PrettyInputArchive& ar, const unsigned int version);
};

class Rectangle {
  public:
  friend class Vec2;
  template<typename T>
  friend class Table;
  Rectangle(int width, int height);
  explicit Rectangle(Vec2 dim);
  Rectangle(int px, int py, int kx, int ky);
  Rectangle(Vec2 p, Vec2 k);
  Rectangle(Range xRange, Range yRange);
  static Rectangle boundingBox(const vector<Vec2>& v);
  static Rectangle centered(Vec2 center, int radius);
  static Rectangle centered(int radius);

  int left() const;
  int top() const;
  int right() const;
  int bottom() const;
  int width() const;
  int height() const;
  Vec2 getSize() const;
  Range getYRange() const;
  Range getXRange() const;
  int area() const;
  bool empty() const;

  Vec2 topLeft() const;
  Vec2 bottomRight() const;
  Vec2 topRight() const;
  Vec2 bottomLeft() const;

  bool intersects(const Rectangle& other) const;
  bool contains(const Rectangle& other) const;
  Rectangle intersection(const Rectangle& other) const;
  // can be negative if rectangles intersect
  int getDistance(const Rectangle& other) const;

  Rectangle minusMargin(int margin) const;
  Rectangle translate(Vec2 v) const;
  Rectangle apply(Vec2::LinearMap) const;

  Vec2 random(RandomGen&) const;
  Vec2 middle() const;

  vector<Vec2> getAllSquares() const;

  bool operator == (const Rectangle&) const;
  bool operator != (const Rectangle&) const;

  class Iter {
    public:
    Iter(int x, int y, int px, int py, int kx, int ky);

    Vec2 operator* () const;
    bool operator != (const Iter& other) const;

    const Iter& operator++ ();

    private:
    Vec2 pos;
    int /*px, */py, /*kx, */ky;
  };

  Iter begin() const;
  Iter end() const;

  private:
  int SERIAL(px) = 0;
  int SERIAL(py) = 0;
  int SERIAL(kx) = 0;
  int SERIAL(ky) = 0;
  int SERIAL(w) = 0;
  int SERIAL(h) = 0;
};

template <class T>
class Table {
  public:
  Table(Table&& t) noexcept = default;

  Table(const Table& t) : Table(t.bounds) {
    for (int i : Range(bounds.w * bounds.h))
      mem[i] = t.mem[i];
  }

  Table(int x, int y, int w, int h) : Table(Rectangle(x, y, x + w, y + h)) {
  }

  Table(const Rectangle& rect) : bounds(rect), mem(new T[rect.w * rect.h]){
    for (int i : Range(bounds.w * bounds.h))
      mem[i] = T();
  }

  Table(const Rectangle& rect, const T& value) : Table(rect) {
    for (int i : Range(bounds.w * bounds.h))
      mem[i] = value;
  }

  Table(int w, int h) : Table(0, 0, w, h) {
  }

  Table(Vec2 size) : Table(size.x, size.y) {
  }

  Table(int x, int y, int width, int height, const T& value) : Table(Rectangle(x, y, x + width, y + height), value) {
  }

  Table(int width, int height, const T& value) : Table(0, 0, width, height, value) {
  }

  Table(Vec2 size, const T& value) : Table(size.x, size.y, value) {
  }

  const Rectangle& getBounds() const {
    return bounds;
  }

  Table& operator = (Table&& other) noexcept = default;
  Table& operator = (const Table& other) {
    bounds = other.bounds;
    mem.reset(new T[bounds.w * bounds.h]);
    for (int i : Range(bounds.w * bounds.h))
      mem[i] = other.mem[i];
    return *this;
  }

  int getWidth() const {
    return bounds.w;
  }
  int getHeight() const {
    return bounds.h;
  }

  class RowAccess {
    public:
    RowAccess(T* m, int p, int w) : px(p), width(w), mem(m) {}
    T& operator[](int ind) {
      assert(ind >= px && ind < px + width);
      return mem[ind - px];
    }

    const T& operator[](int ind) const {
      assert(ind >= px && ind < px + width);
      return mem[ind - px];
    }

    private:
    int px;
    int width;
    T* mem;
  };

  RowAccess operator[](int ind) {
    return RowAccess(mem.get() + (ind - bounds.px) * bounds.h, bounds.py, bounds.h);
  }

  RowAccess operator[](int ind) const {
    return RowAccess(mem.get() + (ind - bounds.px) * bounds.h, bounds.py, bounds.h);
  }

  T& operator[](const Vec2& vAbs) {
    assert(vAbs.inRectangle(bounds));// << "Table index out of bounds " << bounds << " " << vAbs;
    return mem[(vAbs.x - bounds.px) * bounds.h + vAbs.y - bounds.py];
  }

  const T& operator[](const Vec2& vAbs) const {
    assert(vAbs.inRectangle(bounds));// << "Table index out of bounds " << bounds << " " << vAbs;
    return mem[(vAbs.x - bounds.px) * bounds.h + vAbs.y - bounds.py];
  }

  private:
  Rectangle bounds;
  unique_ptr<T[]> mem;
};

class RandomGen {
  public:
  RandomGen();
  RandomGen(const RandomGen&) = delete;
  RandomGen(RandomGen&&) = default;
  void init(int seed);
  int get(int max);
  long long getLL();
  int get(int min, int max);
  int get(Range);
  int get(const vector<double>& weights);
  double getDouble();
  double getDouble(double a, double b);
  pair<float, float> getFloat2Fast();
  float getFloat(float a, float b);
  float getFloatFast(float a, float b);
  bool roll(int chance);
  bool chance(double chance);
  bool chance(float chance);
  template <typename T>
  T choose(const vector<T>& v, const vector<double>& p) {
    assert(v.size() == p.size());
    return v[get(p)];
  }

  template <typename T>
  T choose(const vector<T>& v) {
    vector<double> pi(v.size(), 1);
    return choose(v, pi);
  }

  template <typename T>
  T choose(vector<T>&& v, const vector<double>& p) {
    CHECK(v.size() == p.size());
    return std::move(v[get(p)]);
  }

  template <typename T>
  T choose(vector<T>&& v) {
    vector<double> pi(v.size(), 1);
    return choose(std::move(v), pi);
  }

  template <typename T>
  T choose(const set<T>& vi) {
    vector<T> v(vi.size());
    std::copy(vi.begin(), vi.end(), v.begin());
    return choose(v);
  }

  template <typename T, typename Hash>
  T choose(const unordered_set<T, Hash>& vi) {
    vector<T> v(vi.size());
    std::copy(vi.begin(), vi.end(), v.begin());
    return choose(v);
  }

  template <typename T>
  T choose(initializer_list<T> vi, initializer_list<double> pi) {
    return choose(vector<T>(vi), vector<double>(pi));
  }

  template <typename T>
  T choose(vector<pair<int, T>> vi) {
    vector<T> v;
    vector<double> p;
    for (auto elem : vi) {
      v.push_back(elem.second);
      p.push_back(elem.first);
    }
    return choose(v, p);
  }

  template <typename T>
  T choose(vector<pair<T, double>> vi) {
    vector<T> v;
    vector<double> p;
    for (auto elem : vi) {
      v.push_back(elem.first);
      p.push_back(elem.second);
    }
    return choose(v, p);
  }

  template <typename T>
  vector<T> permutation(vector<T> v) {
    std::shuffle(v.begin(), v.end(), generator);
    return v;
  }

  template <typename Iterator>
  void shuffle(Iterator begin, Iterator end) {
    std::shuffle(begin, end, generator);
  }

  template <typename T>
  vector<T> permutation(const set<T>& vi) {
    vector<T> v(vi.size());
    std::copy(vi.begin(), vi.end(), v.begin());
    return permutation(v);
  }

  template <typename T, typename Hash>
  vector<T> permutation(const unordered_set<T, Hash>& vi) {
    vector<T> v(vi.size());
    std::copy(vi.begin(), vi.end(), v.begin());
    return permutation(v);
  }

  template <typename T>
  vector<T> permutation(initializer_list<T> vi) {
    vector<T> v(vi);
    std::random_shuffle(v.begin(), v.end(), [this](int a) { return get(a);});
    return v;
  }

  vector<int> permutation(Range r) {
    vector<int> v;
    for (int i : r)
      v.push_back(i);
    std::random_shuffle(v.begin(), v.end(), [this](int a) { return get(a);});
    return v;
  }

  template <typename T>
  vector<T> chooseN(int n, vector<T> v) {
    CHECK(n <= v.size());
    std::random_shuffle(v.begin(), v.end(), [this](int a) { return get(a);});
    return v.getPrefix(n);
  }

  template <typename T>
  vector<T> chooseN(int n, initializer_list<T> v) {
    return chooseN(n, vector<T>(v));
  }

  template <typename T, typename... Args>
  T&& choose(T&& first, T&& second, Args&&... rest) {
    return chooseImpl(std::forward<T>(first), 2, std::forward<T>(second), std::forward<Args>(rest)...);
  }

  private:
  std::mt19937 generator;
  std::uniform_real_distribution<double> defaultDist;

  template <typename T>
  T&& chooseImpl(T&& cur, int total) {
    return std::forward<T>(cur);
  }

  template <typename T, typename... Args>
  T&& chooseImpl(T&& chosen, int total,  T&& next, Args&&... rest) {
    return chooseImpl(roll(total) ? std::forward<T>(next) : std::forward<T>(chosen), total + 1, std::forward<Args>(rest)...);
  }
};

vector<string> split(const string& s, const std::initializer_list<char>& delim);

template<typename T>
class EnumInfo {
};

#define RICH_ENUM2(Type, Name, ...) \
enum class Name : Type { __VA_ARGS__ };\
template<> \
class EnumInfo<Name> { \
  public:\
  static const char* getName() {\
    return #Name;\
  }\
  static string getString(Name e) {\
    static vector<string> names = split(#__VA_ARGS__, {' ', ','}).filter([](const string& s){ return !s.empty(); });\
    return names[int(e)];\
  }\
  enum Tmp { __VA_ARGS__, size};\
  static Name fromString(const string& s) { \
    for (int i : Range(size)) \
      if (getString(Name(i)) == s) \
        return Name(i); \
    fail();\
  }\
  static optional<Name> fromStringSafe(const string& s) {\
    for (int i : Range(size)) \
      if (getString(Name(i)) == s) \
        return Name(i); \
    return none;\
  }\
}
#define RICH_ENUM(Name, ...) RICH_ENUM2(int, Name, __VA_ARGS__)

template<typename T>
class DirtyTable {
  public:
  DirtyTable(Rectangle bounds, T dirty) : val(bounds), dirty(bounds, 0), dirtyVal(dirty) {}

  const T& getValue(Vec2 v) const {
    return dirty[v] < counter ? dirtyVal : val[v];
  }

  bool isDirty(Vec2 v) const {
    return dirty[v] == counter;
  }

  T& getDirtyValue(Vec2 v) {
    assert(isDirty(v));
    return val[v];
  }

  void setValue(Vec2 v, const T& d) {
    val[v] = d;
    dirty[v] = counter;
  }

  const Rectangle& getBounds() const {
    return val.getBounds();
  }

  void clear() {
    ++counter;
  }

  void clear(Vec2 v) {
    dirty[v] = counter - 1;
  }

  private:
  Table<T> val;
  Table<int> dirty;
  T dirtyVal;
  int counter = 1;
};
