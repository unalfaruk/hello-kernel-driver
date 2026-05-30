#!/usr/bin/env python3
"""
Userspace client for the sys_ringbuf kernel module.

Requires the module loaded and /dev/sys_ringbuf present:
  make && sudo insmod sys_ringbuf.ko
"""

import argparse
import os
import sys

DEVICE = "/dev/sys_ringbuf"
MAX_SIZE = 512


def open_device():
    if not os.path.exists(DEVICE):
        print(f"error: {DEVICE} not found (is the module loaded?)", file=sys.stderr)
        sys.exit(1)
    try:
        return os.open(DEVICE, os.O_RDWR)
    except PermissionError:
        print(f"error: permission denied opening {DEVICE} (try sudo)", file=sys.stderr)
        sys.exit(1)


def cmd_write(message: bytes) -> None:
    if len(message) > MAX_SIZE:
        print(f"warning: truncating to {MAX_SIZE} bytes", file=sys.stderr)
        message = message[:MAX_SIZE]

    fd = open_device()
    try:
        n = os.write(fd, message)
        print(f"wrote {n} bytes")
    finally:
        os.close(fd)


def cmd_read() -> None:
    fd = open_device()
    try:
        data = os.read(fd, MAX_SIZE)
        if not data:
            print("(empty — write something first, or buffer already read)")
            return
        print(f"read {len(data)} bytes:")
        try:
            print(data.decode())
        except UnicodeDecodeError:
            print(data)
    finally:
        os.close(fd)


def cmd_roundtrip(message: bytes) -> None:
    """Write then read on the same open file descriptor."""
    if len(message) > MAX_SIZE:
        message = message[:MAX_SIZE]

    fd = open_device()
    try:
        n = os.write(fd, message)
        print(f"wrote {n} bytes")
        data = os.read(fd, MAX_SIZE)
        print(f"read {len(data)} bytes:")
        try:
            print(data.decode())
        except UnicodeDecodeError:
            print(data)
    finally:
        os.close(fd)


def main() -> None:
    parser = argparse.ArgumentParser(description="sys_ringbuf userspace client")
    sub = parser.add_subparsers(dest="command", required=True)

    p_write = sub.add_parser("write", help="write a message to the driver buffer")
    p_write.add_argument("message", help="text to store (max 512 bytes)")

    sub.add_parser("read", help="read from the driver buffer")

    p_trip = sub.add_parser("roundtrip", help="write then read on one fd")
    p_trip.add_argument("message", help="text to store and read back")

    args = parser.parse_args()

    if args.command == "write":
        cmd_write(args.message.encode())
    elif args.command == "read":
        cmd_read()
    elif args.command == "roundtrip":
        cmd_roundtrip(args.message.encode())


if __name__ == "__main__":
    main()
