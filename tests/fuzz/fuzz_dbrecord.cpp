/**************************** Fuzz DB Record **********************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

The record reader mounts whatever the medium holds and believes the superblock
and the directory it finds there. A flash sector that went bad, a half finished
write or an image from another firmware all arrive looking like this.

Author          : Suraj I.
created Date    : 26th Aug 2026
******************************************************************************/

#include <FuzzCommon.h>

#include <database/core/DbLayout.h>

namespace
{

    /**
     * @brief A ram store whose bytes are the fuzz input, padded with erase.
     */
    class FuzzDbStore : public iDbStoreInterface
    {

    public:
        static const uint32_t SIZE = 4096;

        FuzzDbStore(const uint8_t *data, size_t size)
        {
            memset(m_bytes, 0xFF, SIZE);
            overlay(0, data, size);
        }

        /**
         * @brief Start from an image and write the fuzz bytes over part of it.
         */
        FuzzDbStore(const uint8_t *image, const uint8_t *data, size_t size, uint32_t at)
        {
            memcpy(m_bytes, image, SIZE);
            overlay(at, data, size);
        }

        /**
         * @brief The whole store, for taking a copy of a formatted image.
         */
        const uint8_t *bytes() const { return m_bytes; }

        bool read(uint32_t offset, uint8_t *buf, uint32_t len) override
        {
            if (offset + len > SIZE)
            {
                return false;
            }
            memcpy(buf, &m_bytes[offset], len);
            return true;
        }

        bool write(uint32_t offset, const uint8_t *buf, uint32_t len) override
        {
            if (offset + len > SIZE)
            {
                return false;
            }
            memcpy(&m_bytes[offset], buf, len);
            return true;
        }

        bool flush() override { return true; }

        uint32_t capacity() override { return SIZE; }

    private:
        void overlay(uint32_t at, const uint8_t *data, size_t size)
        {
            if (nullptr == data || at >= SIZE)
            {
                return;
            }
            uint32_t room = SIZE - at;
            uint32_t take = size < room ? (uint32_t)size : room;
            memcpy(&m_bytes[at], data, take);
        }

        uint8_t m_bytes[SIZE];
    };

    /**
     * @brief A table list the mount is asked to fit the found geometry to.
     */
    pdiutil::vector<struct_tables> fuzzTables(pdifuzz::FuzzInput &input)
    {
        pdiutil::vector<struct_tables> tables;
        uint8_t count = 1 + input.pick(4);

        for (uint8_t i = 0; i < count; i++)
        {
            struct_tables table;
            memset(&table, 0, sizeof(table));
            table.m_table_id = (uint16_t)(i + 1);
            table.m_table_size = (uint16_t)(8 + input.pick(64));
            table.m_table_version = 1;
            table.m_table_secret = 0 != input.pick(2);
            tables.push_back(table);
        }

        return tables;
    }

    /**
     * @brief An image the engine itself laid down, to corrupt rather than guess.
     *
     * Random bytes almost never carry a valid superblock, so a run made only of
     * them never reaches the directory reader. Overwriting part of a real image
     * is also the shape of the actual hazard: a sector that went bad under a
     * database that was sound.
     */
    const uint8_t *formattedImage()
    {
        static uint8_t image[FuzzDbStore::SIZE];
        static bool ready = false;

        if (!ready)
        {
            ready = true;

            pdiutil::vector<struct_tables> tables;
            for (uint8_t i = 0; i < 3; i++)
            {
                struct_tables table;
                memset(&table, 0, sizeof(table));
                table.m_table_id = (uint16_t)(i + 1);
                table.m_table_size = (uint16_t)(16 + (i * 16));
                table.m_table_version = 1;
                tables.push_back(table);
            }

            FuzzDbStore blank(nullptr, 0);
            DbLayout layout;

            if (PDI_OK == layout.mount(&blank, tables))
            {
                uint8_t content[48];
                memset(content, 0x5A, sizeof(content));
                for (uint8_t i = 0; i < tables.size(); i++)
                {
                    layout.write_record(tables[i].m_table_id, content,
                                        tables[i].m_table_size, 1);
                }
            }

            memcpy(image, blank.bytes(), FuzzDbStore::SIZE);
        }

        return image;
    }

} // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    pdifuzz::FuzzInput input(data, size);
    bool corrupt = 0 != input.pick(2);
    uint32_t at = (uint32_t)input.pick(64) * 64;
    pdiutil::vector<struct_tables> tables = fuzzTables(input);

    FuzzDbStore store = corrupt
                            ? FuzzDbStore(formattedImage(), input.rest(), input.remaining(), at)
                            : FuzzDbStore(input.rest(), input.remaining());
    DbLayout layout;

    if (PDI_OK != layout.mount(&store, tables))
    {
        return 0;
    }

    uint8_t buf[128];
    for (uint16_t id = 1; id <= tables.size() + 1; id++)
    {
        uint16_t len = 0;
        uint16_t version = 0;
        layout.read_record(id, buf, sizeof(buf), len, version);
        layout.verify_record(id);
    }

    return 0;
}
