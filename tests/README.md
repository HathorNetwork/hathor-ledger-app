# End-to-end tests

The tests are implemented in python and need a Speculos emulator to run.
Python dependencies are listed in [pyproject.toml](pyproject.toml), install them using [poetry](https://github.com/python-poetry/poetry)

Some basic commands are defined on the Makefile.

```
poetry install
# or
make install
```

### Run with Speculos

First you need to build and run the application with speculos.

On the root directory of the project run:

```bash
make -f .dev.Makefile build DEBUG=1
```

```bash
make -f .dev.Makefile speculos
```

### Running tests

In the `tests` folder run:

```bash
make test
# or
poetry run pytest
```

### Running qa

You need to open the speculos web ui, the actual address will be shown in the speculos stdout.

In the `tests` folder run:

```bash
make qa
# or
poetry run pytest qa.py
```

Obs: you need to restart speculos if the tests ran before since they configure automation rules which need to be turned off during qa.
