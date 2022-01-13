py_sources = app_client test_* conftest.py utils.py


.PHONY: flake8
flake8:
	poetry run flake8 $(py_sources)

.PHONY: isort-check
isort-check:
	poetry run isort --ac --check-only $(py_sources)

.PHONY: black-check
black-check:
	poetry run black --check $(py_sources)

.PHONY: check
check: flake8 isort-check black-check

.PHONY: black
black:
	poetry run black $(py_sources)

.PHONY: isort
isort:
	poetry run isort --ac $(py_sources)

.PHONY: fmt
fmt: isort black

.PHONY: test
test:
	poetry run pytest --headless

.PHONY: qa
qa:
	poetry run pytest qa.py

.PHONY: install
install:
	poetry install
