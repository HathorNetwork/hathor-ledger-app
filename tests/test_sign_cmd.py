from app_client.transaction import *


def test_sign_tx(cmd, button):
    bip32_path: str = "m/44'/280'/0'/0/0"

    script = b''.join([
        b'\x76\xa9\x14',
        b'\xca\xfe'*10,
        b'\x88\xac'
    ])

    inputs = [TxInput(b'\x00'*32, 0, bip32_path)]
    outputs = [TxOutput(100, script)]
    tokens = [b'\x00'*32]

    tx = Transaction(1, tokens, inputs, outputs)

    signatures = cmd.sign_tx(button=button,
                             transaction=tx,
                             has_change=False)

    assert len(signatures) == 1
