"""
Speculos default seed:
'glory promote mansion idle axis finger extra february uncover one trip resource'
'lawn turtle enact monster seven myth punch hobby comfort wild raise skin'

xprv at m/44'/280'/0'/0 :
htpr57ja7bTZnPuNnvUCxQF66SxQgmtrQEQTjYaYVM9SYthQMn1Jo9SqHGBhmuVeFHf7oD31TjVRSt98S2x5nkUgLS1rMS587RBkScgSyJvmywr
xpub at m/44'/280'/0'/0 :
xpub6EM8JzFyzrR5ogY1v51P5BNqw3vxsuA7Vm8Cxmix7aRnwdGpazCXm3gnf3qw1uoDPrLeFsK7Tjs32Zb1ceocLUzfyyodxSjzmNanonJzEC7

Addresses: m/44'/280'/0'/0/0-9
| index | address |
| --- | --- | --- |
| 0 | HJYAnivCTdRtxcgeJW8pKk1y7przypne3C |
| 1 | HAgKJhndq6LCd3qd5tRZC7ixFAk5AyXH2J |
| 2 | H7Db39doQWtPxPx28e47xGt2hRWekhPsrF |
| 3 | HA8LYkjuoM5pYJMRowDRyxF7ynGrcZAXSJ |
| 4 | HRW3GHh7ocfWkYFtn8ek5aNatEKCuZNz9X |
| 5 | HGN15ort55joYigANsSgc25bihEcWhT9AC |
| 6 | HQWUGHSmhDRYFTihwRXV7KYtjb4u84LJgZ |
| 7 | HBdTnYddrfDAw4mSXWystLVRdQgiDoikrH |
| 8 | HTwehQ1oZvxwAxVB9ET8tesQnNWR1BnfKJ |
| 9 | HR3vvY4AktDo9EWBnvsqqfKeWttRWc1wUW |
"""

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


@pytest.fixture(scope="session")
def public_key_bytes():
    """Public key bytes for paths m/44'/280'/0'/0/0-9 for test seed.
    Generated with bitcore-lib from the speculos xpub"""

    return [
        bytes.fromhex(
            "02962e6c4afe696afa985363fb53bee05cd22463b1cb79bde72ffb8fbd029c6e7d"
        ),
        bytes.fromhex(
            "03f29ab54cf2a6311b55a6cd80b00eb605e7fdd9a2ee4042ca223c8e5bd8f8dc8d"
        ),
        bytes.fromhex(
            "02238651df8a17ead4d7087869ab5b6e7b161160a872b6b1d5fe4403ae8a7d04a7"
        ),
        bytes.fromhex(
            "03e220a2442d3c4430592593d570df94cf0575f25612489f08eda622647f2e53bb"
        ),
        bytes.fromhex(
            "029849e864f80a04e55a86b9341c4781479397c474cdf14aebdc76ee15f8019684"
        ),
        bytes.fromhex(
            "03e136df9cd71863adb6ad06e0083022a973beb86a1f972eb54439c4db9bd53e46"
        ),
        bytes.fromhex(
            "02929bb096be77b70cb8aa36ec3d516e42e6f26513342207752ac4ebd4d5d8d798"
        ),
        bytes.fromhex(
            "0397d4aa74896248601c9f6e525d0463b693bac7ed8db0db56eb25cf0271f717e3"
        ),
        bytes.fromhex(
            "0336c5a922be2e97661943a9f6c0580e012c5c86f27c3c0a9c4a8b2f5e24fb1aee"
        ),
        bytes.fromhex(
            "03cc7f5488dd61522d75c69035c72431b12e869be3ffc4da13c86cfe8a672d06d2"
        ),
    ]
