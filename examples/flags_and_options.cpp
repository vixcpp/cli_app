#include <cli_app/cli_app.hpp>
#include <iostream>

using namespace vix::cli_app;

int main(int argc, const char **argv)
{
  CliApp app;

  app.set_program("deploy");
  app.set_summary("Deployment CLI");

  app.add_command({"run",
                   "Run deployment",
                   "",
                   [](const Args &args) -> int
                   {
                     if (args.flags.count("v"))
                       std::cout << "verbose mode\n";

                     auto it = args.options.find("env");
                     if (it != args.options.end())
                       std::cout << "env=" << it->second << "\n";

                     std::cout << "deploy running\n";

                     return 0;
                   }});

  return app.run_cli(argc, argv);
}
// deploy run -v --env=prod
