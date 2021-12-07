from typing import Tuple
from abc import ABCMeta, abstractmethod
from urllib.parse import urljoin

from requests import Session


class ApduTransport(metaclass=ABCMeta):
    @abstractmethod
    def exchange_apdu_raw(self, data: bytes):
        ...


class TransportAPI(ApduTransport):

    def __init__(self, server: str) -> None:
        self.server = server
        self.session = Session()

    def close(self) -> None:
        self.session.close()

    def endpoint(self, path: str) -> str:
        return urljoin(self.server, path)

    def exchange_apdu_raw(self, data: bytes) -> Tuple[int, bytes]:
        print('Transport =====')
        print(data.hex())
        url = self.endpoint('/apdu')
        print(url)
        response = self.session.post(url, json={"data": data.hex()})
        print(response.status_code, response.text)
        if response.status_code != 200:
            raise Exception('Exchange failed with {}'.format(data.hex()))
        rdata = response.json()
        cdata = rdata['data']
        sw = int.from_bytes(bytes.fromhex(cdata[-4:]), byteorder='big', signed=False)
        return sw, bytes.fromhex(cdata[:-4])
