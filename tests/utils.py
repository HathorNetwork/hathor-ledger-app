from typing import Optional

import hathorlib
from faker import Faker

from app_client.token import Token
from app_client.transaction import Transaction, TxInput, TxOutput

fake = Faker()


def fake_path() -> str:
    return "m/44'/280'/0'/0/{}".format(fake.pyint(0, 255))


def fake_script() -> bytes:
    """Output P2PKH script for a random address"""
    # return b"".join([b"\x76\xa9\x14", pkh, b"\x88\xac"])
    return hathorlib.scripts.P2PKH.create_output_script(fake.binary(length=25))


def fake_input() -> TxInput:
    return TxInput(fake.sha256(True), fake.pyint(0, 255), fake_path())


def fake_output() -> TxOutput:
    return TxOutput(fake.pyint(1), fake_script())


def fake_tx(
    inputs: Optional[TxInput] = None,
    outputs: Optional[TxOutput] = None,
    tokens: Optional[bytes] = None,
) -> Transaction:
    tx_inps = inputs or [fake_input() for _ in range(fake.pyint(1, 5))]
    tx_outps = outputs or [fake_output() for _ in range(fake.pyint(1, 5))]
    tkns = (
        [fake.sha256(True) for _ in range(fake.pyint(1, 10))]
        if tokens is None
        else tokens
    )

    return Transaction(1, tkns, tx_inps, tx_outps)


def fake_token():
    c = fake.cryptocurrency()
    return Token(1, c[0], c[1], fake.sha256())
