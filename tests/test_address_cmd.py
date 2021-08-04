import pytest

from app_client.exception import BOLOSPathPrefixError, WrongDataLengthError

def test_address(cmd, button):
    cmd.get_address(button=button, bip32_path="m/44'/280'/0'/0")
    cmd.get_address(button=button, bip32_path="m/44'/280'/0'/0/0")
    cmd.get_address(button=button, bip32_path="m/44'/280'/0'/0/1")

def test_address_limits(cmd, button):
    with pytest.raises(BOLOSPathPrefixError):
        cmd.get_address(button=button, bip32_path="m/44'/28'")
    with pytest.raises(WrongDataLengthError):
        cmd.get_address(button=button, bip32_path="m/44'/280'/0'/0/1/0")
