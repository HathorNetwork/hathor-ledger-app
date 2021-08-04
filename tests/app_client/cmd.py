import struct
from typing import Tuple, List

from ledgercomm import Transport

from app_client.cmd_builder import CommandBuilder, InsType
from app_client.button import Button
from app_client.exception import DeviceException
from app_client.transaction import Transaction


class Command:
    def __init__(self,
                 transport: Transport,
                 debug: bool = False) -> None:
        self.transport = transport
        self.builder = CommandBuilder(debug=debug)
        self.debug = debug

    def get_app_and_version(self) -> Tuple[str, str]:
        sw, response = self.transport.exchange_raw(
            self.builder.get_app_and_version()
        )  # type: int, bytes

        if sw != 0x9000:
            raise DeviceException(error_code=sw, ins=0x01)

        # response = format_id (1) ||
        #            app_name_len (1) ||
        #            app_name (var) ||
        #            version_len (1) ||
        #            version (var) ||
        offset: int = 0

        format_id: int = response[offset]
        offset += 1
        app_name_len: int = response[offset]
        offset += 1
        app_name: str = response[offset:offset + app_name_len].decode("ascii")
        offset += app_name_len
        version_len: int = response[offset]
        offset += 1
        version: str = response[offset:offset + version_len].decode("ascii")
        offset += version_len

        return app_name, version

    def get_version(self) -> Tuple[bytes, int, int, int]:
        sw, response = self.transport.exchange_raw(
            self.builder.get_version()
        )  # type: int, bytes

        if sw != 0x9000:
            raise DeviceException(error_code=sw, ins=InsType.INS_GET_VERSION)

        # response = 'H' || 'T' || 'R' || MAJOR (1) || MINOR (1) || PATCH (1)
        assert len(response) == 6

        h, t, r, major, minor, patch = struct.unpack(
            "cccBBB",
            response
        )  # type: bytes, bytes, bytes, int, int, int

        htr = b''.join([h, t, r])

        return htr, major, minor, patch

    def get_address(self, button: Button, bip32_path: str) -> str:
        self.transport.send_raw(self.builder.get_address(bip32_path))

        # Go to approve screen (screen loops back)
        button.left_click()
        button.left_click()
        # Approve
        button.both_click()

        sw, response = self.transport.recv()  # type: int, bytes

        if sw != 0x9000:
            raise DeviceException(error_code=sw, ins=InsType.INS_GET_ADDRESS)

        return

    def get_xpub(self, button: Button, bip32_path: str) -> Tuple[bytes, bytes]:
        self.transport.send_raw(self.builder.get_xpub(bip32_path=bip32_path))

        # Go to approve screen (screen loops back)
        button.left_click()
        button.left_click()
        # Approve
        button.both_click()

        sw, response = self.transport.recv()  # type: int, bytes

        if sw != 0x9000:
            raise DeviceException(error_code=sw, ins=InsType.INS_GET_XPUB)

        # response = raw_public_key(65) ||
        #            chain_code(32) ||
        #            fingerprint(4)
        offset: int = 0
        pub_key_len: int = 65
        chain_code_len: int = 32

        pub_key: bytes = response[offset:offset + pub_key_len]
        offset += pub_key_len
        chain_code: bytes = response[offset:offset + chain_code_len]
        offset += chain_code_len
        fingerprint: bytes = response[offset:offset + 4]
        offset += 4

        assert len(response) == pub_key_len + chain_code_len + 4

        return pub_key, chain_code, fingerprint

    def sign_tx(self, button: Button, transaction: Transaction, has_change: bool = False, change_index:int = None, change_path: str = None) -> List[bytes]:
        def handle_data(num_outputs: int, is_last: bool):
            for _ in range(num_outputs):
                # Go to approve screen (screen loops back)
                button.left_click()
                button.left_click()
                # Approve
                button.both_click()

            if is_last:
                # One more time for Send Transaction confirmation
                # Go to approve screen (screen loops back)
                button.left_click()
                button.left_click()
                # Approve
                button.both_click()

        sw: int
        response: bytes = b""

        signatures: List[bytes] = []
        stage = 0
        for is_last, num_outputs, chunk in self.builder.sign_tx(transaction=transaction, has_change=has_change, change_index=change_index, bip32_path=change_path):
            self.transport.send_raw(chunk)

            if stage == 0:
                handle_data(num_outputs, is_last)

            sw, response = self.transport.recv()  # type: int, bytes
            print('\n', 'ledger_resp:', sw, response)

            if sw != 0x9000:
                raise DeviceException(error_code=sw, ins=InsType.INS_SIGN_TX)

            if stage == 1:
                signatures.append(response)

            if is_last:
                stage += 1

        return signatures
