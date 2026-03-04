# cli_app

Command-line application scaffold for modern C++.

`cli_app` provides a minimal and deterministic foundation for building
production-grade CLI tools on top of `vix/app`.

It enables:

- command routing
- argument parsing
- flags and options
- structured output
- lifecycle management

Header-only. Lightweight. Explicit.

## Download

https://vixcpp.com/registry/pkg/vix/cli_app

## Why cli_app?

Many C++ command-line tools grow organically and quickly become difficult to maintain:

- ad-hoc argument parsing
- inconsistent command structure
- duplicated help logic
- mixed application lifecycle code
- fragile error handling

`cli_app` provides a simple and deterministic structure for CLI tools.

It standardizes:

- command registration
- argument parsing
- flag handling
- option handling
- output helpers
- program lifecycle

This allows building tools similar to:

- `git`
- `docker`
- `kubectl`
- `vix`

without heavy frameworks or external dependencies.

No macros.
No reflection.
No runtime framework.

Just a clean CLI foundation.

## Dependency

`cli_app` depends on:

- `vix/app`

Architecture layering:

```
vix/app
  ↑
cli_app
```

This ensures:

- minimal runtime overhead
- explicit program lifecycle
- deterministic CLI execution

Dependencies are installed automatically via Vix Registry.

## Installation

### Using Vix Registry

```bash
vix add @vix/cli_app
vix deps
```

### Manual

```bash
git clone https://github.com/vixcpp/cli_app.git
```

Add the `include/` directory to your project.

## Core concepts

### Commands

Commands are registered explicitly:

```cpp
app.add_command({
    "hello",
    "Say hello",
    "",
    [](const Args& args) -> int {
        std::cout << "hello\n";
        return 0;
    }
});
```

Execution:

```bash
tool hello
```

### Positional arguments

Commands can receive positional arguments:

```cpp
app.add_command({
    "greet",
    "Greeting command",
    "",
    [](const Args& args) -> int {

        if (!args.positionals.empty())
            std::cout << "Hello " << args.positionals[0] << "\n";

        return 0;
    }
});
```

Execution:

```bash
tool greet world
```

### Flags

Flags are boolean options:

```cpp
if (args.flags.count("v"))
{
    std::cout << "verbose mode\n";
}
```

Execution:

```bash
tool run -v
```

### Options

Options carry values:

```cpp
auto it = args.options.find("env");

if (it != args.options.end())
{
    std::cout << "environment: " << it->second << "\n";
}
```

Execution:

```bash
tool deploy --env=prod
```

or

```bash
tool deploy --env prod
```

## JSON output helpers

CLI tools sometimes need machine-readable output.

`cli_app` provides simple helpers:

```cpp
app.print_ok("operation successful");
```

JSON mode:

```bash
tool run --json
```

Output:

```json
{
  "ok": true,
  "message": "operation successful"
}
```

Error output:

```cpp
app.print_error("operation failed");
```

Result:

```json
{
  "ok": false,
  "error": "operation failed"
}
```

## Help system

`cli_app` automatically renders a help page.

Example:

```bash
tool --help
```

Output:

```text
Usage:
  tool <command> [options]

Commands:
  run      Run command
  status   Show status

Global options:
  -h, --help
  -V, --version
  --json
```

## Version

Programs can expose their version:

```cpp
app.set_version("1.0.0");
```

Execution:

```bash
tool --version
```

Output:

```text
tool 1.0.0
```

## Typical CLI structure

Typical program flow:

1. configure program metadata
2. register commands
3. run CLI dispatcher
4. execute command handler
5. return exit code

Example:

```cpp
CliApp app;

app.set_program("tool");
app.set_version("1.0");

app.add_command({ /* ... */ });

return app.run_cli(argc, argv);
```

## Complexity

| Operation        | Complexity |
|-----------------|------------|
| command lookup   | O(1) |
| argument parsing | O(n) |
| flag lookup      | O(1) |
| option lookup    | O(1) |

CLI workloads remain lightweight and deterministic.

## Design philosophy

`cli_app` focuses on:

- explicit command routing
- deterministic argument parsing
- minimal dependencies
- predictable CLI behavior
- embeddable CLI runtime

It does not attempt to replace:

- full shell frameworks
- terminal UI frameworks
- complex plugin systems

Those belong to higher-level tooling.

## Tests

Run:

```bash
vix build
vix test
```

Tests verify:

- command routing
- argument parsing
- flags and options
- JSON output helpers
- exit codes

## License

MIT License\
Copyright (c) Gaspard Kirira

