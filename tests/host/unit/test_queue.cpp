/******************************** Queue Tests *********************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 16th Aug 2026
******************************************************************************/

#include <pditest.h>
#include <utility/queue/proto.h>
#include <utility/queue/queue.h>

/**
 * queue.h offers no teardown call, so the owner of a QUEUE releases queue.buf
 * itself the way the mqtt client does.
 */
struct ScopedQueue
{
    QUEUE queue;

    ScopedQueue(int buffersize) { QUEUE_Init(&queue, buffersize); }
    ~ScopedQueue() { pdiutil::safe_delete_array(queue.buf); }

    QUEUE *operator&() { return &queue; }
};

TEST(queue, starts_empty)
{
    ScopedQueue queue(64);
    ASSERT_TRUE(QUEUE_IsEmpty(&queue));
}

TEST(queue, is_not_empty_after_a_put)
{
    ScopedQueue queue(64);
    uint8_t payload[] = {1, 2, 3};

    ASSERT_GE(QUEUE_Puts(&queue, payload, sizeof(payload)), 0);
    ASSERT_FALSE(QUEUE_IsEmpty(&queue));
}

TEST(queue, round_trips_a_message)
{
    ScopedQueue queue(64);
    uint8_t payload[] = {0x10, 0x20, 0x30, 0x40};
    uint8_t out[32];
    uint16_t outlen = 0;

    QUEUE_Puts(&queue, payload, sizeof(payload));

    memset(out, 0, sizeof(out));
    ASSERT_GE(QUEUE_Gets(&queue, out, &outlen, sizeof(out)), 0);
    ASSERT_EQ(outlen, (uint16_t)sizeof(payload));
    ASSERT_MEMEQ(out, payload, sizeof(payload));
}

TEST(queue, keeps_message_boundaries)
{
    ScopedQueue queue(64);
    uint8_t first[] = {1, 2};
    uint8_t second[] = {3, 4, 5};
    uint8_t out[32];
    uint16_t outlen = 0;

    QUEUE_Puts(&queue, first, sizeof(first));
    QUEUE_Puts(&queue, second, sizeof(second));

    memset(out, 0, sizeof(out));
    QUEUE_Gets(&queue, out, &outlen, sizeof(out));
    ASSERT_EQ(outlen, (uint16_t)sizeof(first));
    ASSERT_MEMEQ(out, first, sizeof(first));

    memset(out, 0, sizeof(out));
    QUEUE_Gets(&queue, out, &outlen, sizeof(out));
    ASSERT_EQ(outlen, (uint16_t)sizeof(second));
    ASSERT_MEMEQ(out, second, sizeof(second));
}

TEST(queue, is_empty_again_once_drained)
{
    ScopedQueue queue(64);
    uint8_t payload[] = {7, 8};
    uint8_t out[32];
    uint16_t outlen = 0;

    QUEUE_Puts(&queue, payload, sizeof(payload));
    QUEUE_Gets(&queue, out, &outlen, sizeof(out));

    ASSERT_TRUE(QUEUE_IsEmpty(&queue));
}

TEST(queue, carries_bytes_that_collide_with_framing)
{
    ScopedQueue queue(128);
    uint8_t payload[] = {0x7D, 0x7E, 0x7D, 0x7E, 0x00, 0xFF};
    uint8_t out[32];
    uint16_t outlen = 0;

    QUEUE_Puts(&queue, payload, sizeof(payload));

    memset(out, 0, sizeof(out));
    ASSERT_GE(QUEUE_Gets(&queue, out, &outlen, sizeof(out)), 0);
    ASSERT_EQ(outlen, (uint16_t)sizeof(payload));
    ASSERT_MEMEQ(out, payload, sizeof(payload));
}

TEST(queue, refuses_a_message_larger_than_the_buffer)
{
    ScopedQueue queue(16);
    uint8_t payload[64];
    memset(payload, 0xAB, sizeof(payload));

    ASSERT_LT(QUEUE_Puts(&queue, payload, sizeof(payload)), 0);
}

