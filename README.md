# Ledger Hathor Application

Hathor application

## Prerequisite

Be sure to have your environment correctly set up (see [Getting Started](https://ledger.readthedocs.io/en/latest/userspace/introduction.html)) and [ledgerblue](https://pypi.org/project/ledgerblue/) and installed.

## Compilation

To compile with your own pre-configured dev-env:
```
make DEBUG=1  # compile optionally with PRINTF
make NETWORK=testnet  # compile app for testnet
make load     # load the app on the Nano using ledgerblue
```

To compile with the builder container:
- build:
    `make -f .dev.Makefile build`
- clean:
    `make -f .dev.Makefile clean`

Build linter with `make -f .dev.Makefile lint-build` and you can use the same linter (and linter configurations) as the CI using the commands:
- `make -f .dev.Makefile lint`
- `make -f .dev.Makefile lint-fix`

To start a fully configured dev-env inside docker run `make -f .dev.Makefile builder`.
Now you can use the normal Makefile commands (including loading on the Ledger Nano S).

## Documentation

High level documentation such as [APDU](doc/APDU.md), [commands](doc/COMMANDS.md) and [transaction serialization](doc/TRANSACTION.md) are included in developer documentation which can be generated with [doxygen](https://www.doxygen.nl)

```
doxygen .doxygen/Doxyfile
```

the process outputs HTML and LaTeX documentations in `doc/html` and `doc/latex` folders.

## Tests & Continuous Integration

The flow processed in [GitHub Actions](https://github.com/features/actions) is the following:

- Code formatting with [clang-format](http://clang.llvm.org/docs/ClangFormat.html)
- Compilation of the application for Ledger Nano S in [ledger-app-builder](https://github.com/LedgerHQ/ledger-app-builder)
- Unit tests of C functions with [cmocka](https://cmocka.org/) (see [unit-tests/](unit-tests/))
- End-to-end tests with [Speculos](https://github.com/LedgerHQ/speculos) emulator (see [tests/](tests/))
- Code coverage with [gcov](https://gcc.gnu.org/onlinedocs/gcc/Gcov.html)/[lcov](http://ltp.sourceforge.net/coverage/lcov.php) and upload to [codecov.io](https://about.codecov.io)
- Documentation generation with [doxygen](https://www.doxygen.nl)

It outputs 4 artifacts:

- `hathor-app-debug` within output files of the compilation process in debug mode
- `speculos-log` within APDU command/response when executing end-to-end tests
- `code-coverage` within HTML details of code coverage
- `documentation` within HTML auto-generated documentation
