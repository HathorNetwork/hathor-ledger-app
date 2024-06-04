import enum
import logging
import struct
from typing import Iterator, List, Tuple, Union, cast

from app_client.create_token_transaction import CreateTokenTransaction
from app_client.token import Token
from app_client.transaction import ChangeInfo, Transaction
from app_client.utils import bip32_path_from_string

MAX_APDU_LEN: int = 255


def chunkify(data: bytes, chunk_len: int) -> Iterator[Tuple[bool, bytes]]:
    size: int = len(data)

    if size <= chunk_len:
        yield True, data
        return

    chunk: int = size // chunk_len
    remaining: int = size % chunk_len
    offset: int = 0

    for i in range(chunk):
        is_last = (i == (chunk - 1)) and remaining
        yield is_last, data[offset : offset + chunk_len]
        offset += chunk_len

    if remaining:
        yield True, data[offset:]


class InsType(enum.IntEnum):
    INS_GET_VERSION = 0x03
    INS_GET_ADDRESS = 0x04
    INS_GET_XPUB = 0x05
    INS_SIGN_TX = 0x06
    INS_SIGN_TOKEN_DATA = 0x07
    INS_SEND_TOKEN_DATA = 0x08
    INS_VERIFY_TOKEN_SIGNATURE = 0x09
    INS_RESET_TOKEN_SIGNATURES = 0x0A
    INS_SEND_CREATE_TOKEN_DATA = 0x0B


