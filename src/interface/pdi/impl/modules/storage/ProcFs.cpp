/********************************** ProcFS *************************************
This file is part of the PDI Stack.

This is free software. You can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
Created Date    : 21st July 2026
******************************************************************************/

#include <config/Config.h>

#ifdef ENABLE_PROCFS

#include "ProcFs.h"
#include <interface/pdi.h>
#include <utility/DataTypeConversions.h>
#include <utility/TaskRecord.h>

#ifdef ENABLE_NETWORK_SERVICE
#include <interface/pdi/impl/modules/netif/NetifRegistry.h>
#endif

namespace {

class ProcFsNullStorage : public iStorageInterface {
public:
    int64_t read(uint64_t, void*, uint64_t) override { return PDI_ERR_NOT_SUPPORTED; }
    int64_t write(uint64_t, const void*, uint64_t) override { return PDI_ERR_NOT_SUPPORTED; }
    bool erase(uint64_t, uint64_t) override { return false; }
    uint64_t size() const override { return 0; }
};

ProcFsNullStorage s_proc_null_storage;

// Plain string literals at namespace scope — RODT_ATTR expands to a GCC
// statement-expression via F()/PSTR() on esp8266, which is only legal inside a
// function body. String literals here already live in RODATA/IROM.
const char* const s_proc_files[] = {
    "uptime",
    "version",
    "meminfo",
    "mounts",
    "stat"
};

const uint8_t s_proc_file_count = sizeof(s_proc_files) / sizeof(s_proc_files[0]);

const char* const s_proc_task_files[] = {
    "stat",
    "cmdline",
    "status"
};

const uint8_t s_proc_task_file_count = sizeof(s_proc_task_files) / sizeof(s_proc_task_files[0]);

#ifdef ENABLE_NETWORK_SERVICE
const char* const s_proc_net_files[] = {
    "route",
    "dev"
};

const uint8_t s_proc_net_file_count = sizeof(s_proc_net_files) / sizeof(s_proc_net_files[0]);
#endif

/**
 * Append a decimal number, which every field of a task node is built from.
 */
void appendNumber(pdiutil::string& out, int64_t value) {
    char buf[24];
    Int64ToString(value, buf, sizeof(buf), 0);
    out += buf;
}

/**
 * Append a keyed byte count, the shape every memory line is built from.
 */
void appendMemLine(pdiutil::string& out, const char* key, uint32_t bytes) {
    out += CHARPTR_WRAP_RO(key);
    out += ":\t";
    appendNumber(out, (int64_t)bytes);
    out += " B\n";
}

}

ProcFs __i_procfs;

ProcFs::ProcFs() : SynthFs(s_proc_null_storage, PROC_MOUNT_PREFIX) {}

/**
 * Which node the path names, and for a task node which task it belongs to.
 */
ProcFs::proc_node_t ProcFs::classify(const char* path, int32_t& taskid_out,
                                     uint8_t& leaf_out) const {
    const char* p = normalizePath(path);
    taskid_out = -1;
    leaf_out = 0;

    if (*p == '\0') return PROC_ROOT;

    for (uint8_t i = 0; i < s_proc_file_count; ++i) {
        const char* cursor = p;
        if (matchSegment(cursor, s_proc_files[i]) && *cursor == '\0') {
            return PROC_TOPFILE;
        }
    }

#ifdef ENABLE_NETWORK_SERVICE
    {
        const char* cursor = p;
        if (matchSegment(cursor, "net")) {
            if (*cursor == '\0') return PROC_NETDIR;
            if (!nextSegment(cursor)) return PROC_INVALID;

            for (uint8_t i = 0; i < s_proc_net_file_count; ++i) {
                const char* leafcursor = cursor;
                if (matchSegment(leafcursor, s_proc_net_files[i]) && *leafcursor == '\0') {
                    leaf_out = i;
                    return PROC_NETFILE;
                }
            }

            return PROC_INVALID;
        }
    }
#endif

    int32_t id = numberSegment(p);
    if (id < 0 || nullptr == __task_scheduler.get_task((pdiutil::task_id_t)id)) {
        return PROC_INVALID;
    }
    taskid_out = id;

    if (*p == '\0') return PROC_TASKDIR;
    if (!nextSegment(p)) return PROC_INVALID;

    for (uint8_t i = 0; i < s_proc_task_file_count; ++i) {
        const char* cursor = p;
        if (matchSegment(cursor, s_proc_task_files[i]) && *cursor == '\0') {
            leaf_out = i;
            return PROC_TASKFILE;
        }
    }

    return PROC_INVALID;
}

SynthFs::synth_node_t ProcFs::resolve(const char* path) {
    int32_t taskid;
    uint8_t leaf;

    switch (classify(path, taskid, leaf)) {
        case PROC_ROOT:
        case PROC_TASKDIR:
        case PROC_NETDIR:
            return SYNTH_DIR;
        case PROC_TOPFILE:
        case PROC_TASKFILE:
        case PROC_NETFILE:
            return SYNTH_FILE;
        default:
            return SYNTH_NONE;
    }
}

