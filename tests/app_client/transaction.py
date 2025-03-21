import hashlib
from io import BytesIO
from typing import List, Union

import hathorlib
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.asymmetric import ec

from app_client.utils import bip32_path_from_string, read, read_int, read_uint, read_var


class TransactionError(Exception):
    pass


class TxInput:
    def __init__(self, tx_id: bytes, index: int, bip32_path: str = None):
        assert len(tx_id) == 32
        self.tx_id = tx_id
        self.index = index
        self.bip32_path = bip32_path

    def serialize(self) -> bytes:
        return b"".join(
            [self.tx_id, self.index.to_bytes(1, byteorder="big"), b"\x00\x00"]
        )

    @classmethod
    def from_bytes(cls, hexa: Union[bytes, BytesIO]):
        buf: BytesIO = BytesIO(hexa) if isinstance(hexa, bytes) else hexa

        tx_id: bytes = read(buf, 32)
        index: int = read_uint(buf, 8, byteorder="big")
        data_len: int = read_uint(buf, 16, byteorder="big")
        if data_len != 0:
            raise TransactionError("Input: data length MUST be 0")

        return cls(tx_id, index)

    def __str__(self):
        return (
            "TxInput("
            f"tx_id={self.tx_id.hex()}, "
            f"index={self.index}, "
            f"bip32_path={self.bip32_path})"
        )


class TxOutput:
    def __init__(
        self, value: int, script: bytes, token_data: int = 0, is_authority: bool = False
    ):
        self.value = value
        self.script = script
        # 0x80 is token authority mask
        self.token_data = token_data | 0x80 if is_authority else token_data

    def serialize_value(self) -> bytes:
        if self.value & 0x80000000:
            # value will use 8 bytes
            return (-self.value).to_bytes(8, byteorder="big", signed=True)
        return self.value.to_bytes(4, byteorder="big")

    def serialize(self) -> bytes:
        script_len = len(self.script)

        return b"".join(
            [
                self.serialize_value(),
                self.token_data.to_bytes(1, byteorder="big"),
                script_len.to_bytes(2, byteorder="big"),
                self.script,
            ]
        )

    @classmethod
    def from_bytes(cls, hexa: Union[bytes, BytesIO]):
        buf: BytesIO = BytesIO(hexa) if isinstance(hexa, bytes) else hexa

        b = buf.read(1)
        buf.seek(-1, 1)

        value: int = 0
        if b & 0x80:
            value = -read_int(buf, 64, byteorder="big")
        else:
            value = read_uint(buf, 32, byteorder="big")

        token_data: bytes = read(buf, 1)
        script_len, script = read_var(buf)

        return cls(value, script, token_data)

    def __str__(self):
        return (
            "TxOutput("
            f"value={self.value}, "
            f"token_data={self.token_data}, "
            f"script={self.script.hex()})"
        )


class Transaction:
    def __init__(
        self,
        tx_version,
        tokens: List[bytes],
        inputs: List[TxInput],
        outputs: List[TxOutput],
    ) -> None:
        self.tx_version = tx_version
        self.tokens = tokens
        self.inputs = inputs
        self.outputs = outputs
        self.sighash_all = None

    def serialize(self) -> bytes:
        """This serialize returns the number of complete outputs on each chunk
        this is imperative for testing, we need to know how many outputs to confirm
        """
        if self.sighash_all is not None:
            return self.sighash_all

        cdata = b"".join(
            [
                self.tx_version.to_bytes(2, byteorder="big"),
                len(self.tokens).to_bytes(1, byteorder="big"),
                len(self.inputs).to_bytes(1, byteorder="big"),
                len(self.outputs).to_bytes(1, byteorder="big"),
            ]
        )

        for token in self.tokens:
            cdata = b"".join([cdata, token])

        for tx_input in self.inputs:
            input_bytes = tx_input.serialize()
            cdata = b"".join([cdata, input_bytes])

        for tx_output in self.outputs:
            output_bytes = tx_output.serialize()
            cdata = b"".join([cdata, output_bytes])

        self.sighash_all = cdata
        return self.sighash_all

    @classmethod
    def from_bytes(cls, hexa: Union[bytes, BytesIO]):
        buf: BytesIO = BytesIO(hex) if isinstance(hexa, bytes) else hexa

        tx_version: int = read_uint(buf, 16, byteorder="big")
        num_tokens: int = read_uint(buf, 8, byteorder="big")
        num_inputs: int = read_uint(buf, 8, byteorder="big")
        num_outputs: int = read_uint(buf, 8, byteorder="big")

        tokens = []
        for _ in range(num_tokens):
            tokens.append(read(buf, 32))

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

        return cls(tx_version, tokens, inputs, outputs)

    def __str__(self):
        stokens = [token.hex() for token in self.tokens]
        sinputs = [str(inp) for inp in self.inputs]
        soutputs = [str(outp) for outp in self.outputs]
        return f"Transaction(tokens={stokens}, inputs={sinputs}, outputs={soutputs})"

    def verify_signature(self, signature: bytes, public_key_bytes: bytes):
        """Verify signature from `self.serialize` that returns the sighash_all bytes
        and `public_key_bytes` which is the compressed pubkey bytes
        """
        hash_ctx = hashlib.sha256()
        hash_ctx.update(self.serialize())
        pubkey = hathorlib.utils.get_public_key_from_bytes_compressed(public_key_bytes)
        return pubkey.verify(signature, hash_ctx.digest(), ec.ECDSA(hashes.SHA256()))


class ChangeInfo:
    def __init__(self, output_index: int, path: str) -> None:
        self.output_index = output_index
        self.path = path

    def __str__(self):
        return f"ChangeInfo(output_index={self.output_index}, path={self.path})"

    @property
    def bip32_path(self) -> List[bytes]:
        return bip32_path_from_string(self.path)

    def serialize(self) -> bytes:
        bip32_path = self.bip32_path
        return b"".join(
            [
                self.output_index.to_bytes(1, byteorder="big"),
                len(bip32_path).to_bytes(1, byteorder="big"),
                *bip32_path,
            ]
        )

    def old_proto_bytes(self) -> bytes:
        bip32_path = self.bip32_path
        return b"".join(
            [
                (0x80 | len(bip32_path)).to_bytes(1, byteorder="big"),
                self.output_index.to_bytes(1, byteorder="big"),
                *bip32_path,
            ]
        )
