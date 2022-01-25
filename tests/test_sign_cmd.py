from utils import fake_tx
from app_client.transaction import TxInput

import hathorlib

from faker import Faker

fake = Faker()


def test_sign_tx(cmd, public_key_bytes):
    inputs = [
        TxInput(fake.sha256(True), fake.pyint(0, 255), "m/44'/280'/0'/0/{}".format(x))
        for x in range(10)
    ]
    tx = fake_tx(inputs=inputs, tokens=[])
    signatures = cmd.sign_tx(tx)
    print("sighash_all = {}".format(tx.serialize().hex()))

    for index, signature in enumerate(signatures):
        print("verifying signature {}".format(signature.hex()))
        print(hathorlib.utils.get_address_b58_from_public_key_bytes(public_key_bytes[index]))
        tx.verify_signature(signature, public_key_bytes[index])
