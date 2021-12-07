import enum
import logging
import struct
from typing import List, Tuple, Union, Iterator, cast

from app_client.transaction import Transaction
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
        is_last = (i == (chunk-1)) and remaining
        yield is_last, data[offset:offset + chunk_len]
        offset += chunk_len

    if remaining:
        yield True, data[offset:]


class InsType(enum.IntEnum):
    INS_GET_VERSION = 0x03
    INS_GET_ADDRESS = 0x04
    INS_GET_XPUB = 0x05
    INS_SIGN_TX = 0x06


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

    def serialize(self,
                  cla: int,
                  ins: Union[int, enum.IntEnum],
                  p1: int = 0,
                  p2: int = 0,
                  cdata: bytes = b"") -> bytes:
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

        header: bytes = struct.pack("BBBBB",
                                    cla,
                                    ins,
                                    p1,
                                    p2,
                                    len(cdata))  # add Lc to APDU header

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
        return self.serialize(cla=0xB0,  # specific CLA for BOLOS
                              ins=0x01,
                              p1=0x00,
                              p2=0x00,
                              cdata=b"")

    def get_version(self) -> bytes:
        """Command builder for GET_VERSION.

        Returns
        -------
        bytes
            APDU command for GET_VERSION.

        """
        return self.serialize(cla=self.CLA,
                              ins=InsType.INS_GET_VERSION,
                              p1=0x00,
                              p2=0x00,
                              cdata=b"")

    def get_address(self, bip32_path: str) -> bytes:
        """Command builder for GET_ADDRESS.

        Returns
        -------
        bytes
            APDU command for GET_APP_NAME.

        """
        bip32_paths: List[bytes] = bip32_path_from_string(bip32_path)

        cdata: bytes = b"".join([
            len(bip32_paths).to_bytes(1, byteorder="big"),
            *bip32_paths
        ])
        return self.serialize(cla=self.CLA,
                              ins=InsType.INS_GET_ADDRESS,
                              p1=0x00,
                              p2=0x00,
                              cdata=cdata)

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

        cdata: bytes = b"".join([
            len(bip32_paths).to_bytes(1, byteorder="big"),
            *bip32_paths
        ])

        return self.serialize(cla=self.CLA,
                              ins=InsType.INS_GET_XPUB,
                              p1=0x00,
                              p2=0x00,
                              cdata=cdata)

    def sign_tx_send_data(self, transaction: Transaction, has_change: bool = False, change_index: int = None, bip32_path: str = None) -> Iterator[bytes]:
        """Command builder for INS_SIGN_TX.

        Parameters
        ----------
        transaction : Transaction
            Representation of the transaction to be signed.
        has_change: bool
            Wether the outputs of the transaction have a change output
        change_index: int
            The change output index, if it exists
        bip32_path : str
            String representation of the change address BIP32 path.

        Yields
        -------
        bytes
            APDU command chunk for INS_SIGN_TX.

        """
        cdata: bytes = None
        if has_change:
            bip32_paths: List[bytes] = bip32_path_from_string(bip32_path)

            cdata = b"".join([
                (0x80 | len(bip32_paths)).to_bytes(1, byteorder='big'),
                change_index.to_bytes(1, byteorder='big'),
                *bip32_paths
            ])
        else:
            cdata = b'\x00'
        cdata = b''.join([cdata, transaction.serialize()])
        for i, (is_last, chunk) in enumerate(chunkify(cdata, MAX_APDU_LEN)):
            yield self.serialize(
                    cla=self.CLA,
                    ins=InsType.INS_SIGN_TX,
                    p1=0x00,
                    p2=i,
                    cdata=chunk)

    def sign_tx_signatures(self, transaction: Transaction) -> Iterator[bytes]:

        # Ask for input signatures
        for i, tx_input in enumerate(transaction.inputs):
            assert tx_input.bip32_path is not None
            bip32_paths: List[bytes] = bip32_path_from_string(tx_input.bip32_path)

            input_data: bytes = b"".join([
                len(bip32_paths).to_bytes(1, byteorder="big"),
                *bip32_paths
            ])
            yield self.serialize(
                    cla=self.CLA,
                    ins=InsType.INS_SIGN_TX,
                    p1=0x01,
                    p2=0x00,
                    cdata=input_data)

    def sign_tx_end(self) -> bytes:
        return self.serialize(cla=self.CLA, ins=InsType.INS_SIGN_TX, p1=0x02, p2=0x00, cdata=b'')
