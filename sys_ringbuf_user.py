#!/usr/bin/env python3
"""
Userspace client for the sys_ringbuf kernel module.

Requires the module loaded and /dev/sys_ringbuf present:
  make && sudo insmod sys_ringbuf.ko

Examples:
  python3 sys_ringbuf_user.py -w "hello"
  python3 sys_ringbuf_user.py -r
  python3 sys_ringbuf_user.py -rt "hello"
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


def _message_bytes(words: list[str] | None, flag: str) -> bytes:
    if not words:
        print(f"error: {flag} requires a message", file=sys.stderr)
        sys.exit(2)
    return " ".join(words).encode()


def main() -> None:
    parser = argparse.ArgumentParser(
        description="sys_ringbuf userspace client",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="examples:\n"
        '  %(prog)s -w "hello"\n'
        "  %(prog)s -r\n"
        '  %(prog)s -rt "hello"',
    )
    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument(
        "-r", "--read",
        action="store_true",
        help="read from the driver buffer",
    )
    group.add_argument(
        "-w", "--write",
        nargs="+",
        metavar="MSG",
        help="write a message to the driver buffer (max 512 bytes)",
    )
    group.add_argument(
        "-rt", "--roundtrip",
        nargs="+",
        metavar="MSG",
        help="write then read on one file descriptor",
    )

    args = parser.parse_args()

    if args.read:
        cmd_read()
    elif args.write is not None:
        cmd_write(_message_bytes(args.write, "-w"))
    else:
        cmd_roundtrip(_message_bytes(args.roundtrip, "-rt"))


if __name__ == "__main__":
    main()
