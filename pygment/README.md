# pygment2 - Modern Exodus C++ Lexer for Pygments

This is a modernized version of the original `exodus/pygment` directory.

## Changes from the original

- Replaced deprecated `setup.py install` with a proper `pyproject.toml`
- Installation now uses `pip install .` (the recommended way)
- No more SetuptoolsDeprecationWarning / EasyInstallDeprecationWarning
- Removed `setup.py` (no longer needed)

## Files

- `exodus_cpp.py` - The custom Pygments lexer for Exodus C++
- `pyproject.toml` - Modern Python packaging configuration
- `install.sh` - Updated installer script
- `test.sh` / `test.cpp` / `test_exp.htm` - Basic test for the lexer

## Installation

```bash
cd pygment2
./install.sh
```

This will uninstall any previous version and install the lexer using pip.

After installation, you can use it with:

```bash
pygmentize -l exoduscpp -f html -o output.html yourfile.cpp
```

## Development

For development (editable install):

```bash
pip install -e . --break-system-packages
```

## Notes

- The lexer is registered under the name `exoduscpp` (and alias `exodus_cpp`).
- It extends the standard CppLexer with many Exodus-specific keywords and functions.
- Tested on Ubuntu 24.04 with Python 3.12 and Pygments.
