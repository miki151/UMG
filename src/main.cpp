#include "generator.h"
#include "ProgramOptions.h"
#include "canvas.h"
#include "render.h"
#include "umg_include.h"

static po::parser getCommandLineFlags(int argc, char* argv[]) {
  po::parser flags;
  flags[""].type(po::string).description("Path to the input program.");
  flags["seed"].type(po::i32).description("Random seed.");
  flags["render"].type(po::string).description("Path to file with glyph definitions.");
  flags["size"].type(po::i32).description("Size of the map.");
  if (!flags.parseArgs(argc, argv))
    exit(-1);
  return flags;
}

static ifstream openFile(const string& path) {
  ifstream in(path);
  if (!in.good()) {
    std::cout << "Failed to open file: " << path << "\n";
    exit(-1);
  }
  return in;
}

static LayoutGenerator readLayoutGenerator(const string& path) {
  stringstream ss;
  ss << openFile(path).rdbuf();
  string input = ss.str();
  LayoutGenerator gen;
  PrettyInputArchive ar({string(umgInclude), input}, {"include.umg", path}, nullptr);
  try {
    ar(gen);
  } catch (PrettyException& ex) {
    std::cout << ex.text << "\n";
    exit(-1);
  }
  return gen;
}

static LayoutCanvas::Map generateMap(const LayoutGenerator& gen, int size, RandomGen& random) {
  LayoutCanvas::Map map{Table<vector<Token>>(size, size)};
  if (!gen.make(LayoutCanvas{map.elems.getBounds(), &map}, random)) {
    std::cout << "Generation failed.\n";
    exit(-1);
  }
  return map;
}

static string getInputPath(po::parser& flags) {
  if (!flags[""].was_set()) {
    std::cout << flags << endl;
    exit(-1);
  }
  return flags[""].get().string;
}

static int getMapSize(po::parser& flags) {
  if (flags["size"].was_set())
    return flags["size"].get().i32;
  return 10;
}

static RandomGen getRNG(po::parser& flags) {
  RandomGen random;
  int seed = int(time(nullptr));
  if (flags["seed"].was_set())
    seed = flags["seed"].get().i32;
  random.init(seed);
  return random;
}

static milliseconds getRealMillis() {
  return duration_cast<milliseconds>(steady_clock::now().time_since_epoch());
}

extern "C" {
const char* get_result(char* input, char* renderer, int size, int seed) {
  RandomGen random;
  if (seed == 0)
    seed = getRealMillis().count();
  random.init(seed);
  LayoutGenerator gen;
  PrettyInputArchive ar({string(umgInclude), string(input)}, {}, nullptr);
  auto get_error = [] (const string& s) {
    return (new string("<font color=red>" + s + "</font>"))->c_str();
  };
  if (size < 1 || size > 30)
    return get_error("Bad map size: " + to_string(size));
  try {
    ar(gen);
  } catch (PrettyException& ex) {
    return get_error(ex.text);
  }
  LayoutCanvas::Map map{Table<vector<Token>>(size, size)};
  if (!gen.make(LayoutCanvas{map.elems.getBounds(), &map}, random)) {
    return get_error("Generation failed.");
  }
  return (new string(renderHtml(map, renderer)))->c_str();
}
}

int main(int argc, char* argv[]) {
  po::parser flags = getCommandLineFlags(argc, argv);
  auto gen = readLayoutGenerator(getInputPath(flags));
  int size = getMapSize(flags);
  auto random = getRNG(flags);
  auto map1 = generateMap(gen, size, random);
  if (flags["render"].was_set()) {
    auto file = openFile(flags["render"].get().string);
    renderAscii(map1, file);
  } else
    for (auto v : map1.elems.getBounds()) {
      for (auto t : map1.elems[v])
        std::cout << t << ", ";
      std::cout << "\n";
    }
  return 0;
}
