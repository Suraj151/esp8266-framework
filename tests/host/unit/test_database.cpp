/****************************** Database Tests ********************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 16th Aug 2026
******************************************************************************/

#include <pditest.h>
#include <utility/Database.h>

/**
 * A table that records whether the engine booted and cleared it.
 *
 * DatabaseTableAbstractLayer registers itself into a static list on
 * construction and never leaves it, so instances have to outlive every test the
 * way the framework's own tables are globals.
 */
struct CountingTable : public DatabaseTableAbstractLayer
{
    int boots;
    int clears;

    CountingTable() : boots(0), clears(0) {}

    bool boot() override { boots++; return true; }
    bool clear() override { clears++; return true; }

    void forget()
    {
        boots = 0;
        clears = 0;
    }
};

static CountingTable s_table_one;
static CountingTable s_table_two;

static struct_tables makeTable(uint16_t id, uint16_t size,
                               DatabaseTableAbstractLayer *instance = nullptr,
                               uint16_t version = 1)
{
    struct_tables table;
    memset(&table, 0, sizeof(table));
    table.m_table_id = id;
    table.m_table_size = size;
    table.m_table_version = version;
    table.m_instance = instance;
    return table;
}

TEST(database, starts_with_no_tables)
{
    Database db;
    ASSERT_EQ(db.m_database_tables.size(), (size_t)0);
}

TEST(database, registers_a_table)
{
    Database db;

    struct_tables table = makeTable(1, 50);
    ASSERT_TRUE(db.register_table(table));
    ASSERT_EQ(db.m_database_tables.size(), (size_t)1);
}

TEST(database, refuses_a_table_with_no_id)
{
    Database db;

    struct_tables table = makeTable(DB_TABLE_ID_NONE, 50);
    ASSERT_FALSE(db.register_table(table));
}

TEST(database, refuses_a_table_with_no_size)
{
    Database db;

    struct_tables table = makeTable(1, 0);
    ASSERT_FALSE(db.register_table(table));
}

/**
 * Two tables sharing an id would share a record, so the second one is refused
 * rather than quietly overwriting the first.
 */
TEST(database, refuses_a_second_table_claiming_the_same_id)
{
    Database db;

    struct_tables first = makeTable(4, 50);
    struct_tables duplicate = makeTable(4, 80);

    ASSERT_TRUE(db.register_table(first));
    ASSERT_FALSE(db.register_table(duplicate));
    ASSERT_EQ(db.m_database_tables.size(), (size_t)1);
}

/**
 * Ids are what identify a table now, so the order tables boot in carries no
 * meaning at all and every one of them still reaches the registry.
 */
TEST(database, registers_every_table_whatever_order_ids_arrive_in)
{
    Database db;

    const uint16_t ids[] = {8, 2, 9, 4, 1, 5, 6, 7, 3};

    for (uint8_t i = 0; i < (sizeof(ids) / sizeof(ids[0])); i++)
    {
        struct_tables table = makeTable(ids[i], 40);
        ASSERT_TRUE(db.register_table(table));
    }

    ASSERT_EQ(db.m_database_tables.size(), (size_t)9);
}

TEST(database, refuses_tables_past_the_registry_limit)
{
    Database db;

    for (uint16_t i = 0; i < MAX_TABLES; i++)
    {
        struct_tables table = makeTable((uint16_t)(i + 1), 50);
        ASSERT_TRUE(db.register_table(table));
    }

    struct_tables overflowing = makeTable((uint16_t)(MAX_TABLES + 1), 50);
    ASSERT_FALSE(db.register_table(overflowing));
    ASSERT_EQ(db.m_database_tables.size(), (size_t)MAX_TABLES);
}

TEST(database, clear_all_reaches_every_registered_instance)
{
    Database db;
    s_table_one.forget();
    s_table_two.forget();

    struct_tables first = makeTable(1, 50, &s_table_one);
    struct_tables second = makeTable(2, 50, &s_table_two);

    db.register_table(first);
    db.register_table(second);
    ASSERT_TRUE(db.clear_all());

    ASSERT_EQ(s_table_one.clears, 1);
    ASSERT_EQ(s_table_two.clears, 1);
}

