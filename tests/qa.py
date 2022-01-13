import pytest

from faker import Faker

from app_client.exception import InvalidSignatureError
from app_client.transaction import Transaction, TxInput, TxOutput
from app_client.token import Token

from utils import fake_tx, fake_token, fake_script

fake = Faker()

def test_qa_version(cmd):
    print("QA::version")
    assert cmd.get_version() == (b'HTR', 1, 0, 0)


def test_qa_address(cmd):
    path = "m/44'/280'/0'/0/10"
    print("QA::address::path:", path)
    cmd.get_address(bip32_path=path)


def test_qa_xpub(cmd):
    path = "m/44'/280'/0'/0"
    print("QA::xpub::path:", path)
    pub_key, chain_code, fingerprint = cmd.get_xpub(bip32_path=path)  # type: bytes, bytes, bytes


def test_qa_sign_tx(cmd):
    # sign_tx
    tx = fake_tx(tokens=[])
    print("QA::sign_tx::tx:", str(tx))
    cmd.sign_tx(tx, has_change=False)


def test_qa_sign_token(cmd):
    # sign_token
    token = fake_token()
    print("QA::sign_token::token:", str(token))
    sig = cmd.sign_token_data(token)
    # verify signature
    cmd.verify_token_signature(token, sig)


def test_qa_reset_token_signatures(cmd):
    # reset
    cmd.reset_token_signatures()


def test_qa_sign_tx_with_token(cmd):
    path = "m/44'/280'/0'/0/10"
    # sign_token
    token = fake_token()
    inputs = [
            TxInput(fake.sha256(True), 1, path),
            TxInput(fake.sha256(True), 0, path),
            ]
    outputs = [
            TxOutput(fake.pyint(1), fake_script(), 1),
            TxOutput(fake.pyint(1), fake_script()),
            ]
    tx = Transaction(1, [token.uid], inputs, outputs)
    print("QA::sign_tx_with_token::token:", str(token))
    print("QA::sign_tx_with_token::tx:", str(tx))
    sig = cmd.sign_token_data(token)
    # send_token_data
    cmd.send_token_data(token, sig)
    # sign_tx with token
    cmd.sign_tx(tx, has_change=False)
