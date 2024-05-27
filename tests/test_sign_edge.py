import pytest
from faker import Faker
from hathorlib.scripts import P2PKH
from hathorlib.utils import get_address_from_public_key_hash, get_hash160

from app_client.exception import TxInvalidError
from app_client.transaction import ChangeInfo, TxInput, TxOutput
from utils import fake_path, fake_script, fake_tx

fake = Faker()


def test_last_chunk_only_change(cmd, public_key_bytes):
    """
    This test creates a transaction that will be sent on 2 chunks.
    The last chunk will have 1 partial and 2 complete outputs, all 3 are change
    The change list is [0, 2, 3, 4] because we need the last output of the
    first chunk (output index 1) to be confirmed manually, this would trigger
    the auto confirmation of the next chunk.
    This is described on issue #68
    https://github.com/HathorNetwork/hathor-ledger-app/issues/68
    """
    inputs = [
        TxInput(fake.sha256(True), fake.pyint(0, 255), fake_path()),
        TxInput(fake.sha256(True), fake.pyint(0, 255), fake_path()),
    ]
    outputs = [
        TxOutput(
            fake.pyint(1),
            P2PKH.create_output_script(
                get_address_from_public_key_hash(get_hash160(public_key_bytes[x]))
            ),
        )
        for x in range(5)
    ]
    change_indices = [0, 2, 3, 4]
    change_list = [
        ChangeInfo(change_index, "m/44'/280'/0'/0/{}".format(change_index))
        for change_index in change_indices
    ]
    print([str(x) for x in change_list])
    tx = fake_tx(inputs=inputs, outputs=outputs, tokens=[])
    print("sighash_all = {}".format(tx.serialize().hex()))
    print("signing tx {}".format(tx))
    cmd.sign_tx(tx, change_list=change_list, use_old_protocol=False)


def test_more_change_than_outputs(cmd, public_key_bytes):
    """
    This test creates a transaction with 3 outputs and 4 change outputs.
    The device is exptected to fail with TxInvalidError
    """
    change_indices = [0, 1, 2, 3]
    change_list = [
        ChangeInfo(change_index, "m/44'/280'/0'/0/{}".format(change_index))
        for change_index in change_indices
    ]
    outputs = [TxOutput(fake.pyint(1), fake_script()) for x in range(3)]
    print([str(x) for x in change_list])
    tx = fake_tx(outputs=outputs, tokens=[])
    print("sighash_all = {}".format(tx.serialize().hex()))
    print("signing tx {}".format(tx))
    with pytest.raises(TxInvalidError):
        cmd.sign_tx(tx, change_list=change_list, use_old_protocol=False)


def test_duplicate_change(cmd, public_key_bytes):
    """
    This test sends a change list with a duplicate entry
    The device is exptected to fail with TxInvalidError
    """
    change_indices = [1, 2, 1]
    change_list = [
        ChangeInfo(change_index, "m/44'/280'/0'/0/{}".format(change_index))
        for change_index in change_indices
    ]
    outputs = [TxOutput(fake.pyint(1), fake_script()) for x in range(3)]
    print([str(x) for x in change_list])
    tx = fake_tx(outputs=outputs, tokens=[])
    print("sighash_all = {}".format(tx.serialize().hex()))
    print("signing tx {}".format(tx))
    with pytest.raises(TxInvalidError):
        cmd.sign_tx(tx, change_list=change_list, use_old_protocol=False)
