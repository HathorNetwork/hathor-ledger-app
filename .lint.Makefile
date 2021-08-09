# Linter functions

EXCLUDES =  -e './dev-env/*'
EXCLUDES += -e './.git/*'

DOCKER_ARGS = -it --rm --workdir /src -v $(PWD):/src

DOCKER_IMAGE = clang-format-lint
LINTER_ARGS = --clang-format-executable /clang-format/clang-format11 --style=file -r ${EXCLUDES}

.PHONY: lint
lint:
	docker run ${DOCKER_ARGS} ${DOCKER_IMAGE} ${LINTER_ARGS} .

.PHONY: fix
fix:
	docker run ${DOCKER_ARGS} ${DOCKER_IMAGE} ${LINTER_ARGS} -i 1 .

.PHONY: lint-build
build:
	docker build -t ${DOCKER_IMAGE} github.com/DoozyX/clang-format-lint-action
