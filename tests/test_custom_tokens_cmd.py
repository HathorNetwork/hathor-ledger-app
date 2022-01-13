import pytest
from faker import Faker

from app_client.exception import InvalidSignatureError
from utils import fake_token

fake = Faker()


def test_sign_token(cmd):
    token = fake_token()
    cmd.sign_token_data(token)


def test_send_token_data(cmd):
    token = fake_token()
    signature = cmd.sign_token_data(token)
    cmd.send_token_data(token, signature)


def test_send_token_data_list(cmd):
    tokens = [fake_token() for _ in range(5)]
    signatures = [cmd.sign_token_data(token) for token in tokens]
    cmd.send_token_data_list(tokens, signatures)


def test_sign_then_reset(cmd):
    token = fake_token()
    response = cmd.sign_token_data(token)
    response2 = cmd.sign_token_data(token)
    # assert that signature is deterministic
    assert response == response2

    # verify signature
    cmd.verify_token_signature(token, response)

    # verify invalid signature
    with pytest.raises(InvalidSignatureError):
        cmd.verify_token_signature(token, fake.binary(70))

    # reset signatures
    cmd.reset_token_signatures()

    # verify signature was invalidated
    with pytest.raises(InvalidSignatureError):
        cmd.verify_token_signature(token, response)
    # generate new signature
    response3 = cmd.sign_token_data(token)
    # assert signature changes
    assert response != response3
