/********************************** SysFS **************************************
This file is part of the PDI Stack.

This is free software. You can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
Created Date    : 23rd July 2026
******************************************************************************/

#include <config/Config.h>

#ifdef ENABLE_SYSFS

#include "SysFs.h"
#include <interface/pdi.h>
#include <utility/DataTypeConversions.h>
#include <config/GpioConfig.h>

#ifdef ENABLE_NETWORK_SERVICE
#include <interface/pdi/impl/modules/netif/NetifRegistry.h>
#endif

#ifdef ENABLE_GPIO_SERVICE
#include <service_provider/device/GpioServiceProvider.h>
#include <service_provider/database/DatabaseServiceProvider.h>
#endif

namespace {

class SysFsNullStorage : public iStorageInterface {
public:
    int64_t read(uint64_t, void*, uint64_t) override { return PDI_ERR_NOT_SUPPORTED; }
    int64_t write(uint64_t, const void*, uint64_t) override { return PDI_ERR_NOT_SUPPORTED; }
    bool erase(uint64_t, uint64_t) override { return false; }
    uint64_t size() const override { return 0; }
};

SysFsNullStorage s_sys_null_storage;

// Leaf node names under a pin directory. Plain literals at namespace scope
// (RODT_ATTR is only legal inside a function body on esp8266).
const char* const s_sys_leaf_names[] = { "value", "mode" };
const uint8_t s_sys_leaf_count = sizeof(s_sys_leaf_names) / sizeof(s_sys_leaf_names[0]);

#ifdef ENABLE_NETWORK_SERVICE
// Leaf node names under a network interface directory. An interface answers
// every one of them, saying down and 0.0.0.0 where it has nothing better.
const char* const s_sys_net_leaf_names[] = {
    "address", "operstate", "ip", "netmask", "gateway", "ssid", "rssi"
};
const uint8_t s_sys_net_leaf_count = sizeof(s_sys_net_leaf_names) / sizeof(s_sys_net_leaf_names[0]);

// a station reports the last two; an access point has no association to
// describe, so its directory stops before them
const uint8_t s_sys_net_common_leaf_count = 5;
#endif

}

#ifdef ENABLE_NETWORK_SERVICE
/**
 * How many leaves an interface's directory holds. Only a station has an
 * association to describe, so an access point's stops before ssid and rssi.
 */
static uint8_t netLeafCount(uint8_t index) {
    iNetifInterface* netif = __netif_registry.at(index);
    if (nullptr == netif) return 0;

    netif_info_t info;
    if (!netif->getInfo(info)) return s_sys_net_common_leaf_count;

    return (NETIF_KIND_WIFI_STA == info.m_kind) ? s_sys_net_leaf_count
                                                : s_sys_net_common_leaf_count;
}
#endif

SysFs __i_sysfs;

SysFs::SysFs() : SynthFs(s_sys_null_storage, SYS_MOUNT_PREFIX) {}

/**
 * Whether the pin exists on this board and is safe to drive.
 */
bool SysFs::isValidPin(uint8_t pin) const {
#ifdef ENABLE_GPIO_SERVICE
    return pin < MAX_GPIO_PINS && !__i_dvc_ctrl.isExceptionalGpio(pin);
#else
    (void)pin;
    return false;
#endif
}

/**
 * Which node the path names, plus the pin or interface index behind it and
 * which of that node's leaves it is.
 */
