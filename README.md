# Ledger Hathor Application

Hathor application

## Prerequisite

Be sure to have your environment correctly set up (see [Getting Started](https://ledger.readthedocs.io/en/latest/userspace/introduction.html)) and [ledgerblue](https://pypi.org/project/ledgerblue/) and installed.

## Dev commands

- build:
    `make -f .dev.Makefile build`
- build with `PRINTF`:
    `make -f .dev.Makefile build DEBUG=1`
- clean:
    `make -f .dev.Makefile clean`

Build linter with `make -f .dev.Makefile lint-build` and you can use the same linter (and linter configurations) as the CI using the commands:
- `make -f .dev.Makefile lint`
- `make -f .dev.Makefile lint-fix`

To start a fully configured dev-env inside docker run `make -f .dev.Makefile builder`.
Now you can use the normal Makefile commands (including loading on the Ledger Nano S).

After building you can start a fully functional Nano S simulator with `speculos`:
`make -f .dev.Makefile speculos`

The simulator can be viewed at `http://localhost:5000`.

## Simulator for devs

After building and running the simulator you can run a fully automated test with `make test` which uses the command `pytest --headless`.
The `--headless` flag automates the interactions, but to disable this simply run without the flag to interact yourself with the simulator.

You can run any test script to simulate the interactions you need, an example of this is the `qa.py` script which has all command interactions.
To run it use `pytest qa.py` or `pytest --headless qa.py`

Obs: Once the tests with `--headless` run they will configure some automations rules which will be kept valid after tests, to disable them simply restart the simulator.

## Documentation

High level documentation such as [APDU](doc/APDU.md), [commands](doc/COMMANDS.md) and [transaction serialization](doc/TRANSACTION.md) are included in developer documentation which can be generated with [doxygen](https://www.doxygen.nl)

```
doxygen .doxygen/Doxyfile
```

the process outputs HTML and LaTeX documentations in `doc/html` and `doc/latex` folders.

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
