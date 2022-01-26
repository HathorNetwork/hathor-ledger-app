def test_get_public_key(cmd):
    pub_key, chain_code, fingerprint = cmd.get_xpub(
        bip32_path="m/44'/280'/0'/0/0"
    )  # type: bytes, bytes, bytes

    assert len(pub_key) == 65
    assert pub_key == bytes.fromhex("04962e6c4afe696afa985363fb53bee05cd22463b1cb79bde72ffb8fbd029c6e7dd6469fb7bc5bdf9e362212434f581d882cbba2522e2a708340d2cba101c5e850")
    assert len(chain_code) == 32
    assert chain_code == bytes.fromhex("0e0bc8953fc617b281ff16d51a771ceee6a20c6f2c3328d6094cb5ab720c5722")
    assert len(fingerprint) == 4
    assert fingerprint == bytes.fromhex("4b38fea9")

    pub_key2, chain_code2, fingerprint2 = cmd.get_xpub(
        bip32_path="m/44'/280'/0'/0/10"
    )  # type: bytes, bytes, bytes


    assert len(pub_key2) == 65
    assert pub_key2 == bytes.fromhex("041e924ed93ca4395048a11007de661b9d16c42dbad3c01743ddd68d8919f945ae21ddb91d1ae45c86c2142bd789dc3628ae796c8fd693a0aa5bca487c60552c5b")
    assert len(chain_code2) == 32
    assert chain_code2 == bytes.fromhex("4a7617afbcbff5beed1b50811a4e07444199130e5da7038339ce4ffde9488da6")
    assert len(fingerprint2) == 4
    assert fingerprint2 == bytes.fromhex("4b38fea9")
