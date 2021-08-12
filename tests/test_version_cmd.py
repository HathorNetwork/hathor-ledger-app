def test_version(cmd):
    assert cmd.get_version() == (b'HTR', 1, 0, 0)
