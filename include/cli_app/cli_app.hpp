/**
 * @file cli_app.hpp
 * @brief Command-line application scaffold for Vix built on top of vix/app.
 *
 * `cli_app` provides a minimal, production-friendly CLI foundation:
 *
 * - Argument parsing (flags, options, positional args)
 * - Subcommand routing (git-style: `tool cmd --flag`)
 * - Structured output helpers (text + JSON strings, no JSON dependency)
 * - Exit codes and usage rendering
 * - Lifecycle hooks via vix/app (start, loop, stop)
 *
 * Design goals:
 * - deterministic parsing rules
 * - no hidden global state
 * - no macros required
 * - header-only
 *
 * Requirements: C++17+
 * Header-only. Depends on vix/app.
 */

#ifndef VIX_CLI_APP_CLI_APP_HPP
#define VIX_CLI_APP_CLI_APP_HPP

#include <app/app.hpp>

#include <cctype>
#include <cstdint>
#include <functional>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace vix::cli_app
{
  /**
   * @brief Minimal JSON escaping for deterministic CLI JSON output.
   */
  inline std::string json_escape(std::string_view s)
  {
    std::string out;
    out.reserve(s.size() + 8);

    for (char c : s)
    {
      switch (c)
      {
      case '\\':
        out += "\\\\";
        break;
      case '"':
        out += "\\\"";
        break;
      case '\b':
        out += "\\b";
        break;
      case '\f':
        out += "\\f";
        break;
      case '\n':
        out += "\\n";
        break;
      case '\r':
        out += "\\r";
        break;
      case '\t':
        out += "\\t";
        break;
      default:
        if (static_cast<unsigned char>(c) < 0x20)
        {
          static const char *hex = "0123456789abcdef";
          out += "\\u00";
          out += hex[(static_cast<unsigned char>(c) >> 4) & 0x0F];
          out += hex[(static_cast<unsigned char>(c)) & 0x0F];
        }
        else
        {
          out += c;
        }
        break;
      }
    }

    return out;
  }

  /**
   * @brief Exit codes for CLI programs.
   */
  enum class ExitCode : int
  {
    Ok = 0,
    Usage = 2,
    Failure = 1
  };

  /**
   * @brief Parsed arguments representation.
   */
  struct Args
  {
    // Positional args, excluding the binary name.
    std::vector<std::string> positionals;

    // Flags: --verbose, -v
    std::unordered_map<std::string, bool> flags;

    // Options: --config=path, --config path
    std::unordered_map<std::string, std::string> options;

    // Extra args after `--`
    std::vector<std::string> passthrough;
  };

  /**
   * @brief A single subcommand handler.
   */
  using CommandHandler = std::function<int(const Args &)>;

  /**
   * @brief Definition of a command (for help text + routing).
   */
  struct Command
  {
    std::string name;
    std::string summary;

    // Example usage for this command (optional)
    std::string usage;

    CommandHandler handler;
  };

  /**
   * @brief Minimal CLI application built on top of vix/app lifecycle.
   *
   * Typical usage:
   *
   * - configure name/version
   * - register commands
   * - call run_cli(argc, argv)
   */
  class CliApp : public vix::app::Application
  {
  public:
    CliApp() = default;
    ~CliApp() override = default;

    CliApp(const CliApp &) = delete;
    CliApp &operator=(const CliApp &) = delete;

    CliApp(CliApp &&) = delete;
    CliApp &operator=(CliApp &&) = delete;

    /**
     * @brief Set program name for help.
     */
    void set_program(std::string name)
    {
      program_ = std::move(name);
      if (program_.empty())
        program_ = "cli";
    }

    /**
     * @brief Set program version.
     */
    void set_version(std::string v)
    {
      version_ = std::move(v);
    }

    /**
     * @brief Set program summary shown in help header.
     */
    void set_summary(std::string s)
    {
      summary_ = std::move(s);
    }

    /**
     * @brief Register a subcommand.
     */
    void add_command(Command cmd)
    {
      if (cmd.name.empty())
        throw std::runtime_error("CliApp::add_command: cmd.name must not be empty");
      if (!cmd.handler)
        throw std::runtime_error("CliApp::add_command: cmd.handler must be set");

      commands_[cmd.name] = std::move(cmd);
    }

    /**
     * @brief Enable JSON output mode (affects helpers only).
     */
    void set_json_output(bool on)
    {
      json_output_ = on;
    }

    /**
     * @brief Run CLI with argv parsing and command dispatch.
     *
     * Returns an exit code.
     */
    int run_cli(int argc, const char **argv)
    {
      if (argc <= 0 || argv == nullptr)
        return static_cast<int>(ExitCode::Usage);

      // argv[0] = binary path/name
      if (program_.empty())
        program_ = base_name(argv[0]);

      std::vector<std::string_view> raw;
      raw.reserve(static_cast<std::size_t>(argc > 1 ? argc - 1 : 0));

      for (int i = 1; i < argc; ++i)
        raw.push_back(argv[i]);

      // Handle global help/version
      if (has_token(raw, "--help") || has_token(raw, "-h"))
      {
        print_help();
        return static_cast<int>(ExitCode::Ok);
      }

      if (has_token(raw, "--version") || has_token(raw, "-V"))
      {
        print_version();
        return static_cast<int>(ExitCode::Ok);
      }

      // Optional: global json flag
      if (has_token(raw, "--json"))
        json_output_ = true;

      // Determine command name: first non-flag token
      const auto cmd_name = first_command_token(raw);
      if (!cmd_name.has_value())
      {
        // No command given.
        print_help();
        return static_cast<int>(ExitCode::Usage);
      }

      // Remove command token from raw for parsing.
      std::vector<std::string_view> cmd_args = raw;
      erase_first(cmd_args, *cmd_name);

      // Parse args for command
      Args parsed = parse_args(cmd_args);

      // Dispatch
      const auto it = commands_.find(std::string(*cmd_name));
      if (it == commands_.end())
      {
        print_error("unknown command: " + std::string(*cmd_name));
        print_help();
        return static_cast<int>(ExitCode::Usage);
      }

      try
      {
        return it->second.handler(parsed);
      }
      catch (const std::exception &e)
      {
        print_error(std::string("error: ") + e.what());
        return static_cast<int>(ExitCode::Failure);
      }
      catch (...)
      {
        print_error("error: unknown failure");
        return static_cast<int>(ExitCode::Failure);
      }
    }

    /**
     * @brief Render top-level help.
     */
    void print_help() const
    {
      std::string out;

      if (!summary_.empty())
      {
        out += program_;
        out += " - ";
        out += summary_;
        out += "\n\n";
      }

      out += "Usage:\n";
      out += "  ";
      out += program_;
      out += " <command> [options] [args]\n\n";

      out += "Commands:\n";
      for (const auto &kv : commands_)
      {
        out += "  ";
        out += kv.second.name;
        if (!kv.second.summary.empty())
        {
          out += "  ";
          out += kv.second.summary;
        }
        out += "\n";
      }

      out += "\nGlobal options:\n";
      out += "  -h, --help       Show help\n";
      out += "  -V, --version    Show version\n";
      out += "  --json           JSON output (helpers)\n";

      write_stdout(out);
    }

    /**
     * @brief Render version.
     */
    void print_version() const
    {
      std::string out = program_;
      if (!version_.empty())
      {
        out += " ";
        out += version_;
      }
      out += "\n";
      write_stdout(out);
    }

    /**
     * @brief Helpers: print success in either text or JSON.
     */
    void print_ok(std::string message) const
    {
      if (!json_output_)
      {
        write_stdout(message + "\n");
        return;
      }

      std::string body;
      body.reserve(message.size() + 32);
      body += "{\"ok\":true,\"message\":\"";
      body += json_escape(message);
      body += "\"}\n";
      write_stdout(body);
    }

    /**
     * @brief Helpers: print error in either text or JSON.
     */
    void print_error(std::string message) const
    {
      if (!json_output_)
      {
        write_stderr(message + "\n");
        return;
      }

      std::string body;
      body.reserve(message.size() + 48);
      body += "{\"ok\":false,\"error\":\"";
      body += json_escape(message);
      body += "\"}\n";
      write_stderr(body);
    }

  protected:
    /**
     * @brief CLI apps do not need a run loop by default.
     *
     * vix/app lifecycle expects loop_step(). We keep it a no-op so
     * the base class can still be used for CLI tooling when desired.
     */
    void loop_step() override {}

  private:
    static void write_stdout(const std::string &s)
    {
      // Keep deterministic and dependency-free.
      // Users can override by wrapping CliApp.
      std::fwrite(s.data(), 1, s.size(), stdout);
    }

    static void write_stderr(const std::string &s)
    {
      std::fwrite(s.data(), 1, s.size(), stderr);
    }

    static std::string base_name(const char *p)
    {
      if (!p)
        return "cli";

      std::string_view s(p);
      const auto pos = s.find_last_of("/\\");
      if (pos == std::string_view::npos)
        return std::string(s);
      return std::string(s.substr(pos + 1));
    }

    static bool has_token(const std::vector<std::string_view> &v, std::string_view t)
    {
      for (auto x : v)
      {
        if (x == t)
          return true;
      }
      return false;
    }

    static std::optional<std::string_view> first_command_token(const std::vector<std::string_view> &v)
    {
      for (auto x : v)
      {
        if (x == "--")
          return std::nullopt; // no command after passthrough
        if (x.size() >= 1 && x[0] == '-')
          continue; // global flag
        return x;
      }
      return std::nullopt;
    }

    static void erase_first(std::vector<std::string_view> &v, std::string_view token)
    {
      for (auto it = v.begin(); it != v.end(); ++it)
      {
        if (*it == token)
        {
          v.erase(it);
          return;
        }
      }
    }

    static Args parse_args(const std::vector<std::string_view> &v)
    {
      Args out;
      bool passthrough = false;

      for (std::size_t i = 0; i < v.size(); ++i)
      {
        const std::string_view tok = v[i];

        if (passthrough)
        {
          out.passthrough.push_back(std::string(tok));
          continue;
        }

        if (tok == "--")
        {
          passthrough = true;
          continue;
        }

        // Long option: --key or --key=value
        if (tok.size() >= 3 && tok[0] == '-' && tok[1] == '-')
        {
          const auto eq = tok.find('=');
          if (eq != std::string_view::npos)
          {
            const auto k = tok.substr(2, eq - 2);
            const auto val = tok.substr(eq + 1);
            if (!k.empty())
              out.options[std::string(k)] = std::string(val);
          }
          else
          {
            const auto k = tok.substr(2);
            if (k.empty())
              continue;

            // --flag (no value)
            if (i + 1 >= v.size() || (v[i + 1].size() > 0 && v[i + 1][0] == '-'))
            {
              out.flags[std::string(k)] = true;
            }
            else
            {
              // --key value
              out.options[std::string(k)] = std::string(v[i + 1]);
              i++;
            }
          }

          continue;
        }

        // Short flags: -v or -abc
        if (tok.size() >= 2 && tok[0] == '-' && tok[1] != '-')
        {
          for (std::size_t j = 1; j < tok.size(); ++j)
          {
            const char c = tok[j];
            if (std::isalnum(static_cast<unsigned char>(c)) == 0)
              continue;
            out.flags[std::string(1, c)] = true;
          }
          continue;
        }

        // Positional
        out.positionals.push_back(std::string(tok));
      }

      return out;
    }

  private:
    std::string program_ = "cli";
    std::string version_;
    std::string summary_;

    bool json_output_ = false;

    std::unordered_map<std::string, Command> commands_;
  };

} // namespace vix::cli_app

#endif // VIX_CLI_APP_CLI_APP_HPP