SysFs::NodeKind SysFs::classify(const char* path, int16_t& index_out,
                                uint8_t& leaf_out) const {
    const char* p = normalizePath(path);
    index_out = -1;
    leaf_out = 0;

    if (*p == '\0') return SYS_ROOT;
    if (!matchSegment(p, "class")) return SYS_INVALID;
    if (*p == '\0') return SYS_CLASS;
    if (!nextSegment(p)) return SYS_INVALID;

    if (matchSegment(p, "gpio")) {
        if (*p == '\0') return SYS_GPIODIR;
        if (!nextSegment(p)) return SYS_INVALID;

        int32_t pin = numberSegment(p);
        if (pin < 0 || !isValidPin((uint8_t)pin)) return SYS_INVALID;
        index_out = (int16_t)pin;
        if (*p == '\0') return SYS_PIN;
        if (!nextSegment(p)) return SYS_INVALID;

        for (uint8_t i = 0; i < s_sys_leaf_count; ++i) {
            const char* cursor = p;
            if (matchSegment(cursor, s_sys_leaf_names[i]) && *cursor == '\0') {
                leaf_out = i;
                return (0 == i) ? SYS_VALUE : SYS_MODE;
            }
        }

        return SYS_INVALID;
    }

#ifdef ENABLE_NETWORK_SERVICE
    if (matchSegment(p, "net")) {
        if (*p == '\0') return SYS_NETDIR;
        if (!nextSegment(p)) return SYS_INVALID;

        int16_t found = -1;
        for (uint8_t i = 0; i < __netif_registry.count(); ++i) {
            iNetifInterface* netif = __netif_registry.at(i);
            if (nullptr == netif) continue;
            const char* cursor = p;
            if (matchSegment(cursor, netif->name()) && (*cursor == '\0' || *cursor == '/')) {
                found = (int16_t)i;
                p = cursor;
                break;
            }
        }

        if (found < 0) return SYS_INVALID;
        index_out = found;
        if (*p == '\0') return SYS_NETIF;
        if (!nextSegment(p)) return SYS_INVALID;

        for (uint8_t i = 0; i < netLeafCount((uint8_t)found); ++i) {
            const char* cursor = p;
            if (matchSegment(cursor, s_sys_net_leaf_names[i]) && *cursor == '\0') {
                leaf_out = i;
                return SYS_NETATTR;
            }
        }

        return SYS_INVALID;
    }
#endif

    return SYS_INVALID;
}

SynthFs::synth_node_t SysFs::resolve(const char* path) {
    int16_t index;
    uint8_t leaf;

    switch (classify(path, index, leaf)) {
        case SYS_ROOT:
        case SYS_CLASS:
        case SYS_GPIODIR:
        case SYS_PIN:
        case SYS_NETDIR:
        case SYS_NETIF:
            return SYNTH_DIR;
        case SYS_VALUE:
        case SYS_MODE:
        case SYS_NETATTR:
            return SYNTH_FILE;
        default:
            return SYNTH_NONE;
    }
}

/**
 * The permission bits a node carries. Read-only trees keep the default.
 */
uint16_t SysFs::permsFor(const char* path, synth_node_t kind) {
    if (SYNTH_DIR == kind) return 0555;

    int16_t index;
    uint8_t leaf;
    return (SYS_NETATTR == classify(path, index, leaf)) ? 0444 : 0666;
}

pdiutil::string SysFs::render(const char* path) {
    int16_t index;
    uint8_t leaf;
    NodeKind k = classify(path, index, leaf);

#ifdef ENABLE_NETWORK_SERVICE
    if (SYS_NETATTR == k) {
        return renderNetAttr((uint8_t)index, leaf);
    }
#endif

#ifdef ENABLE_GPIO_SERVICE
    if (k != SYS_VALUE && k != SYS_MODE) return pdiutil::string();

    uint32_t v = (k == SYS_VALUE)
                     ? (uint32_t)__gpio_service.m_gpio_config_copy.gpio_readings[index]
                     : (uint32_t)__gpio_service.m_gpio_config_copy.gpio_mode[index];

    char buf[12];
    Uint32ToString(v, buf, sizeof(buf));
    pdiutil::string out(buf);
    out += TERMINAL_NEW_LINE;
    return out;
#else
    return pdiutil::string();
#endif
}

