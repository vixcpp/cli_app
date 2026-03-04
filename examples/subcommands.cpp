#include <cli_app/cli_app.hpp>
#include <iostream>

using namespace vix::cli_app;

int main(int argc, const char **argv)
{
  CliApp app;

  app.set_program("tool");
  app.set_summary("Multi-command CLI");

  app.add_command({"version",
                   "Print version",
                   "",
                   [](const Args &) -> int
                   {
                     std::cout << "tool version 1.0\n";
                     return 0;
                   }});

  app.add_command({"status",
                   "Show system status",
                   "",
                   [](const Args &) -> int
                   {
                     std::cout << "system ok\n";
                     return 0;
                   }});

  return app.run_cli(argc, argv);
}
// tool status