TEST(proto, ring_buffer_round_trip)
{
    RINGBUF ring;
    uint8_t storage[128];
    uint8_t out[64];
    uint16_t outlen = 0;
    const uint8_t packet[] = {0x01, 0x7E, 0x7D, 0x02};

    RINGBUF_Init(&ring, storage, sizeof(storage));
    ASSERT_GE(PROTO_AddRb(&ring, packet, (int)sizeof(packet)), 0);

    memset(out, 0, sizeof(out));
    ASSERT_GE(PROTO_ParseRb(&ring, out, &outlen, sizeof(out)), 0);
    ASSERT_EQ(outlen, (uint16_t)sizeof(packet));
    ASSERT_MEMEQ(out, packet, sizeof(packet));
}

TEST(proto, ring_buffer_escapes_every_reserved_byte)
{
    RINGBUF ring;
    uint8_t storage[128];
    uint8_t out[64];
    uint16_t outlen = 0;
    const uint8_t packet[] = {0x7D, 0x7E, 0x7F, 0x7D, 0x7E, 0x7F};

    RINGBUF_Init(&ring, storage, sizeof(storage));
    ASSERT_GE(PROTO_AddRb(&ring, packet, (int)sizeof(packet)), 0);

    memset(out, 0, sizeof(out));
    ASSERT_GE(PROTO_ParseRb(&ring, out, &outlen, sizeof(out)), 0);
    ASSERT_EQ(outlen, (uint16_t)sizeof(packet));
    ASSERT_MEMEQ(out, packet, sizeof(packet));
}

TEST(proto, ring_buffer_keeps_successive_packets_separate)
{
    RINGBUF ring;
    uint8_t storage[128];
    uint8_t out[64];
    uint16_t outlen = 0;
    const uint8_t first[] = {0xAA, 0xBB};
    const uint8_t second[] = {0xCC, 0xDD, 0xEE};

    RINGBUF_Init(&ring, storage, sizeof(storage));
    PROTO_AddRb(&ring, first, (int)sizeof(first));
    PROTO_AddRb(&ring, second, (int)sizeof(second));

    memset(out, 0, sizeof(out));
    ASSERT_GE(PROTO_ParseRb(&ring, out, &outlen, sizeof(out)), 0);
    ASSERT_EQ(outlen, (uint16_t)sizeof(first));
    ASSERT_MEMEQ(out, first, sizeof(first));

    memset(out, 0, sizeof(out));
    ASSERT_GE(PROTO_ParseRb(&ring, out, &outlen, sizeof(out)), 0);
    ASSERT_EQ(outlen, (uint16_t)sizeof(second));
    ASSERT_MEMEQ(out, second, sizeof(second));
}

TEST(proto, ring_buffer_parse_reports_nothing_when_empty)
{
    RINGBUF ring;
    uint8_t storage[64];
    uint8_t out[32];
    uint16_t outlen = 0;

    RINGBUF_Init(&ring, storage, sizeof(storage));
    ASSERT_LT(PROTO_ParseRb(&ring, out, &outlen, sizeof(out)), 0);
}

TEST(proto, ring_buffer_carries_a_binary_payload_unchanged)
{
    RINGBUF ring;
    uint8_t storage[256];
    uint8_t out[128];
    uint16_t outlen = 0;
    uint8_t packet[64];

    for (uint8_t i = 0; i < sizeof(packet); i++)
    {
        packet[i] = (uint8_t)(i * 3);
    }

    RINGBUF_Init(&ring, storage, sizeof(storage));
    ASSERT_GE(PROTO_AddRb(&ring, packet, (int)sizeof(packet)), 0);

    memset(out, 0, sizeof(out));
    ASSERT_GE(PROTO_ParseRb(&ring, out, &outlen, sizeof(out)), 0);
    ASSERT_EQ(outlen, (uint16_t)sizeof(packet));
    ASSERT_MEMEQ(out, packet, sizeof(packet));
}

