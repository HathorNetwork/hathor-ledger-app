from utils import fake_tx


def test_sign_tx(cmd):
    cmd.sign_tx(fake_tx(), has_change=False)
