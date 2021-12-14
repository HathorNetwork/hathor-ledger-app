from typing import Optional

from app_client.transaction import TxInput, TxOutput, Transaction

from faker import Faker

fake = Faker()


def fake_path() -> str:
    return "m/44'/280'/0'/0/{}".format(fake.pyint(0, 255))


def fake_script() -> bytes:
    pkh = fake.binary(length=20)
    # P2PKH script with random pubkey hash
    return b''.join([
        b'\x76\xa9\x14',
        pkh,
        b'\x88\xac'
    ])


def fake_input() -> TxInput:
    return TxInput(fake.sha256(True), fake.pyint(0, 255), fake_path())


def fake_output() -> TxOutput:
    return TxOutput(fake.pyint(1), fake_script())


def fake_tx(
        inputs: Optional[TxInput] = None,
        outputs: Optional[TxOutput] = None,
        tokens: Optional[bytes] = None,
        ) -> Transaction:
    tx_inps = inputs or [fake_input() for _ in range(fake.pyint(1, 10))]
    tx_outps = outputs or [fake_output() for _ in range(fake.pyint(1, 10))]
    tkns = tokens or [fake.sha256(True) for _ in range(fake.pyint(1, 10))]

    return Transaction(1, tkns, tx_inps, tx_outps)