int SysFs::writeFile(const char* path, const char* content, uint32_t size, bool append) {
#ifdef ENABLE_GPIO_SERVICE
    int16_t pin;
    uint8_t leaf;
    NodeKind k = classify(path, pin, leaf);
    if (!content) return PDI_ERR_NULL_PTR;
    if (k != SYS_VALUE && k != SYS_MODE) return STORAGE_ERROR_READ_ONLY;

    uint16_t val = StringToUint16(content, (uint8_t)(size > 255 ? 255 : size));

    if (k == SYS_MODE) {
        if (val >= GPIO_MODE_MAX) return PDI_ERR_RANGE;
        __gpio_service.m_gpio_config_copy.gpio_mode[pin] = (uint8_t)val;
    } else {
        __gpio_service.m_gpio_config_copy.gpio_readings[pin] = val;
    }

    __database_service.set_gpio_config_table(&__gpio_service.m_gpio_config_copy);
    __gpio_service.handleGpioModes(GPIO_WRITE_CONFIG);
    return (int)size;
#else
    (void)path; (void)content; (void)size; (void)append;
    return PDI_ERR_NOT_SUPPORTED;
#endif
}

int SysFs::listChildren(const char* path, pdiutil::vector<file_info_t>& items) {
    int16_t index;
    uint8_t leaf;
    NodeKind k = classify(path, index, leaf);

    if (k == SYS_ROOT) {
        addEntry(items, "class", FILE_TYPE_DIR, 0555);
    } else if (k == SYS_CLASS) {
        addEntry(items, "gpio", FILE_TYPE_DIR, 0555);
#ifdef ENABLE_NETWORK_SERVICE
        addEntry(items, "net", FILE_TYPE_DIR, 0555);
#endif
#ifdef ENABLE_NETWORK_SERVICE
    } else if (k == SYS_NETDIR) {
        for (uint8_t i = 0; i < __netif_registry.count(); ++i) {
            iNetifInterface* netif = __netif_registry.at(i);
            if (nullptr == netif) continue;
            addEntry(items, netif->name(), FILE_TYPE_DIR, 0555);
        }
    } else if (k == SYS_NETIF) {
        for (uint8_t i = 0; i < netLeafCount((uint8_t)index); ++i) {
            addEntry(items, s_sys_net_leaf_names[i], FILE_TYPE_REG, 0444);
        }
#endif
    } else if (k == SYS_GPIODIR) {
        char numbuf[6];
        for (uint8_t i = 0; i < MAX_GPIO_PINS; ++i) {
            if (!isValidPin(i)) continue;
            Uint32ToString(i, numbuf, sizeof(numbuf));
            addEntry(items, numbuf, FILE_TYPE_DIR, 0555);
        }
    } else if (k == SYS_PIN) {
        for (uint8_t i = 0; i < s_sys_leaf_count; ++i) {
            addEntry(items, s_sys_leaf_names[i], FILE_TYPE_REG, 0666);
        }
    } else {
        return STORAGE_ERROR_NOT_A_DIRECTORY;
    }

    return (int)items.size();
}

#ifdef ENABLE_NETWORK_SERVICE
/**
 * What one leaf of a registered network interface currently reads.
 */
pdiutil::string SysFs::renderNetAttr(uint8_t index, uint8_t leaf) {
    iNetifInterface* netif = __netif_registry.at(index);
    if (nullptr == netif) return pdiutil::string();

    netif_info_t info;
    if (!netif->getInfo(info)) return pdiutil::string();

    pdiutil::string out;

    switch (leaf) {
        case 0:
            out = pdiutil::string(info.m_mac);
            break;
        case 1:
            out = info.m_up ? CHARPTR_WRAP("up") : CHARPTR_WRAP("down");
            break;
        case 2:
            out = pdiutil::string(info.m_ip);
            break;
        case 3:
            out = pdiutil::string(info.m_netmask);
            break;
        case 4:
            out = pdiutil::string(info.m_gateway);
            break;
        case 5:
            out = info.m_ssid;
            break;
        default: {
            char buf[12];
            Int32ToString(info.m_rssi, buf, sizeof(buf), 0);
            out = pdiutil::string(buf);
            break;
        }
    }

    // a leaf that has nothing to say still reads as a line, so a reader can
    // tell an empty value from a node that is not there
    out += TERMINAL_NEW_LINE;
    return out;
}
#endif

#endif
