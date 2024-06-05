import hashlib
from io import BytesIO
from typing import List, Union

import hathorlib
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.asymmetric import ec

from app_client.transaction import TxInput, TxOutput
from app_client.utils import read, read_uint, read_var

TOKEN_INFO_VERSION = 1
CREATE_TOKEN_TX_VERSION = 3


class CreateTokenTransaction:
    def __init__(
        self,
        signal_bits,
        name: str,
        symbol: str,
        inputs: List[TxInput],
        outputs: List[TxOutput],
    ) -> None:
        self.tx_version = CREATE_TOKEN_TX_VERSION
        self.name = name
        self.symbol = symbol
        self.inputs = inputs
        self.outputs = outputs
        self.sighash_all = None

    def serialize_token(self) -> bytes:
        """
        This serializes only the token information
        """
        return b"".join(
            [
                TOKEN_INFO_VERSION.to_bytes(1, byteorder="big"),
                len(self.name.encode()).to_bytes(1, byteorder="big"),
                self.name.encode(),
                len(self.symbol.encode()).to_bytes(1, byteorder="big"),
                self.symbol.encode(),
            ]
        )

    def serialize(self) -> bytes:
        """
        This serialize returns the number of complete outputs on each chunk
        this is imperative for testing,
        we need to know how many outputs to confirm
        """
        if self.sighash_all is not None:
            return self.sighash_all

        cdata = b"".join(
            [
                self.signal_bits.to_bytes(1, byteorder="big"),
                self.tx_version.to_bytes(1, byteorder="big"),
                len(self.inputs).to_bytes(1, byteorder="big"),
                len(self.outputs).to_bytes(1, byteorder="big"),
            ]
        )

        for tx_input in self.inputs:
            input_bytes = tx_input.serialize()
            cdata = b"".join([cdata, input_bytes])

        for tx_output in self.outputs:
            output_bytes = tx_output.serialize()
            cdata = b"".join([cdata, output_bytes])

        cdata = b"".join([cdata, self.serialize_token()])

        self.sighash_all = cdata
        return self.sighash_all

    @classmethod
    def from_bytes(cls, hexa: Union[bytes, BytesIO]):
        buf: BytesIO = BytesIO(hex) if isinstance(hexa, bytes) else hexa

        signal_bits: int = read_uint(buf, 8, byteorder="big")
        tx_version: int = read_uint(buf, 8, byteorder="big")
        if tx_version != CREATE_TOKEN_TX_VERSION:
            raise ValueError(
                f"CreateTokenTransaction with version {tx_version} not supported"
            )
        num_inputs: int = read_uint(buf, 8, byteorder="big")
        num_outputs: int = read_uint(buf, 8, byteorder="big")

        inputs = []
        for _ in range(num_inputs):
            input_bytes = read(buf, 32)
            tx_input = TxInput.from_bytes(input_bytes)
            inputs.append(tx_input)

        outputs = []
        for _ in range(num_outputs):
            output_bytes = read(buf, 32)
            tx_output = TxOutput.from_bytes(output_bytes)
            outputs.append(tx_output)

        token_info_version: int = read_uint(buf, 8, byteorder="big")
        if token_info_version != TOKEN_INFO_VERSION:
            raise ValueError(f"Token info version {token_info_version} not supported")
        name_len, name = read_var(buf)
        symbol_len, symbol = read_var(buf)

        return cls(signal_bits, inputs, outputs)

    def __str__(self):
        sinputs = [str(inp) for inp in self.inputs]
        soutputs = [str(outp) for outp in self.outputs]
        return (
            f"CreateTokenTransaction(symbol={self.symbol},"
            f" name={self.name}, inputs={sinputs}, outputs={soutputs})"
        )

    def verify_signature(self, signature: bytes, public_key_bytes: bytes):
        """Verify signature from `self.serialize` that returns the sighash_all bytes
        and `public_key_bytes` which is the compressed pubkey bytes
        """
        hash_ctx = hashlib.sha256()
        hash_ctx.update(self.serialize())
        pubkey = hathorlib.utils.get_public_key_from_bytes_compressed(public_key_bytes)
        return pubkey.verify(signature, hash_ctx.digest(), ec.ECDSA(hashes.SHA256()))
