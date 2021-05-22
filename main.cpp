#include <raylib.h>
#include <cmath>

#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <filesystem>
#include <limits>
#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>
#include <chrono>

using std::string;
using std::vector;
using std::map;
using std::ifstream;
using std::ostream;

namespace fs = std::filesystem;

#include "object.hpp"
#include "gm.hpp"
#include "conf.hpp"
#include "messenger.hpp"
#include "msgid.hpp"
#include "util.hpp"
#include "net.hpp"

#include "game/Main.hpp"
