# humpty

> _For when you need all the king's horses and all the king's men to get around filesize limits_

A small C++ utility to split large files into smaller chunks and rejoin them later.

## Features

- Streams file I/O (does not load full files into memory)
- Rejoins chunks using manifest
- Verifies per-chunk and whole-file checksums on join (optional to disable)

## Requirements

- xmake
- C++23 compiler

## Build

Build app only:

```bash
bash scripts/build.sh
```

Equivalent:

```bash
xmake build humpty
```

Run with:

```bash
xmake run humpty -- --help
```

## Test

Run tests:

```bash
bash scripts/test.sh
```

Equivalent:

```bash
xmake build humpty_tests
xmake run humpty_tests --case all
xmake run humpty_tests --case splitter
xmake run humpty_tests --case joiner
xmake run humpty_tests --case roundtrip
xmake run humpty_tests --case urandom
```

## CLI

```text
humpty split <input-file> [--out|-o <dir>] --chunk-size|-c <size>
humpty join <manifest-file> --output|-o <file> [--no-verify|-n]
humpty --help
humpty --version
```

### Chunk size values

- Raw bytes: `1048576`
- Suffixes: `K`, `M`, `G` (binary units, base 1024)
  - Example: `64K`, `8M`

### Split defaults

- If `--out/-o` is omitted, output dir defaults to:
  - `./<input-filename>-humpty`

## Quick Start

Split:

```bash
xmake run humpty -- split big-file.bin -c 64M
```

This creates:

- `./big-file.bin-humpty/big-file.bin.manifest`
- `./big-file.bin-humpty/big-file.bin.part0000`
- `./big-file.bin-humpty/big-file.bin.part0001`
- ...

Join:

```bash
xmake run humpty -- join ./big-file.bin-humpty/big-file.bin.manifest -o ./big-file-restored.bin
```

Skip verification (faster, less safe):

```bash
xmake run humpty -- join ./big-file.bin-humpty/big-file.bin.manifest -o ./big-file-restored.bin -n
```

## Manifest Format

Current manifest is line-based text:

- `version`
- `source_file`
- `source_size`
- `chunk_size`
- `source_checksum`
- `chunks`
- repeated `chunk` lines with:
  - index
  - offset
  - size
  - chunk filename
  - chunk checksum
