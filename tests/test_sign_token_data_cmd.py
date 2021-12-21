from utils import fake_token

def test_sign_token(cmd):
    token = fake_token()
    response = cmd.sign_token_data(token)
    print('sign_token response:', response)
    # verify signature
    cmd.verify_token_signature(token, response)
    # reset signatures
    cmd.reset_token_signatures()
    # verify signature to see it's invalid
