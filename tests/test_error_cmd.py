import pytest

from app_client.exception import *


@pytest.mark.xfail(raises=ClaNotSupportedError)
def test_bad_cla(cmd):
    c = cmd.builder.serialize(cla=0xa0,  # 0xa0 instead of 0xe0
                               ins=0x03,
                               p1=0x00,
                               p2=0x00,
                               cdata=b"")

    sw, _ = cmd.transport.exchange_apdu_raw(c)

    raise DeviceException(error_code=sw)


@pytest.mark.xfail(raises=InsNotSupportedError)
def test_bad_ins(cmd):
    c = cmd.builder.serialize(cla=0xe0,
                                   ins=0xff,  # bad INS
                                   p1=0x00,
                                   p2=0x00,
                                   cdata=b"")

    sw, _ = cmd.transport.exchange_apdu_raw(c)

    raise DeviceException(error_code=sw)


@pytest.mark.xfail(raises=WrongP1P2Error)
def test_wrong_p1p2(cmd):
    c = cmd.builder.serialize(cla=0xe0,
                                   ins=0x03,
                                   p1=0x01,  # 0x01 instead of 0x00
                                   p2=0x00,
                                   cdata=b"")

    sw, _ = cmd.transport.exchange_apdu_raw(c)

    raise DeviceException(error_code=sw)


@pytest.mark.xfail(raises=WrongDataLengthError)
def test_wrong_data_length(cmd):
    # APDUs must be at least 5 bytes: CLA, INS, P1, P2, Lc.
    sw, _ = cmd.transport.exchange_apdu_raw(bytes.fromhex('E000'))

    raise DeviceException(error_code=sw)
