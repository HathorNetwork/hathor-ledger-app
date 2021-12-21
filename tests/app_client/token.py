from typing import Optional


class TokenError(Exception):
    pass

class Token:
    def __init__(self, version: int, symbol: str, name: str, uid_hex: str):
        assert len(uid_hex) == 64
        self.version = version
        self.symbol = symbol
        self.name = name
        self.uid_hex = uid_hex

    @property
    def uid(self):
        return bytes.fromhex(self.uid_hex)

    def serialize(self, signature: Optional[bytes] = None) -> bytes:
        cdata = b''.join([
            self.version.to_bytes(1, byteorder='big'),
            self.uid,
            len(self.symbol).to_bytes(1, byteorder='big'),
            self.symbol.encode(),
            len(self.name).to_bytes(1, byteorder='big'),
            self.name.encode(),
        ])

        if signature is not None:
            cdata = b''.join([cdata, len(signature).to_bytes(1, byteorder='big'), signature])

        return cdata
