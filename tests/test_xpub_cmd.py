def test_get_public_key(cmd, button):
    pub_key, chain_code, fingerprint = cmd.get_xpub(
        button=button,
        bip32_path="m/44'/280'/0'/0/0")  # type: bytes, bytes, bytes

    assert len(pub_key) == 65
    assert len(chain_code) == 32
    assert len(fingerprint) == 4

    pub_key2, chain_code2, fingerprint2 = cmd.get_xpub(
        button=button,
        bip32_path="m/44'/280'/0'/0/10")  # type: bytes, bytes, bytes

    assert len(pub_key2) == 65
    assert len(chain_code2) == 32
    assert len(fingerprint2) == 4