class CommandBuilder:
    """APDU command builder for the application.

    Parameters
    ----------
    debug: bool
        Whether you want to see logging or not.

    Attributes
    ----------
    debug: bool
        Whether you want to see logging or not.

    """

    CLA: int = 0xE0

    def __init__(self, debug: bool = False):
        """Init constructor."""
        self.debug = debug

    def serialize(
        self,
        cla: int,
        ins: Union[int, enum.IntEnum],
        p1: int = 0,
        p2: int = 0,
        cdata: bytes = b"",
    ) -> bytes:
        """Serialize the whole APDU command (header + data).

        Parameters
        ----------
        cla : int
            Instruction class: CLA (1 byte)
        ins : Union[int, IntEnum]
            Instruction code: INS (1 byte)
        p1 : int
            Instruction parameter 1: P1 (1 byte).
        p2 : int
            Instruction parameter 2: P2 (1 byte).
        cdata : bytes
            Bytes of command data.

        Returns
        -------
        bytes
            Bytes of a complete APDU command.

        """
        ins = cast(int, ins.value) if isinstance(ins, enum.IntEnum) else cast(int, ins)

        header: bytes = struct.pack(
            "BBBBB", cla, ins, p1, p2, len(cdata)
        )  # add Lc to APDU header

        if self.debug:
            logging.info("header: %s", header.hex())
            logging.info("cdata:  %s", cdata.hex())

        return header + cdata

    def get_app_and_version(self) -> bytes:
        """Command builder for GET_APP_AND_VERSION (builtin in BOLOS SDK).

        Returns
        -------
        bytes
            APDU command for GET_APP_AND_VERSION.

        """
        return self.serialize(
            cla=0xB0, ins=0x01, p1=0x00, p2=0x00, cdata=b""  # specific CLA for BOLOS
        )

    def get_version(self) -> bytes:
        """Command builder for GET_VERSION.

        Returns
        -------
        bytes
            APDU command for GET_VERSION.

        """
        return self.serialize(
            cla=self.CLA, ins=InsType.INS_GET_VERSION, p1=0x00, p2=0x00, cdata=b""
        )

    def get_address(self, bip32_path: str) -> bytes:
        """Command builder for GET_ADDRESS.

        Returns
        -------
        bytes
            APDU command for GET_APP_NAME.

        """
        bip32_paths: List[bytes] = bip32_path_from_string(bip32_path)

        cdata: bytes = b"".join(
            [len(bip32_paths).to_bytes(1, byteorder="big"), *bip32_paths]
        )
        return self.serialize(
            cla=self.CLA, ins=InsType.INS_GET_ADDRESS, p1=0x00, p2=0x00, cdata=cdata
        )

    def get_xpub(self, bip32_path: str) -> bytes:
        """Command builder for GET_XPUB.

        Parameters
        ----------
        bip32_path: str
            String representation of BIP32 path.

        Returns
        -------
        bytes
            APDU command for GET_XPUB.

        """
        bip32_paths: List[bytes] = bip32_path_from_string(bip32_path)

        cdata: bytes = b"".join(
            [len(bip32_paths).to_bytes(1, byteorder="big"), *bip32_paths]
        )

        return self.serialize(
            cla=self.CLA, ins=InsType.INS_GET_XPUB, p1=0x00, p2=0x00, cdata=cdata
        )

    def sign_tx_send_data(
        self,
        transaction: Transaction,
        change_list: List["ChangeInfo"] = [],
        use_old_protocol: bool = False,
    ) -> Iterator[bytes]:
        """Command builder for INS_SIGN_TX.

        Parameters
        ----------
        transaction : Transaction
            Representation of the transaction to be signed.
        change_list: List[ChangeInfo]
            List of change information, default empty list.
        use_old_protocol: bool
            Weather to use the old or new protocol, default True.

        Yields
        -------
        bytes
            APDU command chunk for INS_SIGN_TX.

        """
        cdata: bytes = None
        if use_old_protocol:
            if len(change_list) > 0:
                # Old proto only allows 1 change
                # ignore the rest of the list
                cdata = change_list[0].old_proto_bytes()
                print("Old change {}".format(cdata.hex()))
            else:
                # No change
                cdata = b"\x00"
        else:
            cdata = b"".join(
                [
                    b"\x01",  # version byte
                    len(change_list).to_bytes(1, byteorder="big"),
                    *[c.serialize() for c in change_list],
                ]
            )
            print("change {}".format(cdata.hex()))
        cdata = b"".join([cdata, transaction.serialize()])
        for i, (is_last, chunk) in enumerate(chunkify(cdata, MAX_APDU_LEN)):
            yield self.serialize(
                cla=self.CLA, ins=InsType.INS_SIGN_TX, p1=0x00, p2=i, cdata=chunk
            )

    def sign_tx_signatures(self, transaction: Transaction) -> Iterator[bytes]:

        # Ask for input signatures
        for i, tx_input in enumerate(transaction.inputs):
            assert tx_input.bip32_path is not None
            bip32_paths: List[bytes] = bip32_path_from_string(tx_input.bip32_path)

            input_data: bytes = b"".join(
                [len(bip32_paths).to_bytes(1, byteorder="big"), *bip32_paths]
            )
            yield self.serialize(
                cla=self.CLA,
                ins=InsType.INS_SIGN_TX,
                p1=0x01,
                p2=0x00,
                cdata=input_data,
            )

    def sign_tx_end(self) -> bytes:
        return self.serialize(
            cla=self.CLA, ins=InsType.INS_SIGN_TX, p1=0x02, p2=0x00, cdata=b""
        )

    def sign_token_data(self, token: Token) -> bytes:
        return self.serialize(
            cla=self.CLA,
            ins=InsType.INS_SIGN_TOKEN_DATA,
            p1=0x00,
            p2=0x00,
            cdata=token.serialize(),
        )

    def send_token_with_signature(
        self, ins: InsType, token: Token, signature: bytes, num: int = 0
    ) -> bytes:
        return self.serialize(
            cla=self.CLA,
            ins=ins,
            p1=num,
            p2=0x00,
            cdata=token.serialize(signature=signature),
        )

    def send_token_data(self, token: Token, signature: bytes, num: int = 0) -> bytes:
        return self.send_token_with_signature(
            InsType.INS_SEND_TOKEN_DATA, token, signature, num=num
        )

    def verify_token_signature(self, token: Token, signature: bytes) -> bytes:
        return self.send_token_with_signature(
            InsType.INS_VERIFY_TOKEN_SIGNATURE, token, signature
        )

    def reset_token_signatures(self) -> bytes:
        return self.serialize(
            cla=self.CLA,
            ins=InsType.INS_RESET_TOKEN_SIGNATURES,
            p1=0x00,
            p2=0x00,
            cdata=b"",
        )

    def send_create_token_data(self, tx: CreateTokenTransaction) -> bytes:
        return self.serialize(
            cla=self.CLA,
            ins=InsType.INS_SEND_CREATE_TOKEN_DATA,
            p1=0x00,
            p2=0x00,
            cdata=tx.serialize_token(),
        )
