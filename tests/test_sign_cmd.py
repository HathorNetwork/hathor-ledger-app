import pytest
from faker import Faker
from hathorlib.scripts import P2PKH
from hathorlib.utils import get_address_from_public_key_hash, get_hash160

from app_client.exception import BadStateError
from app_client.transaction import ChangeInfo, TxInput, TxOutput
from utils import fake_tx

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
        tx.verify_signature(signature, public_key_bytes[index])


@pytest.mark.skip("speculos: hanging tests")
def test_sign_tx_change_old_protocol(cmd, public_key_bytes):
    outputs = [
        TxOutput(
            fake.pyint(1),
            P2PKH.create_output_script(
                get_address_from_public_key_hash(get_hash160(public_key_bytes[x]))
            ),
        )
        for x in range(5)
    ]
    change_index = fake.pyint(0, 5)
    change_list = [ChangeInfo(change_index, "m/44'/280'/0'/0/{}".format(change_index))]
    tx = fake_tx(outputs=outputs, tokens=[])
    try:
        cmd.sign_tx(tx, change_list=change_list, use_old_protocol=True)
    except BadStateError:
        pytest.skip("speculos automation failed, not the test")


@pytest.mark.skip("speculos: hanging tests")
def test_sign_tx_change_protocol_v1(cmd, public_key_bytes):
    outputs = [
        TxOutput(
            fake.pyint(1),
            P2PKH.create_output_script(
                get_address_from_public_key_hash(get_hash160(public_key_bytes[x]))
            ),
        )
        for x in range(5)
    ]
    change_indices = [fake.pyint(0, 5) for x in range(5)]
    change_list = [
        ChangeInfo(change_index, "m/44'/280'/0'/0/{}".format(change_index))
        for change_index in change_indices
    ]
    tx = fake_tx(outputs=outputs, tokens=[])
    try:
        cmd.sign_tx(tx, change_list=change_list, use_old_protocol=False)
    except BadStateError:
        pytest.skip("speculos automation failed, not the test")
