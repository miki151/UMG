#include "render.h"
#include "canvas.h"

string getColorCode(const string& color) {
  auto number = [&] {
    if (color == "black")
      return 30;
    if (color == "red")
      return 31;
    if (color == "green")
      return 32;
    if (color == "brown")
      return 33;
    if (color == "yellow")
      return 93;
    if (color == "blue")
      return 34;
    if (color == "magenta")
      return 35;
    if (color == "cyan")
      return 36;
    if (color == "white")
      return 37;
    if (color == "gray")
      return 90;
    std::cout << "Unknown color: " << color << "\n";
    return 37;
  }();
  return "\033[" + to_string(number) + "m";
}

template <typename Container, typename Fun>
auto chooseBest(const Container& c, Fun f) {
  auto ret = *c.begin();
  int p = f(ret);
  for (auto& elem : c) {
    int np = f(elem);
    if (np < p) {
      p = np;
      ret = elem;
    }
  }
  return ret;
}

void renderAscii(const Map& map1, istream& file) {
  unordered_map<string, string> tokens;
  unordered_map<string, int> priority;
  int cnt = 0;
  while (1) {
    string token, character, color;
    file >> std::quoted(token) >> character >> color;
    if (!file)
      break;
    tokens[token] = getColorCode(color) + character + "\033[0m";
    priority[token] = cnt++;
  }
  for (auto y : map1.elems.getBounds().getYRange()) {
    for (auto x : map1.elems.getBounds().getXRange()) {
      auto& elems = map1.elems[x][y];
      if (!elems.empty()) {
        auto glyph = chooseBest(elems, [&](const Token& t) {
          if (priority.count(t))
            return -priority.at(t);
          else
            return 10000;
        });
        if (tokens.count(glyph)) {
          std::cout << tokens.at(glyph);
          continue;
        }
      }
      std::cout << " ";
    }
    std::cout << "\n";
  }
}

static string getHtmlColor(string c, const string& color) {
  return "<font color=\"" + color + "\">" + c + "</font>";
}

string renderHtml(const Map& map1, const char* renderer) {
  string ret;
  unordered_map<string, string> tokens;
  unordered_map<string, int> priority;
  int cnt = 0;
  istringstream file(renderer);
  while (1) {
    string token, character, color;
    file >> std::quoted(token) >> character >> color;
    if (!file)
      break;
    tokens[token] = getHtmlColor(character, color);
    priority[token] = cnt++;
  }
  for (auto y : map1.elems.getBounds().getYRange()) {
    for (auto x : map1.elems.getBounds().getXRange()) {
      auto& elems = map1.elems[x][y];
      if (!elems.empty()) {
        auto glyph = chooseBest(elems, [&](const Token& t) {
          if (priority.count(t))
            return -priority.at(t);
          else
            return 10000;
        });
        if (tokens.count(glyph)) {
          ret += tokens.at(glyph);
          continue;
        }
      }
      ret += " ";
    }
    ret += "<br/>";
  }
  return ret;
}
