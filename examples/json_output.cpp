#include <cli_app/cli_app.hpp>

using namespace vix::cli_app;

int main(int argc, const char **argv)
{
  CliApp app;

  app.set_program("jsoncli");

  app.add_command({"ok",
                   "Return success",
                   "",
                   [&](const Args &) -> int
                   {
                     app.print_ok("operation successful");
                     return 0;
                   }});

  app.add_command({"fail",
                   "Return error",
                   "",
                   [&](const Args &) -> int
                   {
                     app.print_error("operation failed");
                     return 1;
                   }});

  return app.run_cli(argc, argv);
}

// jsoncli ok --json
