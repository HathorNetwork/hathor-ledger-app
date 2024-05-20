from faker import Faker

from app_client.transaction import Transaction, TxInput, TxOutput
from utils import fake_path, fake_script, fake_token

fake = Faker()


def test_sign_tx_with_token(cmd):
    num = fake.pyint(2, 8)
    print("Sending TX with {} custom tokens".format(num))

    tokens = [fake_token() for _ in range(num)]
    inputs = [
        TxInput(fake.sha256(True), fake.pyint(0, 255), fake_path())
        for _ in range(num + 1)
    ]
    outputs = [TxOutput(fake.pyint(1), fake_script(), x) for x in range(num + 1)]
    tx = Transaction(0, 1, [t.uid for t in tokens], inputs, outputs)
    print(str(tx))
    sigs = []
    for i in range(num):
        print("Signing token", tokens[i].symbol, tokens[i].name)
        sig = cmd.sign_token_data(tokens[i])
        sigs.append(sig)
    # send_token_data
    cmd.send_token_data_list(tokens, sigs)
    # sign_tx with token
    cmd.sign_tx(tx)
