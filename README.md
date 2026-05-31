# sys_ringbuf

A minimal Linux **character device driver** plus a small **userspace** program (refresher for myself, a quick weekend hands-on work). The goal is learning: how a kernel module exposes `/dev/sys_ringbuf`, how `read`/`write` cross the kernel boundary, and how normal tools (or Python) use that device.

> **Note:** Despite the name, this is a simple **512-byte message buffer**, not a ring buffer yet. The name reflects where the project may go next.

## Kernel driver (basics)

The module is a teaching skeleton built in small steps:

1. **`file_operations`** — `open`, `release`, `read`, `write` callbacks the VFS invokes for `/dev/sys_ringbuf`.
2. **Char device registration** — `alloc_chrdev_region` + `cdev_add` (dynamic major/minor, hooked to your ops table).
3. **Device node** — `class_create` + `device_create` so `/dev/sys_ringbuf` appears (via devtmpfs/udev).
4. **Storage** — Global `storage[512]` and `storage_len`; `write` copies from userspace with `copy_from_user`, `read` copies out with `copy_to_user`.
5. **Synchronization** — `sys_ringbuf_storage_lock` mutex so concurrent readers/writers do not corrupt the buffer.

### Behavior (good to know)

- Each **`write` replaces** the whole buffer (up to 512 bytes; longer writes are truncated).
- **`read`** returns data from the current file offset (`*ppos`); further reads return `0` when exhausted (EOF).
- After a full read, you must **`write` again** before new data is available (or open a fresh fd and write).
- Kernel logs use `pr_info` — inspect with `dmesg`.

### Architecture (high level)

```
userspace                    kernel
─────────                    ──────
open/read/write/close  →     sys_ringbuf_fops
       │                          │
       └──────────────────►  storage[512] + mutex
```

## Prerequisites

- Linux with kernel **headers** for your running version, e.g. on Debian/Ubuntu:

  ```bash
  sudo apt install build-essential linux-headers-$(uname -r)
  ```

- `make`, `sudo` (for `insmod` / device access)
- Python 3 (optional, for the userspace script)

## Build and load

```bash
cd kernel-driver
make
sudo insmod sys_ringbuf.ko
```

Confirm:

```bash
ls -l /dev/sys_ringbuf
lsmod | grep sys_ringbuf
dmesg | tail
```

Unload:

```bash
sudo rmmod sys_ringbuf
make clean   # optional: remove .ko and build artifacts
```

If `insmod` says **“File exists”**, the module is already loaded — run `sudo rmmod sys_ringbuf` first.

## Interacting from the shell (`echo`, `cat`, …)

The device is a file. Any program that reads/writes file descriptors can use it.

### Write a message

```bash
echo -n "hello kernel" | sudo tee /dev/sys_ringbuf
# or (requires permission)
echo "hello kernel" >> /dev/sys_ringbuf
```

`-n` avoids appending a newline unless you want one.

### Read it back

```bash
sudo cat /dev/sys_ringbuf
# hello kernel
```

## Userspace app (Python)

Help: `python3 sys_ringbuf_user.py -h`

## Possible next steps

- Real **ring buffer** (wrap-around, separate read/write pointers)
- Reset read offset on **write**
- **`ioctl`** for extra commands
- **udev** rule so unprivileged users can access the device

## License

GPL (see `MODULE_LICENSE("GPL")` in `sys_ringbuf.c`). Use and modify for learning as you like.
