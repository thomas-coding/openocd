/* SPDX-License-Identifier: GPL-2.0-or-later */

/***************************************************************************
 *   Copyright (C) 2009 Jinping Wu <wunekky@gmail.com>              *
 ***************************************************************************/

#ifndef OPENOCD_ALIUS_ROM_TABLE_H
#define OPENOCD_ALIUS_ROM_TABLE_H

#define ROM_TABLE_ENTRY_MAX_MUNBER 512
#define MAX_CHILD_COMPONENT 10
#define MAX_CHILD_INTERCONNECTION 10

enum coresight_component_type {
    UNKNOW = 1,
    ROM_TABLE = 2
};

struct coresight_component {
    char name[256];
    uint32_t base;
    enum coresight_component_type type;
};

struct rom_table {
    uint32_t entry[ROM_TABLE_ENTRY_MAX_MUNBER];
};

struct coresight_apb_interconnection {
    struct rom_table rom;
    struct coresight_component component[MAX_CHILD_COMPONENT];
    struct coresight_apb_interconnection *child[MAX_CHILD_INTERCONNECTION];
};

enum rom_table_state {
    ROM_TABLE_ENTRY_NOT_PRESENT_AND_LAST = 0,
    RESERVED = 1,
    ROM_TABLE_ENTRY_NOT_PRESENT = 2,
    ROM_TABLE_ENTRY_PRESENT = 3
};

enum rom_table_region {
    TOP = 0,
    AON = 1,
    LP = 2,
    HP = 3
};

enum target_class {
    TARGET_UNKNOW = 0,
    TARGET_ALIUS = 1
};

/* Coresight component address */
//top
#define TOP_BASE 0x0
#define TOP_HP_APBAP 0x00010000
#define TOP_LP_APBAP 0x00020000
#define TOP_AON_APBAP 0x00030000

//aon
#define AON_M33_AHBAP 0x00010000

//lp
#define LP_A32_APBAP 0x01000000

//hp
#define HP_A32_APBAP 0x02000000

#define DEV_ARCH_OFFSET 0x1FBC

#define ARCH_TYPE_ROM_TABLE 0x47700af7

extern const struct command_registration alius_rom_table_command_handlers[];
#endif /* OPENOCD_ALIUS_ROM_TABLE_H */
