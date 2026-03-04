#include <cli_app/cli_app.hpp>
#include <iostream>

using namespace vix::cli_app;

int main(int argc, const char **argv)
{
  CliApp app;

  app.set_program("example");
  app.set_version("0.1.0");
  app.set_summary("Minimal CLI example");

  app.add_command({"hello",
                   "Say hello",
                   "",
                   [](const Args &args) -> int
                   {
                     if (!args.positionals.empty())
                       std::cout << "Hello " << args.positionals[0] << "\n";
                     else
                       std::cout << "Hello\n";

                     return 0;
                   }});

  return app.run_cli(argc, argv);
}
// example hello world
