from utils import fake_tx


def test_sign_tx(cmd):
    tx = fake_tx(tokens=[])
    print("==" * 20)
    print("Inputs:", [str(inp) for inp in tx.inputs])
    print("Outputs:", [str(outp) for outp in tx.outputs])
    print("Tokens:", tx.tokens)
    print("==" * 20)
    cmd.sign_tx(fake_tx(tokens=[]), has_change=False)
