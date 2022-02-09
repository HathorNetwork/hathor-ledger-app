[hathor-logo]: doc/images/hathor-logo.png (Hathor)
[website]: https://hathor.network


# Ledger Hathor Application

[![Hathor][hathor-logo]][website]

This is the official Hathor app for Ledger Nano S and X.

## Building

See [BUILD.md](doc/BUILD.md)

## Documentation

High level documentation such as [APDU](doc/APDU.md), [commands](doc/COMMANDS.md) and [transaction serialization](doc/TRANSACTION.md) are included in developer documentation which can be generated with [doxygen](https://www.doxygen.nl)

## Tests & Continuous Integration

The flow processed in [GitHub Actions](https://github.com/features/actions) is the following:

- Code formatting with [clang-format](http://clang.llvm.org/docs/ClangFormat.html)
- Python integration tests formatting with isort, flake8 and black
- Compilation of the application for Ledger Nano S in [ledger-app-builder](https://github.com/LedgerHQ/ledger-app-builder)
- Unit tests of C functions with [cmocka](https://cmocka.org/) (see [unit-tests/](unit-tests/))
- End-to-end tests with [Speculos](https://github.com/LedgerHQ/speculos) emulator (see [tests/](tests/))
- Code coverage with [gcov](https://gcc.gnu.org/onlinedocs/gcc/Gcov.html)/[lcov](http://ltp.sourceforge.net/coverage/lcov.php) and upload to [codecov.io](https://about.codecov.io)

It outputs 4 artifacts:

- `hathor-app-debug` within output files of the compilation process in debug mode
- `speculos-log` within APDU command/response when executing end-to-end tests
- `code-coverage` within HTML details of code coverage

# Contact Us

You can email us at [contact@hathor.network](mailto:contact@hathor.network) or join our [Discord](https://discord.gg/aWukZZX) and [Telegram](https://t.me/HathorOfficial).
