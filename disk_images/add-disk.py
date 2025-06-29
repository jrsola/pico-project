#!/usr/bin/env python3

FLASH_SIZE = 2 * 1024 * 1024         # 2 MB
DISK_SIZE = 128 * 1024               # 128 KB
DISK_OFFSET = FLASH_SIZE - DISK_SIZE

INPUT_BIN = "build/pico-project.bin"
DISK_IMG = "disk_images/fat12_disk_128.img"
OUTPUT_BIN = "build/pico-project+disk.bin"

def main():
    with open(INPUT_BIN, "rb") as f:
        firmware = bytearray(f.read())

    with open(DISK_IMG, "rb") as f:
        disk = f.read()

    if len(disk) != DISK_SIZE:
        raise ValueError("La imatge FAT12 no fa exactament 128 KB")

    # Assegura que el binari fa exactament 2MB
    if len(firmware) < FLASH_SIZE:
        firmware += b'\x00' * (FLASH_SIZE - len(firmware))
    elif len(firmware) > FLASH_SIZE:
        raise ValueError("El firmware excedeix els 2 MB de la flash")

    # Substitueix els últims 128 KB amb la imatge FAT12
    firmware[DISK_OFFSET:] = disk

    # Escriu el nou fitxer
    with open(OUTPUT_BIN, "wb") as f:
        f.write(firmware)

    print(f"[OK] Generat {OUTPUT_BIN} amb la imatge FAT12 incrustada als últims 128 KB")

if __name__ == "__main__":
    main()
