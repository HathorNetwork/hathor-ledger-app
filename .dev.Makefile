# Linter functions

EXCLUDES =  -e './dev-env/*'
EXCLUDES += -e './.git/*'

LINTER_IMAGE = clang-format-lint
LINTER_REPO = github.com/DoozyX/clang-format-lint-action
LINTER_ARGS = --clang-format-executable /clang-format/clang-format11 --style=file -r ${EXCLUDES}
DOCKER_LINTER_ARGS = -it --rm --workdir /src -v $(PWD):/src

BUILDER_TAG=latest
BUILDER_IMAGE = ghcr.io/ledgerhq/ledger-app-builder/ledger-app-builder:$(BUILDER_TAG)
DOCKER_BUILDER_ARGS = --rm -ti -v "$(realpath .):/app"

.PHONY: lint
lint:
	docker run ${DOCKER_LINTER_ARGS} ${LINTER_IMAGE} ${LINTER_ARGS} .

.PHONY: lint-fix
lint-fix:
	docker run ${DOCKER_LINTER_ARGS} ${LINTER_IMAGE} ${LINTER_ARGS} -i 1 .

.PHONY: lint-build
lint-build:
	docker build -t ${LINTER_IMAGE} ${LINTER_REPO}

.PHONY: clean
clean:
	$(info make clean)
	docker run ${DOCKER_BUILDER_ARGS} ${BUILDER_IMAGE} make clean

.PHONY: build
build: clean
	$(info make)
	docker run ${DOCKER_BUILDER_ARGS} ${BUILDER_IMAGE} make

.PHONY: build-nanox
build-nanox:
	$(info To build for Nano X run the command below inside the builder (`make builder`))
	@echo BOLOS_SDK=\$$NANOX_SDK make

.PHONY: builder
builder:
	sudo docker run ${DOCKER_BUILDER_ARGS} -v "/dev/bus/usb:/dev/bus/usb" --privileged ${BUILDER_IMAGE}
