#!/usr/bin/env python
# Copyright 2016-2021 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.
from usb_packet import USB_PID, TokenPacket, RxDataPacket
from usb_session import UsbSession
from usb_transaction import UsbTransaction
import pytest
from conftest import PARAMS, test_RunUsbSession

# TODO Can this be moved?
@pytest.fixture
def test_file():
    return __file__


@pytest.fixture
def test_session(ep, address, bus_speed):

    ep = 1
    address = 1

    # Note, quite big gap to allow checking
    ied = 4000

    session = UsbSession(
        bus_speed=bus_speed, run_enumeration=False, device_address=address
    )

    for pktLength in range(10, 14):

        if pktLength == 12:
            session.add_event(
                TokenPacket(
                    pid=USB_PID["IN"],
                    address=address,
                    endpoint=ep,
                    interEventDelay=ied,
                )
            )
            session.add_event(
                RxDataPacket(
                    dataPayload=session.getPayload_in(ep, pktLength, resend=True)
                )
            )
            # Missing ACK - simulate CRC fail at host

        session.add_event(
            UsbTransaction(
                session,
                deviceAddress=address,
                endpointNumber=ep,
                endpointType="BULK",
                direction="IN",
                dataLength=pktLength,
                interEventDelay=ied,
            )
        )

    return session
