#!/usr/bin/env python3

import sys
import os

# Pico Flash size = 2MB
FLASH_SIZE = 2 * 1024 * 1024

def main():
    if len(sys.argv) != 4:
        print(f"Incorrect number of arguments: {len(sys.argv)}")
        print(f"Syntax: {sys.argv[0]} <input_binary> <disk_image> <output_binary")
        sys.exit(1)

    input_binary = f"{sys.argv[1]}"
    disk_image = os.path.join(os.path.dirname(os.path.abspath(__file__)), sys.argv[2])
    output_binary = f"{sys.argv[3]}"

    # Read compiled (uncompressed) .bin file in a read/write byte array
    with open(input_binary, "rb") as f:
        firmware = bytearray(f.read())

    # Check size of input binary (needs to be exactly 2MB)
    if len(firmware) < FLASH_SIZE:
        firmware += b'\x00' * (FLASH_SIZE - len(firmware))
    elif len(firmware) > FLASH_SIZE:
        raise ValueError("Error: Binary image beyond Pico Flash Spaice")

    # Read disk image file in a read only byte array
    with open(disk_image, "rb") as f:
        disk = bytes(f.read())

    disk_size = len(disk)

    if disk_size > FLASH_SIZE:
        raise ValueError("Disk image is bigger than flash space!")
    
    disk_offset = FLASH_SIZE - disk_size
    
    # Overwrite the disk image on the flash memory from the end
    firmware[disk_offset:] = disk

    # Write new binary file: code + disk image 
    with open(output_binary, "wb") as f:
        f.write(firmware)

    print(f"[OK] {output_binary} generated successfully {disk_size} bytes")

if __name__ == "__main__":
    main()