pdiutil::string ProcFs::render(const char* path) {
    int32_t taskid;
    uint8_t leaf;
    proc_node_t node = classify(path, taskid, leaf);

    if (PROC_TOPFILE == node) {
        const char* norm = normalizePath(path);
        char buf[128];
        buf[0] = '\0';

        if (strcmp_ro(norm, RODT_ATTR("uptime")) == 0) {
            uint32_t ms = __i_dvc_ctrl.millis_now();
            uint32_t sec = ms / 1000UL;
            uint32_t frac = (ms % 1000UL) / 10;
            pdiutil::string fmt = CHARPTR_WRAP("%u.%02u %u.%02u\n");
            __snprintf(buf, sizeof(buf), fmt.c_str(), sec, frac, sec, frac);
            return pdiutil::string(buf);
        }

        if (strcmp_ro(norm, RODT_ATTR("meminfo")) == 0) {
            return renderMemInfo();
        }

        if (strcmp_ro(norm, RODT_ATTR("mounts")) == 0) {
            return renderMounts();
        }

        if (strcmp_ro(norm, RODT_ATTR("stat")) == 0) {
            return renderStat();
        }

        // RELEASE / CONFIG_VERSION land in IROM on esp8266; __vsnprintf's %s
        // reads char-by-char with plain *p and faults there. Marshal to RAM
        // first via CHARPTR_WRAP (same trick already used for format strings).
        pdiutil::string fmt = CHARPTR_WRAP("PDI Stack version %s (%s)\n");
        pdiutil::string rel = CHARPTR_WRAP(RELEASE);
        pdiutil::string cfg = CHARPTR_WRAP(CONFIG_VERSION);
        __snprintf(buf, sizeof(buf), fmt.c_str(), rel.c_str(), cfg.c_str());
        return pdiutil::string(buf);
    }

#ifdef ENABLE_NETWORK_SERVICE
    if (PROC_NETFILE == node) {
        return (0 == leaf) ? renderNetRoute() : renderNetDev();
    }
#endif

    if (PROC_TASKFILE != node) return pdiutil::string();

    task_t* task = __task_scheduler.get_task((pdiutil::task_id_t)taskid);
    if (nullptr == task) return pdiutil::string();

    pdiutil::string out;

    if (1 == leaf) {
        out += taskDisplayName(task);
        out += "\n";
        return out;
    }

    pdiutil::string line;
    taskStatLine(task, line);

    if (0 == leaf) {
        return line;
    }

    pdiutil::string state, policy, mode;
    taskStatField(line, 2, state);
    taskStatField(line, 10, policy);
    taskStatField(line, 11, mode);

    out += CHARPTR_WRAP("Name:\t");
    out += taskDisplayName(task);
    out += CHARPTR_WRAP("\nPid:\t");
    appendNumber(out, task->m_task_id);
    out += CHARPTR_WRAP("\nState:\t");
    out += state;
    out += CHARPTR_WRAP("\nOwner:\t");
    appendNumber(out, task->m_owner);
    out += CHARPTR_WRAP("\nPrio:\t");
    appendNumber(out, task->m_task_priority);
    out += CHARPTR_WRAP("\nNice:\t");
    appendNumber(out, task->m_nice);
    out += CHARPTR_WRAP("\nPolicy:\t");
    out += policy;
    out += CHARPTR_WRAP("\nMode:\t");
    out += mode;
    out += CHARPTR_WRAP("\nRuns:\t");
    appendNumber(out, (int64_t)task->m_run_count);
    out += CHARPTR_WRAP("\nExecUs:\t");
    appendNumber(out, (int64_t)task->m_total_exec_us);
    out += CHARPTR_WRAP("\nIntvlMs:\t");
    appendNumber(out, (int64_t)task->m_duration);
    out += "\n";

    return out;
}

/**
 * What the heap holds, and how much of it is in one piece. The gap between the
 * two is the fragmentation a long running board suffers from.
 */
pdiutil::string ProcFs::renderMemInfo() {
    pdiutil::string out;
    appendMemLine(out, RODT_ATTR("MemFree"), __i_dvc_ctrl.get_free_heap());
    appendMemLine(out, RODT_ATTR("MemMaxBlock"), __i_dvc_ctrl.get_max_free_block());
    return out;
}

/**
 * One line per mount, in the order the dispatcher searches them.
 */
pdiutil::string ProcFs::renderMounts() {
    pdiutil::string out;

    for (uint8_t i = 0; i < __i_fs.getMountCount(); ++i) {
        const vfs_mount_t* mount = __i_fs.getMount(i);
        if (nullptr == mount) continue;

        out += mount->m_name;
        out += " ";
        out += mount->m_prefix;
        out += " ";
        out += CHARPTR_WRAP_RO(VfsTypeToString(mount->m_type));
        out += CHARPTR_WRAP(" rw 0 0\n");
    }

    return out;
}

/**
 * Cumulative scheduler counters. The cpu figures are microseconds rather than
 * the jiffies linux counts in, because every reader turns them into a ratio.
 */
