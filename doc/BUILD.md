# Build

The `Makefile` uses the Nano SDK to build the app but it requires a fully configured dev-env.

We use a configured dev-env inside Docker so you don't have to install the dev-env on your machine.
All dev commands are configured on a separate `.dev.Makefile`.

## Dev-env

This command will start a docker container with the configured dev-env where you can run any commands on the `Makefile` or the SDK, like [loading](https://developers.ledger.com/docs/nano-app/load/) the app on Nano S.


```bash
make -f .dev.Makefile builder
```

## Compilation

```bash
make -f .dev.Makefile build
```

You can add the flag `DEBUG=1` at the end to compile on debug mode.

To remove all files generated from the build process run:
```bash
make -f .dev.Makefile clean
```

## Linter

Build linter with `make -f .dev.Makefile lint-build` and you can use the same linter (and linter configurations) as the CI using the commands:

- `make -f .dev.Makefile lint`
- `make -f .dev.Makefile lint-fix`


## Nano S simulator

Start a [Speculos]() simulator with `make -f .dev.Makefile speculos`
It will run the last compiled app and start a service on `http://localhost:5000`.

## Simulator for devs

After building and running the simulator, go to `tests/` and you can run a fully automated test with `make test` which uses the command `pytest --headless`.
The `--headless` flag automates the interactions, but to disable this simply run without the flag to interact yourself with the simulator.

You can run any test script to simulate the interactions you need, an example of this is the `qa.py` script which has all command interactions.
To run it use `pytest qa.py` or `pytest --headless qa.py`

Obs: Once the tests with `--headless` run they will configure some automations rules which will be kept valid after tests, to disable them simply restart the simulator.
