def test_get_public_key(cmd):
    pub_key, chain_code, fingerprint = cmd.get_xpub(
        bip32_path="m/44'/280'/0'/0/0"
    )  # type: bytes, bytes, bytes

    # Generated with bitcore-lib from the speculos xpub
    # m/44'/280'/0'/0/0
    # "04962e6c4afe696afa985363fb53bee05cd22463b1cb79bde72ffb8fbd029c6e7dd6469fb7bc5bdf9e362212434f581d882cbba2522e2a708340d2cba101c5e850"
    actual_pubkey_0 = bytes.fromhex(
        (
            "04962e6c4afe696afa985363fb53bee05cd22463b1cb7"
            "9bde72ffb8fbd029c6e7dd6469fb7bc5bdf9e36221243"
            "4f581d882cbba2522e2a708340d2cba101c5e850"
        )
    )
    actual_chain_0 = bytes.fromhex(
        ("0e0bc8953fc617b281ff16d51a771cee" "e6a20c6f2c3328d6094cb5ab720c5722")
    )
    # "m/44'/280'/0'/0/10"
    # "041e924ed93ca4395048a11007de661b9d16c42dbad3c01743ddd68d8919f945ae21ddb91d1ae45c86c2142bd789dc3628ae796c8fd693a0aa5bca487c60552c5b"
    actual_pubkey_10 = bytes.fromhex(
        (
            "041e924ed93ca4395048a11007de661b9d16c42dbad3c"
            "01743ddd68d8919f945ae21ddb91d1ae45c86c2142bd7"
            "89dc3628ae796c8fd693a0aa5bca487c60552c5b"
        )
    )
    actual_chain_10 = bytes.fromhex(
        ("4a7617afbcbff5beed1b50811a4e0744" "4199130e5da7038339ce4ffde9488da6")
    )

    assert len(pub_key) == 65
    assert pub_key == actual_pubkey_0
    assert len(chain_code) == 32
    assert chain_code == actual_chain_0
    assert len(fingerprint) == 4
    assert fingerprint == bytes.fromhex("4b38fea9")

    pub_key2, chain_code2, fingerprint2 = cmd.get_xpub(
        bip32_path="m/44'/280'/0'/0/10"
    )  # type: bytes, bytes, bytes

    assert len(pub_key2) == 65
    assert pub_key2 == actual_pubkey_10
    assert len(chain_code2) == 32
    assert chain_code2 == actual_chain_10
    assert len(fingerprint2) == 4
    assert fingerprint2 == bytes.fromhex("4b38fea9")
