# cmd_set_colors

Alternative (c99) ColorTool for CMD.exe.

A Windows console color utility that sets CMD console colors using Windows API. This is essentially a "poor man's version" of Microsoft ColorTool.

## Overview

`cmd_set_colors` loads color palette information from an INI file and applies it to the Windows Console (CMD). It uses the Windows API functions `GetStdHandle()`, `GetConsoleScreenBufferInfoEx()`, and `SetConsoleScreenBufferInfoEx()` to manipulate console colors.

## Features

- Sets all 16 DOS-style console colors
- Loads color definitions from INI files
- Default color scheme: Dracula Classic (Dark)
- Command-line options for help and version info
- Configurable color palette via external configuration

## Compilation

### Using GCC:

```bash
gcc ini.c cmd_set_colors.c -o cmd_set_colors
```

### Using MSVC:

```bash
cl ini.c cmd_set_colors.c
```

## Usage

```bash
cmd_set_colors [ini_filename]
```

### Command-line Options

| Option | Description |
|--------|-------------|
| `-h`, `--help` | Display usage information |
| `--version` | Display version number |
| `ini_filename` | Path to INI file with color definitions (default: `colors.ini`) |

### Examples

```bash
# Use default colors.ini file
cmd_set_colors

# Use custom color configuration
cmd_set_colors my_colors.ini
cmd_set_colors /full/path/my_colors.ini

# Show version
cmd_set_colors --version

# Show help
cmd_set_colors --help
```

## INI File Format

Subset, based on the same format ColorTool uses, samples:

   * https://github.com/microsoft/terminal/tree/main/src/tools/ColorTool/schemes (NOTE only .ini files supported)
   * https://github.com/clach04/base24_colortool/tree/main/themes/base24

Color definitions are stored in an INI file with a `[table]` section. Each color is defined as `COLOR_NAME=R,G,B`:

```ini
[table]
DARK_BLACK=40,42,54
DARK_BLUE=189,147,249
DARK_GREEN=80,250,123
DARK_CYAN=139,233,253
DARK_RED=255,85,85
DARK_MAGENTA=255,121,198
DARK_YELLOW=241,250,140
DARK_WHITE=248,248,242
BRIGHT_BLACK=98,114,164
BRIGHT_BLUE=214,172,255
BRIGHT_GREEN=105,255,148
BRIGHT_CYAN=164,255,255
BRIGHT_RED=255,110,110
BRIGHT_MAGENTA=255,146,223
BRIGHT_YELLOW=255,255,165
BRIGHT_WHITE=255,255,255
```

## Color Indices

The tool supports the standard 16-color DOS palette:

| Index | Name | Description |
|-------|------|-------------|
| 0 | DARK_BLACK | Black |
| 1 | DARK_BLUE | Dark Blue |
| 2 | DARK_GREEN | Dark Green |
| 3 | DARK_CYAN | Dark Cyan |
| 4 | DARK_RED | Dark Red |
| 5 | DARK_MAGENTA | Dark Magenta |
| 6 | DARK_YELLOW | Dark Yellow |
| 7 | DARK_WHITE | Dark White (Gray) |
| 8 | BRIGHT_BLACK | Bright Black (Gray) |
| 9 | BRIGHT_BLUE | Bright Blue |
| 10 | BRIGHT_GREEN | Bright Green |
| 11 | BRIGHT_CYAN | Bright Cyan |
| 12 | BRIGHT_RED | Bright Red |
| 13 | BRIGHT_MAGENTA | Bright Magenta |
| 14 | BRIGHT_YELLOW | Bright Yellow |
| 15 | BRIGHT_WHITE | Bright White |

## Current Status

### Implemented

- Hard-coded default colors (Dracula Classic theme)
- INI file color loading
- Works well enough for my day-to-day usage

## License

TBD
