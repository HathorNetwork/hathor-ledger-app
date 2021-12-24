from pathlib import Path

import pytest

from app_client.automation import CommandAutomation, FakeAutomation
from app_client.cmd import Command
from app_client.transport import TransportAPI


def pytest_addoption(parser):
    parser.addoption("--hid", action="store_true")
    parser.addoption("--headless", action="store_true")
    parser.addoption(
        "--url", help="Speculos API endpoint", default="http://localhost:5000/"
    )


@pytest.fixture(scope="module")
def sw_h_path():
    # path with tests
    conftest_folder_path: Path = Path(__file__).parent
    # sw.h should be in src/sw.h
    sw_h_path = conftest_folder_path.parent / "src" / "sw.h"

    if not sw_h_path.is_file():
        raise FileNotFoundError(f"Can't find sw.h: '{sw_h_path}'")

    return sw_h_path


@pytest.fixture(scope="session")
def server(pytestconfig):
    return pytestconfig.getoption("url")


@pytest.fixture(scope="session")
def hid(pytestconfig):
    return pytestconfig.getoption("hid")


@pytest.fixture(scope="session")
def headless(pytestconfig):
    return pytestconfig.getoption("headless")


@pytest.fixture(scope="session", autouse=True)
def automation(headless, server):
    if headless:
        ca = CommandAutomation(server)
    else:
        ca = FakeAutomation()
    ca.set_accept_all()
    yield ca
    ca.close()


@pytest.fixture(scope="session")
def transport(server):
    transport = TransportAPI(server)
    yield transport
    transport.close()


@pytest.fixture(scope="session")
def cmd(transport):
    command = Command(transport=transport, debug=True)

    yield command
