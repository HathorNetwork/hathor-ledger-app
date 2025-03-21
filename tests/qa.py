import pytest

from faker import Faker

from app_client.exception import InvalidSignatureError
from app_client.transaction import Transaction, TxInput, TxOutput
from app_client.token import Token

from utils import fake_tx, fake_token, fake_script

fake = Faker()

def test_qa_version(cmd):
    print("QA::version")
    assert cmd.get_version() == (b'HTR', 1, 1, 1)


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
    cmd.sign_tx(tx)


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


def test_qa_sign_tx_custom_tokens(cmd):
    path = "m/44'/280'/0'/0/10"
    token1 = fake_token()
    token2 = fake_token()
    inputs = [
        TxInput(fake.sha256(True), 1, path),
        TxInput(fake.sha256(True), 0, path),
    ]
    outputs = [
        TxOutput(fake.pyint(1), fake_script(), 1),
        TxOutput(fake.pyint(1), fake_script(), 2),
        TxOutput(fake.pyint(1), fake_script()),
    ]
    tx = fake_tx(inputs=inputs, outputs=outputs, tokens=[token1.uid, token2.uid])
    print("QA::sign_tx_custom_tokens::token1:", str(token1))
    print("QA::sign_tx_custom_tokens::token2:", str(token2))
    print("QA::sign_tx_custom_tokens::tx:", str(tx))

    print("QA::sign_tx_custom_tokens::sign::token1")
    sig1 = cmd.sign_token_data(token1)
    print("QA::sign_tx_custom_tokens::sign::token1")
    sig2 = cmd.sign_token_data(token2)

    print("QA::sign_tx_custom_tokens::send::token1")
    cmd.send_token_data(token1, sig1, 0)
    print("QA::sign_tx_custom_tokens::send::token2")
    cmd.send_token_data(token2, sig2, 1)

    print("QA::sign_tx_custom_tokens::sign::tx")
    signatures = cmd.sign_tx(tx)


def test_qa_sign_tx_with_authority(cmd):
    path = "m/44'/280'/0'/0/10"
    # sign_token
    token = fake_token()
    inputs = [
            TxInput(fake.sha256(True), 1, path),
            TxInput(fake.sha256(True), 0, path),
            ]
    outputs = [
            TxOutput(1, fake_script(), 1, True),
            TxOutput(2, fake_script(), 1, True),
            TxOutput(fake.pyint(1), fake_script()),
            ]
    tx = Transaction(0, 1, [token.uid], inputs, outputs)
    print("QA::sign_tx_with_authority::token:", str(token))
    print("QA::sign_tx_with_authority::tx:", str(tx))
    sig = cmd.sign_token_data(token)
    # send_token_data
    cmd.send_token_data(token, sig)
    # sign_tx with token
    cmd.sign_tx(tx)
