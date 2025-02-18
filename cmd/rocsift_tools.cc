#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <map>
#include <string_view>
#include <string>
#include <unistd.h>
#include <vector>
#include <sys/wait.h>

#include "CLI/CLI.hpp"

#if __has_include(<filesystem>)
#include <filesystem>
namespace std_fs = std::filesystem;
#elif __has_include(<experimental/filesystem>)
#include <experimental/filesystem>
namespace std_fs = std::experimental::filesystem;
#else
error "Missing the <filesystem> header."
#endif

typedef std::map<std::string, std_fs::path> RocsiftTools;

static const std_fs::path ROCSIFT_TOOLS_DEFAULT_PATH = {"/usr/lib/rocsift-tools"};

static inline std::string get_envvar(const std::string &var)
{
  auto val = std::getenv(var.c_str());
  return val ? std::string(val) : std::string("");
}

template <typename Map, typename T>
bool contains(const Map &m, const T &x)
{
  return m.find(x) != m.cend();
}

static inline bool is_rocsift_tool(const std_fs::directory_entry &dir_ent)
{
  const auto exec_perms = (std_fs::perms::owner_exec | std_fs::perms::group_exec |
                           std_fs::perms::others_exec);
  const auto path = dir_ent.path();
  const auto stat = dir_ent.status();

#if __has_include(<filesystem>)
  return (dir_ent.is_regular_file()
#elif __has_include(<experimental/filesystem>)
  return (std_fs::is_regular_file(dir_ent.path())
#endif
          // Apps don't use extensions
          && path.extension().empty()
          // Apps are executable
          && (stat.permissions() & exec_perms) != std_fs::perms::none
          // Don't include ourselves :)
          && path.filename() != "rocsift-tools");
}

static inline std_fs::path get_rocsift_tools_path()
{
  auto path = std_fs::path(get_envvar("ROCSIFT_TOOLS_PATH"));
  // Fall-back to default - if this path doesn't exist we just catch the
  // exception later when trying to iterate over the directory.
  if (path.empty() || !std_fs::is_directory(path)) {
    path = ROCSIFT_TOOLS_DEFAULT_PATH;
  }

  return path;
}

static inline RocsiftTools find_tools(const std_fs::path &path)
{
  auto tools = RocsiftTools();

  try {
    for (auto const &dir_ent : std_fs::directory_iterator{path}) {
      if (is_rocsift_tool(dir_ent)) {
        tools.emplace(std::make_pair(dir_ent.path().filename(), dir_ent.path()));
      }
    }
  } catch (const std_fs::filesystem_error &) {
    std::cerr << "ERROR: rocsift-tools directory path " << path << " does not exist" << std::endl;
  }

  return tools;
}

[[noreturn]] static inline void exec_tool(const std_fs::path &path,
                                          const std::vector<std::string> &args)
{
  std::vector<char *> args_cstr;

  // The first argument is the executable's filename
  args_cstr.push_back(const_cast<char *>(path.filename().c_str()));

  // Convert std::vector<std::string> to std::vector<char *> for the crusty
  // C execv() function call.
  std::transform(args.begin(), args.end(), std::back_inserter(args_cstr),
                 [](const std::string &s) -> char * { return const_cast<char *>(s.c_str()); });

  // Must terminate the argument array with a null-pointer
  args_cstr.push_back(NULL);

  int rc = execv(path.c_str(), args_cstr.data());

  // execv() *should not* return - if it does then something's very wrong.
  std::cerr << "ERROR: execv(...) returned " << rc << " and set errno: " << std::strerror(errno)
            << std::endl;
  std::exit(1);
}

int main(int argc, char **argv)
{
  CLI::App app;

  const auto path = get_rocsift_tools_path();
  const auto tools = find_tools(path);
  bool list_tools = false;
  std::string tool;
  std::vector<std::string> tool_args;

  app.prefix_command();

  app.add_option("subcommand", tool, "'list' to list the name of available tools, or tool name");
  app.set_help_flag();

  CLI11_PARSE(app, argc, argv);
  try {
    if (!tool.size()) {
      throw CLI::CallForHelp();
    }
  } catch (const CLI::Error &e) {
    return app.exit(e);
  }

  if (tool == "list") {
    list_tools = true;
  }

  if (list_tools) {
    for (auto const &tool : tools) {
      std::cout << tool.first << "\n";
    }
    std::cout << std::flush;
    return 0;
  }

  tool_args = app.remaining();

  if (tool.size() && contains(tools, tool)) {
    exec_tool(tools.at(tool), tool_args);  // Does not return!
  } else {
    std::cerr << "ERROR: rocsift-tools " << tool << " not found in " << path << std::endl;
    return 1;
  }

  return 0;
}
