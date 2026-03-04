#include <cli_app/cli_app.hpp>

#include <cassert>
#include <iostream>

using namespace vix::cli_app;

int main()
{
  CliApp app;

  app.set_program("test_cli");
  app.set_version("0.1.0");
  app.set_summary("CLI test");

  bool executed = false;

  app.add_command({"hello",
                   "simple hello command",
                   "",
                   [&](const Args &args) -> int
                   {
                     executed = true;

                     if (!args.positionals.empty())
                       std::cout << "Hello " << args.positionals[0] << "\n";
                     else
                       std::cout << "Hello\n";

                     return 0;
                   }});

  const char *argv[] = {
      "test_cli",
      "hello",
      "world"};

  int rc = app.run_cli(3, argv);

  assert(rc == 0);
  assert(executed);

  std::cout << "cli_app basic test passed\n";

  return 0;
}
