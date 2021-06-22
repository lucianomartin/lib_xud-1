#!/usr/bin/env python
# Copyright 2016-2021 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

# Basic check of PING functionality
from usb_packet import (
    TokenPacket,
    TxDataPacket,
    RxDataPacket,
    TxHandshakePacket,
    RxHandshakePacket,
    USB_PID,
)
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

    address = 1
    ep = 1

    session = UsbSession(
        bus_speed=bus_speed, run_enumeration=False, device_address=address
    )

    # Ping EP 2, expect NAK
    session.add_event(
        TokenPacket(
            pid=USB_PID["PING"],
            address=address,
            endpoint=2,
        )
    )
    session.add_event(RxHandshakePacket(pid=USB_PID["NAK"]))

    # And again
    session.add_event(
        TokenPacket(
            pid=USB_PID["PING"],
            address=address,
            endpoint=2,
        )
    )
    session.add_event(RxHandshakePacket(pid=USB_PID["NAK"]))

    # Send packet to EP 1, xCORE should mark EP 2 as ready
    session.add_event(
        UsbTransaction(
            session,
            deviceAddress=address,
            endpointNumber=ep,
            endpointType="BULK",
            direction="OUT",
            dataLength=10,
        )
    )

    # Ping EP 2 again - expect ACK
    session.add_event(
        TokenPacket(
            pid=USB_PID["PING"],
            address=address,
            endpoint=2,
            interEventDelay=6000,
        )
    )
    session.add_event(RxHandshakePacket(pid=USB_PID["ACK"]))

    # And again..
    session.add_event(
        TokenPacket(
            pid=USB_PID["PING"],
            address=address,
            endpoint=2,
            interEventDelay=6000,
        )
    )
    session.add_event(RxHandshakePacket(pid=USB_PID["ACK"]))

    # Send out to EP 2.. expect ack
    session.add_event(
        UsbTransaction(
            session,
            deviceAddress=address,
            endpointNumber=2,
            endpointType="BULK",
            direction="OUT",
            dataLength=10,
            interEventDelay=6000,
        )
    )

    # Re-Ping EP 2, expect NAK
    session.add_event(
        TokenPacket(
            pid=USB_PID["PING"],
            address=address,
            endpoint=2,
        )
    )
    session.add_event(RxHandshakePacket(pid=USB_PID["NAK"]))

    # And again
    session.add_event(
        TokenPacket(
            pid=USB_PID["PING"],
            address=address,
            endpoint=2,
        )
    )
    session.add_event(RxHandshakePacket(pid=USB_PID["NAK"]))

    # Send a packet to EP 1 so the DUT knows it can exit.
    session.add_event(
        UsbTransaction(
            session,
            deviceAddress=address,
            endpointNumber=ep,
            endpointType="BULK",
            direction="OUT",
            dataLength=10,
        )
    )

    return session
