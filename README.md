# JPipe

[![CodeQL](https://github.com/scokmen/jpipe/actions/workflows/github-code-scanning/codeql/badge.svg?branch=main)](https://github.com/scokmen/jpipe/actions/workflows/github-code-scanning/codeql)
[![Coverage Status](https://coveralls.io/repos/github/scokmen/jpipe/badge.svg?branch=main)](https://coveralls.io/github/scokmen/jpipe?branch=main)

A lightweight, high-performance pipe-to-JSON logger written in C11. 
Capture stdin streams with extensible metadata injection, configurable buffering, and efficient multiplexing across macOS and Linux.

## Features

- **Zero-Copy**: Platform-specific multiplexing (kqueue on macOS, epoll on Linux) with lock-free queue patterns.
- **Extensible Metadata**: Inject custom JSON fields at capture time with type inference.
- **Configurable Throughput**: Tune chunk size, buffer capacity, and overflow policies for your workload.
- **Memory-Safe**: Error handling and address/thread sanitizer support.
- **Production-Ready**: Full CI/CD pipeline, high test coverage, and performance benchmarks.
- **CLI-First**: Simple, powerful command-line interface with help and version commands.

## Installation

### Prerequisites

- CMake 3.20 or later
- C11 compiler (GCC 9+ or Clang 10+)
- POSIX-compliant OS (macOS 10.7+, Linux 2.6.32+)
- Threads support (pthread)
- GNU Make

### Building

```bash
git clone https://github.com/scokmen/jpipe.git
cd jpipe

# Debug build (with tests)
make debug

# Release build (optimized)
make release

# Release with debug symbols
make relwithdebinfo
```

### Installing

TBD.

## Usage

### Basic Capture

```bash
cat data.log | jpipe run -o /tmp/output
```

Reads stdin and writes JSON-encoded lines to `/tmp/output/jpipe_<timestamp>_<pid>.json`.

### Command-Line Options

```bash
jpipe run [options]
```

| Option             | Short  | Default  | Description                                                                          |
|--------------------|--------|----------|--------------------------------------------------------------------------------------|
| `--chunk-size`     | `-c`   | 16kb     | Chunk size (1kb–128kb). Larger chunks reduce syscalls; smaller chunks lower latency. |
| `--buffer-size`    | `-b`   | 64       | Queue capacity (1–1024). Number of pending chunks before applying overflow policy.   |
| `--policy`         | `-p`   | wait     | Overflow policy: `wait` (block until queue drains) or `drop` (discard messages).     |
| `--output`         | `-o`   | `.`      | Output directory for JSON files.                                                     |
| `--field`          | `-f`   | —        | Custom JSON field (`key=value`). Repeatable; supports type inference.                |
| `--dry-run`        | `-n`   | —        | Parse and validate inputs without writing.                                           |
| `--help`           | `-h`   | —        | Show help message.                                                                   |

### Global Options

```bash
jpipe [--quiet | -q] [--no-color | -C] <command> [options]
```

| Option             | Description                                              |
|--------------------|----------------------------------------------------------|
| `help`             | Show global help message.                                |
| `version`          | Show version.                                            |
| `-q, --quiet`      | Suppress non-critical logs.                              |
| `-C, --no-color`   | Disable colored output.                                  |
| `NO_COLOR` env var | Respects the [NO_COLOR](https://no-color.org/) standard. |

### Field Injection

Use `-f key=value` to add custom fields to every JSON object:

```bash
cat logs.txt | jpipe run -f "app=myapp" -f "env=prod" -f "timeout=30" -f "active=true"
```

**Value Type Inference:**

| Input          | Type            | JSON Output       |
|----------------|-----------------|-------------------|
| `count=42`     | Number          | `"count": 42`     |
| `rate=1.5e-3`  | Number (float)  | `"rate": 0.0015`  |
| `enabled=true` | Boolean         | `"enabled": true` |
| `text=hello`   | String          | `"text": "hello"` |
| `forced="123"` | String (forced) | `"forced": "123"` |

Key constraints:
- Must contain only: `a-z`, `A-Z`, `0-9`, `_`, `-`
- Maximum 64 characters

### Examples

#### High-Throughput Capture
```bash
tail -f /var/log/syslog | jpipe run -c 64kb -b 128 -o /var/log/json_archive
```

#### Drop Overflow Mode
```bash
heavy_stream | jpipe run -p drop -b 32 -o /tmp/logs
```

#### Metadata Enrichment
```bash
./app | jpipe run \
  -f "container=my_service" \
  -f "version=1.2.3" \
  -f "deployment=prod" \
  -o /var/log/app_logs/
```

#### Dry Run (Validation)
```bash
jpipe run -n -f "bad key!=value"  # Validates without writing
```

## Output Format

Each output file contains newline-delimited JSON (NDJSON), one JSON object per line:

```
{"line":"2025-04-21T10:30:45 INFO Starting service","app":"myapp","env":"prod"}
{"line":"2025-04-21T10:30:46 INFO Worker ready","app":"myapp","env":"prod"}
{"line":"2025-04-21T10:30:47 ERROR Connection timeout","app":"myapp","env":"prod"}
```

Output filenames follow the pattern: `jpipe_<start_timestamp>_<pid>.json`

## Configuration

### Memory Tuning

Memory usage is approximately:

```
estimated_mb = (chunk_size_kb * buffer_size) / 1024
```

Example: `16kb * 64` = ~1 MB queue overhead

Thread stacks and OS overhead are additional; actual usage will be higher. Check resource estimate on startup:

```bash
jpipe run -c 32kb -b 128 -o /tmp  # Prints estimated memory on startup
```

### Overflow Policies

- **`wait`** (default): Block the reader thread if the queue fills. Safe but may slow input processing.
- **`drop`**: Discard incoming messages when the queue is full. Preserves forward progress but loses data.

Choose based on your priorities: data completeness vs. availability.

## Architecture

jpipe uses a producer-consumer architecture:

1. **Reader Thread**: Reads from stdin in configurable chunks. Multiplexed with kqueue (macOS) / epoll (Linux) for non-blocking I/O.
2. **Queue**: Lock-free ring buffer with condition variables. Chunk-sized blocks prevent fragmentation.
3. **Writer Thread**: Consumes blocks from the queue and writes JSON-encoded output files.
4. **Encoder**: Encodes each line to JSON with custom fields injected. Type inference for field values.

**Signal Handling**: SIGINT and SIGTERM trigger graceful shutdown. Active chunks are flushed to disk before exit.

## Development

### Building & Testing

```bash
# Run tests
make test

# Run tests with Address Sanitizer (memory safety)
make test-with-asan

# Run tests with Thread Sanitizer (concurrency)
make test-with-tsan

# Generate code coverage report
make coverage

# Format code
make format

# Static analysis
make check-and-inform    # Report all issues
make check-and-enforce   # Report critical issues only
```

### Benchmarking

```bash
# Prepare benchmarking data
make bench-prepare

# Run benchmarks
make bench-run
```

## Limitations & Known Issues

- **File Rotation**: Not yet implemented. Use external tools (logrotate, systemd-rotate) or custom scripts for file management.
- **SIGTERM on Empty Queue**: Consumer may block briefly if the queue empties during shutdown.

## Contributing

Contributions are welcome! Please:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/my-feature`)
3. Write tests for new functionality
4. Ensure all tests pass and code is sanitizer-clean
5. Open a pull request with a clear description

## License

MIT License — See LICENSE file for details.

## Credits

Inspired by high-performance logging infrastructure in modern cloud-native systems.

## Support

- **Issues**: [GitHub Issues](https://github.com/scokmen/jpipe/issues)
- **Build Help**: Run `make help` for all available build targets
