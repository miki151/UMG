#include "generator.h"
#include "ProgramOptions.h"
#include "canvas.h"
#include "render.h"

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

static Generator readGenerator(const string& path) {
  stringstream ss;
  ss << openFile(path).rdbuf();
  string input = ss.str();
  Generator gen;
  PrettyInputArchive ar({input}, {path});
  try {
    ar(gen);
  } catch (PrettyException& ex) {
    std::cout << ex.text << "\n";
    exit(-1);
  }
  return gen;
}

static Map generateMap(const Generator& gen, int size, RandomGen& random) {
  Map map{Table<unordered_set<Token>>(size, size)};
  if (!gen.make(Canvas{map.elems.getBounds(), &map}, random)) {
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

int main(int argc, char* argv[]) {
  po::parser flags = getCommandLineFlags(argc, argv);
  auto gen = readGenerator(getInputPath(flags));
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