TEST(database, clear_all_skips_a_table_with_no_instance)
{
    Database db;
    s_table_one.forget();

    struct_tables headless = makeTable(1, 50, nullptr);
    struct_tables attached = makeTable(2, 50, &s_table_one);

    db.register_table(headless);
    db.register_table(attached);
    db.clear_all();

    ASSERT_EQ(s_table_one.clears, 1);
}

TEST(database, init_boots_every_registered_table_instance)
{
    Database db;
    s_table_one.forget();
    s_table_two.forget();

    ASSERT_EQ(db.init_database(), (uint8_t)0);

    ASSERT_EQ(s_table_one.boots, 1);
    ASSERT_EQ(s_table_two.boots, 1);
}

/**
 * A table that cannot register is counted, because its configs will not
 * persist and the service layer has to be able to say so.
 */
TEST(database, init_counts_a_table_that_refuses_to_boot)
{
    struct RefusingTable : public DatabaseTableAbstractLayer
    {
        bool boot() override { return false; }
        bool clear() override { return true; }
    };

    static RefusingTable refusing;
    Database db;

    ASSERT_GE(db.init_database(), (uint8_t)1);
}

TEST(database, clear_all_on_an_empty_database_is_harmless)
{
    Database db;
    ASSERT_TRUE(db.clear_all());
    ASSERT_EQ(db.m_database_tables.size(), (size_t)0);
}

TEST(database, the_instance_registry_is_capped_at_max_tables)
{
    ASSERT_LE(DatabaseTableAbstractLayer::m_total_instances, (int)MAX_TABLES);
}

/**
 * A service that initialises twice must keep the tables it already has. The
 * registry used to refuse the second registration, which left every table
 * marked unregistered and every get() returning false for the rest of the run.
 */
TEST(database, registering_the_same_instance_again_keeps_it)
{
    Database db;

    struct_tables first = makeTable(4, 50, &s_table_one);
    struct_tables again = makeTable(4, 50, &s_table_one);

    ASSERT_TRUE(db.register_table(first));
    ASSERT_TRUE(db.register_table(again));
    ASSERT_EQ(db.m_database_tables.size(), (size_t)1);
}

TEST(database, re_registering_refreshes_the_descriptor)
{
    Database db;

    struct_tables first = makeTable(4, 50, &s_table_one, 1);
    struct_tables grown = makeTable(4, 80, &s_table_one, 2);

    ASSERT_TRUE(db.register_table(first));
    ASSERT_TRUE(db.register_table(grown));

    ASSERT_EQ(db.m_database_tables.size(), (size_t)1);
    ASSERT_EQ(db.m_database_tables[0].m_table_size, (uint16_t)80);
    ASSERT_EQ(db.m_database_tables[0].m_table_version, (uint16_t)2);
}

TEST(database, another_instance_may_not_take_a_registered_id)
{
    Database db;

    struct_tables first = makeTable(4, 50, &s_table_one);
    struct_tables other = makeTable(4, 50, &s_table_two);

    ASSERT_TRUE(db.register_table(first));
    ASSERT_FALSE(db.register_table(other));
    ASSERT_EQ(db.m_database_tables.size(), (size_t)1);
}

/**
 * A full registry has no room for a new table but always has room for one it is
 * already holding, so reinitialising a device whose tables fill it still works.
 */
TEST(database, a_full_registry_still_takes_a_table_it_already_holds)
{
    Database db;

    struct_tables first = makeTable(1, 50, &s_table_one);
    ASSERT_TRUE(db.register_table(first));

    for (uint16_t i = 1; i < MAX_TABLES; i++)
    {
        struct_tables table = makeTable((uint16_t)(i + 1), 50, &s_table_two);
        ASSERT_TRUE(db.register_table(table));
    }

    ASSERT_EQ(db.m_database_tables.size(), (size_t)MAX_TABLES);

    struct_tables again = makeTable(1, 50, &s_table_one);
    ASSERT_TRUE(db.register_table(again));
    ASSERT_EQ(db.m_database_tables.size(), (size_t)MAX_TABLES);
}