/* ------------------------------------------------- behaviour under pressure */

TEST(queue, a_full_queue_refuses_rather_than_making_room)
{
    // Each message frames to 18 bytes, so two fit and the third cannot.
    ScopedQueue queue(40);

    uint8_t first[16], second[16], late[16];
    memset(first, 0xA1, sizeof(first));
    memset(second, 0xB2, sizeof(second));
    memset(late, 0xC3, sizeof(late));

    ASSERT_GT(QUEUE_Puts(&queue, first, sizeof(first)), 0);
    ASSERT_GT(QUEUE_Puts(&queue, second, sizeof(second)), 0);
    ASSERT_LT(QUEUE_Puts(&queue, late, sizeof(late)), 0);

    uint8_t out[256];
    uint16_t outlen = 0;
    ASSERT_EQ(QUEUE_Gets(&queue, out, &outlen, sizeof(out)), 0);
    ASSERT_EQ(outlen, (uint16_t)sizeof(first));
    ASSERT_MEMEQ(out, first, sizeof(first));
}

TEST(queue, a_refused_put_leaves_the_queue_exactly_as_it_was)
{
    ScopedQueue queue(32);

    uint8_t filler[20], oversize[20];
    memset(filler, 0x11, sizeof(filler));
    memset(oversize, 0x22, sizeof(oversize));

    ASSERT_GT(QUEUE_Puts(&queue, filler, sizeof(filler)), 0);

    size_t before = queue.queue.rb.fill_cnt;
    ASSERT_LT(QUEUE_Puts(&queue, oversize, sizeof(oversize)), 0);
    ASSERT_EQ((uint32_t)queue.queue.rb.fill_cnt, (uint32_t)before);

    uint8_t out[256];
    uint16_t outlen = 0;
    ASSERT_EQ(QUEUE_Gets(&queue, out, &outlen, sizeof(out)), 0);
    ASSERT_EQ(outlen, (uint16_t)sizeof(filler));
    ASSERT_MEMEQ(out, filler, sizeof(filler));
    ASSERT_TRUE(QUEUE_IsEmpty(&queue));
}

TEST(queue, room_freed_by_a_read_admits_the_message_that_was_refused)
{
    ScopedQueue queue(40);

    uint8_t first[16], late[16];
    memset(first, 0xA1, sizeof(first));
    memset(late, 0xC3, sizeof(late));

    ASSERT_GT(QUEUE_Puts(&queue, first, sizeof(first)), 0);
    ASSERT_GT(QUEUE_Puts(&queue, first, sizeof(first)), 0);
    ASSERT_LT(QUEUE_Puts(&queue, late, sizeof(late)), 0);

    uint8_t out[256];
    uint16_t outlen = 0;
    ASSERT_EQ(QUEUE_Gets(&queue, out, &outlen, sizeof(out)), 0);

    ASSERT_GT(QUEUE_Puts(&queue, late, sizeof(late)), 0);
}

TEST(proto, an_orphaned_fragment_does_not_corrupt_the_next_message)
{
    ScopedQueue queue(32);

    uint8_t oversize[64], good[8];
    memset(oversize, 0x33, sizeof(oversize));
    memset(good, 0x44, sizeof(good));

    ASSERT_LT(QUEUE_Puts(&queue, oversize, sizeof(oversize)), 0);
    ASSERT_GT(QUEUE_Puts(&queue, good, sizeof(good)), 0);

    uint8_t out[256];
    uint16_t outlen = 0;
    ASSERT_EQ(QUEUE_Gets(&queue, out, &outlen, sizeof(out)), 0);
    ASSERT_EQ(outlen, (uint16_t)sizeof(good));
    ASSERT_MEMEQ(out, good, sizeof(good));
}
