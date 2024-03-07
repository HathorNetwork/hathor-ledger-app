# End-to-end tests

The tests are implemented in python and need a Speculos emulator to run.
Python dependencies are listed in [pyproject.toml](pyproject.toml), install them using [poetry](https://github.com/python-poetry/poetry)

Some basic commands are defined on the Makefile.

```
poetry install
# or
make install
```

### Launch with Speculos

First start your application with Speculos

```
./path/to/speculos.py /path/to/app.elf --ontop --sdk 2.0
```

then in the `tests` folder run

```
poetry run pytest
# or
make test
```
