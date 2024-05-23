import struct
from typing import List, Tuple

from app_client.cmd_builder import CommandBuilder, InsType
from app_client.exception import DeviceException
from app_client.token import Token
from app_client.transaction import ChangeInfo, Transaction
from app_client.transport import ApduTransport


class Command:
    def __init__(self, transport: ApduTransport, debug: bool = False) -> None:
        self.builder = CommandBuilder(debug=debug)
        self.debug = debug
        self.transport = transport

    def get_app_and_version(self) -> Tuple[str, str]:
        print("[+] sending: get_app_and_version request")
        sw, response = self.transport.exchange_apdu_raw(
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

        # format_id: int = response[offset]
        offset += 1
        app_name_len: int = response[offset]
        offset += 1
        app_name: str = response[offset : offset + app_name_len].decode("ascii")
        offset += app_name_len
        version_len: int = response[offset]
        offset += 1
        version: str = response[offset : offset + version_len].decode("ascii")
        offset += version_len

        return app_name, version

    def get_version(self) -> Tuple[bytes, int, int, int]:
        print("[+] sending: get_version request")
        sw, response = self.transport.exchange_apdu_raw(self.builder.get_version())

        if sw != 0x9000:
            raise DeviceException(error_code=sw, ins=InsType.INS_GET_VERSION)

        # response = 'H' || 'T' || 'R' || MAJOR (1) || MINOR (1) || PATCH (1)
        assert len(response) == 6

        h, t, r, major, minor, patch = struct.unpack(
            "cccBBB", response
        )  # type: bytes, bytes, bytes, int, int, int

        htr = b"".join([h, t, r])

        return htr, major, minor, patch

    def get_address(self, bip32_path: str) -> str:

        print("[+] sending: get_address request")
        sw, response = self.transport.exchange_apdu_raw(
            self.builder.get_address(bip32_path)
        )

        if sw != 0x9000:
            raise DeviceException(error_code=sw, ins=InsType.INS_GET_ADDRESS)

        return

    def get_xpub(self, bip32_path: str) -> Tuple[bytes, bytes]:
        print("[+] sending: get_xpub request")
        sw, response = self.transport.exchange_apdu_raw(
            self.builder.get_xpub(bip32_path=bip32_path)
        )

        if sw != 0x9000:
            raise DeviceException(error_code=sw, ins=InsType.INS_GET_XPUB)

        # response = raw_public_key(65) ||
        #            chain_code(32) ||
        #            fingerprint(4)
        offset: int = 0
        pub_key_len: int = 65
        chain_code_len: int = 32

        pub_key: bytes = response[offset : offset + pub_key_len]
        offset += pub_key_len
        chain_code: bytes = response[offset : offset + chain_code_len]
        offset += chain_code_len
        fingerprint: bytes = response[offset : offset + 4]
        offset += 4

        assert len(response) == pub_key_len + chain_code_len + 4

        return pub_key, chain_code, fingerprint

    def sign_tx(
        self,
        transaction: Transaction,
        change_list: List["ChangeInfo"] = [],
        use_old_protocol: bool = False,
    ) -> List[bytes]:

        sw: int
        response: bytes = b""

        signatures: List[bytes] = []

        index = 0
        for chunk in self.builder.sign_tx_send_data(
            transaction=transaction,
            change_list=change_list,
            use_old_protocol=use_old_protocol,
        ):
            print("[+] sending: sign_tx send for chunk {}\n".format(index))
            index += 1
            sw, response = self.transport.exchange_apdu_raw(chunk)
            print("\n", "ledger_resp:", sw, response)

            if sw != 0x9000:
                raise DeviceException(error_code=sw, ins=InsType.INS_SIGN_TX)

        index = 0
        # ask for signatures
        for chunk in self.builder.sign_tx_signatures(transaction):
            print("[+] sending: sign_tx sign for chunk {}\n".format(index))
            index += 1
            sw, response = self.transport.exchange_apdu_raw(chunk)
            print("\n", "ledger_resp:", sw, response)

            if sw != 0x9000:
                raise DeviceException(error_code=sw, ins=InsType.INS_SIGN_TX)

            signatures.append(response)

        sw, response = self.transport.exchange_apdu_raw(self.builder.sign_tx_end())

        if sw != 0x9000:
            raise DeviceException(error_code=sw, ins=InsType.INS_SIGN_TX)

        return signatures

    def sign_token_data(self, token: Token) -> bytes:
        print("[+] sending: sign_token_data request")
        sw, response = self.transport.exchange_apdu_raw(
            self.builder.sign_token_data(token)
        )

        if sw != 0x9000:
            raise DeviceException(error_code=sw, ins=InsType.INS_GET_ADDRESS)

        return response

    def send_token_data(self, token: Token, signature: bytes, num: int = 0):
        print("[+] sending: send_token_data request")
        sw, response = self.transport.exchange_apdu_raw(
            self.builder.send_token_data(token, signature, num=num)
        )

        if sw != 0x9000:
            raise DeviceException(error_code=sw, ins=InsType.INS_GET_ADDRESS)

    def send_token_data_list(self, tokens: List[Token], signatures: List[bytes]):
        assert len(tokens) == len(signatures)
        for i, token in enumerate(tokens):
            print(
                "[+] sending: send_token_data_list request for token {} - {}".format(
                    i, token
                )
            )
            self.send_token_data(token, signatures[i], num=i)

    def verify_token_signature(self, token: Token, signature: bytes):
        print("[+] sending: verify_token_signature request")
        sw, response = self.transport.exchange_apdu_raw(
            self.builder.verify_token_signature(token, signature)
        )

        if sw != 0x9000:
            raise DeviceException(error_code=sw, ins=InsType.INS_GET_ADDRESS)

    def reset_token_signatures(self):
        print("[+] sending: reset_token_signatures request")
        sw, response = self.transport.exchange_apdu_raw(
            self.builder.reset_token_signatures()
        )

        if sw != 0x9000:
            raise DeviceException(error_code=sw, ins=InsType.INS_GET_ADDRESS)
