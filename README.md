# FixEngine

A C++20 FIX (Financial Information eXchange) engine focused on FIX 4.4.

This repository currently builds a reusable library (`fix_engine`) plus a GoogleTest suite (`fix_tests`). The engine is under active development; several protocol features and hardening items are still in-progress.

## Goals

- Provide a scalable FIX session runtime (multiple sessions, asynchronous IO).
- Provide a pragmatic FIX 4.4 message pipeline: parse → validate → dispatch → serialize.
- Keep the core library embeddable (use it from your own application binary).

## Non-goals (currently)

- A production-complete FIX implementation.
- A polished CLI/operator console.
- Full FIX versioning (the code defaults to FIX 4.4).

## What’s implemented

The project has working building blocks across the stack (transport/session/message/validation/logging). For a detailed map of modules to responsibilities, see [docs/architecture/system_map.md](docs/architecture/system_map.md).

Highlights:

- Asynchronous transport built on Boost.Asio (`Fix::Reactor`, `Fix::AsioConnection`).
- Session runtime + manager (`Fix::Session`, `Fix::SessionManager`) with basic admin handlers.
- Parser/serializer and message model (`Fix::Parser`, `Fix::Serializer`, `Fix::Message`, `Fix::GenericMessage`).
- Schema registry + repeating group support (see `include/fix/schema/*`).
- Validator with error enums (`Fix::Validator`).
- JSON logging with a ring-buffer core and file sink (writes under `./logs/`).

Project status and gaps are tracked in:

- [docs/completeness.md](docs/completeness.md)
- [docs/task.md](docs/task.md)
- [docs/quirks.md](docs/quirks.md)

## Requirements

- Linux (development environment assumed)
- CMake ≥ 3.15
- A C++20 compiler (GCC 11+ or Clang 14+ recommended)
- Boost (headers + `Boost::system`)
- Internet access during configure (GoogleTest is fetched via `FetchContent`)

## Build

From the repo root:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

Notes:

- The primary library target is `fix_engine`.
- The test executable target is `fix_tests`.
- A `compile_commands.json` is produced in the build directory; you can symlink/copy it to the repo root if your tooling expects that.

## Run tests

Using CTest:

```bash
ctest --test-dir build --output-on-failure
```

Or run the test binary directly:

```bash
./build/tests/fix_tests
```

## Using the library (embedding)

This repo does not currently ship a standalone “engine executable”; you embed the library into your own program.

### CMake integration

In your project:

```cmake
add_subdirectory(path/to/FixEngine)
target_link_libraries(your_app PRIVATE fix_engine)
```

Include headers from `include/` (the library exports that include directory):

```cpp
#include <fix/core/SessionManager.hpp>
```

### Minimal runtime skeleton

The following is a minimal pattern for creating sessions and running the IO reactor (adapted from [tests/server.cpp](tests/server.cpp)):

```cpp
#include <fix/core/Reactor.hpp>
#include <fix/core/SessionManager.hpp>
#include <fix/core/definitions.hpp>
#include <fix/core/Application.hpp>

int main() {
	Fix::Reactor reactor{};
	Fix::Application app{};

	Fix::AsioConnectionFactory conn_factory{reactor.context()};
	Fix::AsioTimerFactory timer_factory{reactor.context()};

	Fix::SessionManager session_manager{
			app,
			conn_factory,
			timer_factory,
			reactor.context()
	};

	Fix::SessionCreationConfig cfg{
			.role = Fix::Role::INITIATOR,
			.conn_config = Fix::ConnectionConfig{"127.0.0.1", 5001, Fix::Role::INITIATOR, 1},
			.params = Fix::SessionParameters{}
	};

	std::vector<Fix::SessionCreationConfig> configs{cfg};
	session_manager.create_all(configs);

	reactor.run(2);      // run io_context on N threads
	// ... keep your process alive here ...

	session_manager.stop_all();
	reactor.stop();
	reactor.wait();
}
```

Configuration today is primarily done through the structs in [include/fix/core/definitions.hpp](include/fix/core/definitions.hpp) (for example `Fix::SessionParameters`, `Fix::ConnectionConfig`). `Config.hpp` exists but is currently empty.

## Logging

The engine logs JSON to files under `./logs/` (relative to the working directory of your process).

- Core log: `logs/<engine_run_id>/core_log.json`
- Per-session logs: `logs/<engine_run_id>/sessions/<readable_session_id>.json`

See the implementation in [src/LogFileSink.cpp](src/LogFileSink.cpp) and the public interface in [include/fix/log/LogFileSink.hpp](include/fix/log/LogFileSink.hpp).

## Repository layout

- [include/](include/) – Public headers (engine API)
- [src/](src/) – Library implementation
- [tests/](tests/) – GoogleTest suite and some ad-hoc executables (not all are wired into CMake)
- [docs/](docs/) – Design notes, architecture map, completeness tracking
- [scripts/](scripts/) – Small utilities used during development

## Troubleshooting

- **Boost not found**: install Boost development packages (must include `Boost::system`) or point CMake at your Boost install (e.g. `-DBOOST_ROOT=...`).
- **No executable to run**: only `fix_tests` is built by default; create your own application and link against `fix_engine`.
- **Offline builds**: initial configure requires fetching GoogleTest (or you’ll need to vendor it / replace `FetchContent`).

## Contributing

This project is evolving quickly. If you’re changing protocol behavior, add/extend a test in [tests/](tests/) and update the relevant design notes in [docs/](docs/).
