#include "msc_disk.h"
#include "hardware/flash.h"
#include "tusb.h"

// Offset i mida de la partició FAT a la flash
#define FLASH_DISK_OFFSET (PICO_FLASH_SIZE_BYTES - 128 * 1024)
#define FLASH_SECTOR_SIZE 512
#define FLASH_DISK_SIZE   (128 * 1024)
#define MSC_BLOCK_COUNT   (FLASH_DISK_SIZE / FLASH_SECTOR_SIZE)

#define CFG_TUD_ENDPOINT0_SIZE 64

static uint8_t flash_sector_buf[FLASH_SECTOR_SIZE];

// Iniciació (si cal)
void msc_init() {
    // No fem res per ara. Es pot afegir logging si vols.
}

// Requerit per TinyUSB MSC: nombre de blocs
extern "C" void tud_msc_inquiry_cb(uint8_t lun, uint8_t vendor_id[8],
                                           uint8_t product_id[16], uint8_t product_rev[4]) {
    (void) lun;
    memcpy(vendor_id, "RP2040  ", 8);
    memcpy(product_id, "FLASH DISK     ", 16);
    memcpy(product_rev, "1.0 ", 4);
}

extern "C" bool tud_msc_test_unit_ready_cb(uint8_t) {
    return true;
}

extern "C" void tud_msc_capacity_cb(uint8_t, uint32_t* block_count, uint16_t* block_size) {
    *block_count = MSC_BLOCK_COUNT;
    *block_size  = FLASH_SECTOR_SIZE;
}

extern "C" bool tud_msc_start_stop_cb(uint8_t, uint8_t, bool, bool) {
    return true;
}

// Lectura d'un sector
extern "C" int32_t tud_msc_read10_cb(uint8_t, uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize) {
    if (lba >= MSC_BLOCK_COUNT) return -1;

    uint32_t addr = FLASH_DISK_OFFSET + (lba * FLASH_SECTOR_SIZE) + offset;
    memcpy(buffer, (const void*)(XIP_BASE + addr), bufsize);
    return (int32_t)bufsize;
}

// Escriptura d'un sector
extern "C" int32_t tud_msc_write10_cb(uint8_t, uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize) {
    if (lba >= MSC_BLOCK_COUNT) return -1;

    uint32_t addr = FLASH_DISK_OFFSET + (lba * FLASH_SECTOR_SIZE) + offset;

    //  Has de llegir el sector complet abans d'esborrar-lo
    uint32_t sector_base = FLASH_DISK_OFFSET + (lba * FLASH_SECTOR_SIZE);
    memcpy(flash_sector_buf, (const void*)(XIP_BASE + sector_base), FLASH_SECTOR_SIZE);
    memcpy(flash_sector_buf + offset, buffer, bufsize);

    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(sector_base, FLASH_SECTOR_SIZE);
    flash_range_program(sector_base, flash_sector_buf, FLASH_SECTOR_SIZE);
    restore_interrupts(ints);

    return (int32_t)bufsize;
}

extern "C" void tud_msc_write10_complete_cb(uint8_t) {
    // No fem res després d’escriure
}

extern "C" bool tud_msc_is_writable_cb(uint8_t) {
    return true;
}

extern "C" int32_t tud_msc_scsi_cb(uint8_t lun, uint8_t const scsi_cmd[16],
                                   void* buffer, uint16_t bufsize) {
    // Per defecte, TinyUSB pot gestionar algunes comandes, i aquí simplement diem que no gestionem cap extra
    return -1;  // -1 = no implementada, TinyUSB provarà built-in handler
}

extern "C" uint8_t const * tud_descriptor_device_cb(void) {
    static const tusb_desc_device_t desc = {
        .bLength            = sizeof(tusb_desc_device_t),
        .bDescriptorType    = TUSB_DESC_DEVICE,
        .bcdUSB             = 0x0200,
        .bDeviceClass       = TUSB_CLASS_MISC,
        .bDeviceSubClass    = MISC_SUBCLASS_COMMON,
        .bDeviceProtocol    = MISC_PROTOCOL_IAD,
        .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
        .idVendor           = 0xCafe,
        .idProduct          = 0x4000,
        .bcdDevice          = 0x0100,
        .iManufacturer      = 0x01,
        .iProduct           = 0x02,
        .iSerialNumber      = 0x03,
        .bNumConfigurations = 1
    };
    return (uint8_t const *) &desc;
}

#define EPNUM_MSC_OUT 0x01
#define EPNUM_MSC_IN  0x81
#define CONFIG_TOTAL_LEN    (TUD_CONFIG_DESC_LEN + TUD_MSC_DESC_LEN)

extern "C" uint8_t const * tud_descriptor_configuration_cb(uint8_t index) {
    (void) index;
    static uint8_t desc_cfg[] = {
        TUD_CONFIG_DESCRIPTOR(1, 1, 0, CONFIG_TOTAL_LEN, 0x00, 100),
        TUD_MSC_DESCRIPTOR(0, 0, EPNUM_MSC_OUT, EPNUM_MSC_IN, 64)
    };
    return desc_cfg;
}

const char* string_desc_arr[] = {
    "PicoCorp",        // index = 1
    "PicoFlashDrive",  // index = 2
    "123456"           // index = 3
};

extern "C" uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    static uint16_t desc[32];
    uint8_t chr_count;

    if (index == 0) {
        // IDIOMA: només cal retornar 0x0409 = anglès (US)
        desc[1] = 0x0409;
        desc[0] = (TUSB_DESC_STRING << 8) | (2 + 2); // 2 bytes per IDIOMA
        return desc;
    }

    index--;  // desplaça perquè string_desc_arr comença a l'índex 0

    if (index >= sizeof(string_desc_arr)/sizeof(string_desc_arr[0])) return NULL;

    const char* str = string_desc_arr[index];
    chr_count = strlen(str);
    if (chr_count > 31) chr_count = 31;

    for (uint8_t i = 0; i < chr_count; ++i)
        desc[1 + i] = str[i];

    desc[0] = (TUSB_DESC_STRING << 8) | (2 * chr_count + 2);
    return desc;
}