pdiutil::string ProcFs::renderStat() {
    uint64_t busy = 0;
    uint16_t running = 0;
    uint32_t switches = 0;
    uint16_t tasks = 0;

    for (uint16_t i = 0; i < __task_scheduler.getTaskSlots(); ++i) {
        task_t* task = __task_scheduler.getTaskByIndex(i);
        if (nullptr == task) continue;
        busy += task->m_total_exec_us;
        switches += task->m_run_count;
        tasks++;
        if (TASK_STATE_RUNNING == task->m_state) running++;
    }

    uint64_t elapsed = (uint64_t)__i_dvc_ctrl.micros_now();
    uint64_t idle = (elapsed > busy) ? (elapsed - busy) : 0;

    pdiutil::string out = CHARPTR_WRAP("cpu 0 0 ");
    appendNumber(out, (int64_t)busy);
    out += " ";
    appendNumber(out, (int64_t)idle);
    out += CHARPTR_WRAP("\nctxt ");
    appendNumber(out, (int64_t)switches);
    out += CHARPTR_WRAP("\nprocesses ");
    appendNumber(out, (int64_t)tasks);
    out += CHARPTR_WRAP("\nprocs_running ");
    appendNumber(out, (int64_t)running);
    out += CHARPTR_WRAP("\nbtime 0\n");

    return out;
}

#ifdef ENABLE_NETWORK_SERVICE
/**
 * The gateway each registered interface routes through.
 */
pdiutil::string ProcFs::renderNetRoute() {
    pdiutil::string out = CHARPTR_WRAP("Iface\tDestination\tGateway\tMask\n");

    for (uint8_t i = 0; i < __netif_registry.count(); ++i) {
        iNetifInterface* netif = __netif_registry.at(i);
        if (nullptr == netif) continue;

        netif_info_t info;
        if (!netif->getInfo(info) || !info.m_up) continue;

        out += CHARPTR_WRAP_RO(netif->name());
        out += CHARPTR_WRAP("\t0.0.0.0\t");
        out += pdiutil::string(info.m_gateway);
        out += "\t";
        out += pdiutil::string(info.m_netmask);
        out += "\n";
    }

    return out;
}

/**
 * Per interface traffic, for the interfaces that can count it.
 */
pdiutil::string ProcFs::renderNetDev() {
    pdiutil::string out = CHARPTR_WRAP("Iface\tRxBytes\tRxPackets\tRxErrs\tTxBytes\tTxPackets\tTxErrs\n");

    for (uint8_t i = 0; i < __netif_registry.count(); ++i) {
        iNetifInterface* netif = __netif_registry.at(i);
        if (nullptr == netif) continue;

        // an interface that cannot count is left out rather than listed with
        // zeroes, which would read as an idle link
        netif_counters_t counters;
        if (!netif->getCounters(counters)) continue;

        out += CHARPTR_WRAP_RO(netif->name());
        out += "\t";
        appendNumber(out, (int64_t)counters.m_rx_bytes);
        out += "\t";
        appendNumber(out, (int64_t)counters.m_rx_packets);
        out += "\t";
        appendNumber(out, (int64_t)counters.m_rx_errors);
        out += "\t";
        appendNumber(out, (int64_t)counters.m_tx_bytes);
        out += "\t";
        appendNumber(out, (int64_t)counters.m_tx_packets);
        out += "\t";
        appendNumber(out, (int64_t)counters.m_tx_errors);
        out += "\n";
    }

    return out;
}
#endif

int ProcFs::listChildren(const char* path, pdiutil::vector<file_info_t>& items) {
    int32_t taskid;
    uint8_t leaf;
    proc_node_t node = classify(path, taskid, leaf);

    if (PROC_TASKDIR == node) {
        for (uint8_t i = 0; i < s_proc_task_file_count; ++i) {
            addEntry(items, s_proc_task_files[i], FILE_TYPE_REG, 0444);
        }
        return (int)items.size();
    }

#ifdef ENABLE_NETWORK_SERVICE
    if (PROC_NETDIR == node) {
        for (uint8_t i = 0; i < s_proc_net_file_count; ++i) {
            addEntry(items, s_proc_net_files[i], FILE_TYPE_REG, 0444);
        }
        return (int)items.size();
    }
#endif

    if (PROC_ROOT != node) return STORAGE_ERROR_NOT_A_DIRECTORY;

    for (uint8_t i = 0; i < s_proc_file_count; ++i) {
        addEntry(items, s_proc_files[i], FILE_TYPE_REG, 0444,
                 getFileSize(s_proc_files[i]));
    }

#ifdef ENABLE_NETWORK_SERVICE
    addEntry(items, "net", FILE_TYPE_DIR, 0555);
#endif

    char numbuf[12];
    for (uint16_t i = 0; i < __task_scheduler.getTaskSlots(); ++i) {
        task_t* task = __task_scheduler.getTaskByIndex(i);
        if (nullptr == task) continue;
        Uint32ToString((uint32_t)task->m_task_id, numbuf, sizeof(numbuf));
        addEntry(items, numbuf, FILE_TYPE_DIR, 0555);
    }

    return (int)items.size();
}

#endif
