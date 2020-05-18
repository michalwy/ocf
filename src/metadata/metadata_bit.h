/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*******************************************************************************
 * Sector mask getter
 ******************************************************************************/

static inline uint64_t _get_mask(uint32_t start, uint32_t stop)
{
	uint64_t mask = 0;

	ENV_BUG_ON(start >= 64);
	ENV_BUG_ON(stop >= 64);
	ENV_BUG_ON(stop < start);

	mask = ~mask;
	mask >>= start + (63 - stop);
	mask <<= start;

	return mask;
}

#define _get_mask_u8(start, stop) _get_mask(start, stop)
#define _get_mask_u16(start, stop) _get_mask(start, stop)
#define _get_mask_u32(start, stop) _get_mask(start, stop)
#define _get_mask_u64(start, stop) _get_mask(start, stop)

typedef __uint128_t u128;

static inline u128 _get_mask_u128(uint32_t start, uint32_t stop)
{
	u128 mask = 0;

	ENV_BUG_ON(start >= 128);
	ENV_BUG_ON(stop >= 128);
	ENV_BUG_ON(stop < start);

	mask = ~mask;
	mask >>= start + (127 - stop);
	mask <<= start;

	return mask;
}

#define ocf_metadata_bit_struct(type, count) \
struct ocf_metadata_map_##type##_##count { \
	struct ocf_metadata_map map; \
	type valid[count]; \
	type dirty[count]; \
} __attribute__((packed))

#define ocf_metadata_bit_for_each_byte(type, count)	\
{ \
	uint32_t bits = sizeof(type) << 3; \
	uint32_t b; \
	uint32_t bits_left; \
	uint32_t s_b, e_b; \
	type mask; \
\
	struct ocf_metadata_hash_ctrl *ctrl = \
		(struct ocf_metadata_hash_ctrl *) cache->metadata.iface_priv; \
\
	struct ocf_metadata_raw *raw = \
			&ctrl->raw_desc[metadata_segment_collision]; \
\
	struct ocf_metadata_map_##type##_##count *map = raw->mem_pool; \
\
	_raw_bug_on(raw, line); \
\
	bits_left = stop - start + 1; \
	b = start / bits; \
	s_b = start % bits; \
	while (bits_left) { \
		BUG_ON(b >= count); \
		e_b = s_b + bits_left - 1; \
		if (e_b >= bits) { \
			e_b = bits - 1; \
		} \
		mask = _get_mask_##type(s_b, e_b);


#define ocf_metadata_bit_next_byte \
} \
		bits_left -= e_b - s_b + 1; \
		s_b = 0; \
		b++; \
}

#define ocf_metadata_bit_func(what, type, count) \
static bool _ocf_metadata_test_##what##_##type##_##count(struct ocf_cache *cache, \
		ocf_cache_line_t line, uint32_t start, uint32_t stop, bool all) \
{ \
	ocf_metadata_bit_for_each_byte(type, count) { \
		if (all) { \
			if (mask != (map[line].what[b] & mask)) { \
				return false; \
			} \
		} else { \
			if (map[line].what[b] & mask) { \
				return true; \
			} \
		} \
		ocf_metadata_bit_next_byte; \
	} \
	return all; \
} \
\
static bool _ocf_metadata_test_out_##what##_##type##_##count(struct ocf_cache *cache, \
		ocf_cache_line_t line, uint32_t start, uint32_t stop) \
{ \
	ocf_metadata_bit_for_each_byte(type, count) { \
		if (map[line].what[b] & ~mask) { \
			return true; \
		} \
		ocf_metadata_bit_next_byte; \
	} \
	return false; \
} \
\
static bool _ocf_metadata_clear_##what##_##type##_##count(struct ocf_cache *cache, \
		ocf_cache_line_t line, uint32_t start, uint32_t stop) \
{ \
	bool result = false; \
	ocf_metadata_bit_for_each_byte(type, count) { \
		map[line].what[b] &= ~mask; \
\
		if (map[line].what[b]) { \
			result |= true; \
		} \
		ocf_metadata_bit_next_byte; \
	} \
	return result; \
} \
\
static bool _ocf_metadata_set_##what##_##type##_##count(struct ocf_cache *cache, \
		ocf_cache_line_t line, uint32_t start, uint32_t stop) \
{ \
	bool result = false; \
	ocf_metadata_bit_for_each_byte(type, count) { \
		result |= (map[line].what[b] ? true : false); \
\
		map[line].what[b] |= mask; \
\
		ocf_metadata_bit_next_byte; \
	} \
\
	return result; \
} \
\
static bool _ocf_metadata_test_and_set_##what##_##type##_##count( \
		struct ocf_cache *cache, ocf_cache_line_t line, \
		uint32_t start, uint32_t stop, bool all) \
{ \
	bool test = all; \
\
	ocf_metadata_bit_for_each_byte(type, count) { \
		if (all) { \
			if (mask != (map[line].what[b] & mask)) { \
				test = false; \
			} \
		} else { \
			if (map[line].what[b] & mask) { \
				test = true; \
			} \
		} \
		map[line].what[b] |= mask; \
\
		ocf_metadata_bit_next_byte; \
	} \
\
	return test; \
} \
\
static bool _ocf_metadata_test_and_clear_##what##_##type##_##count( \
		struct ocf_cache *cache, ocf_cache_line_t line, \
		uint32_t start, uint32_t stop, bool all) \
{ \
	bool test = all; \
\
	ocf_metadata_bit_for_each_byte(type, count) { \
		if (all) { \
			if (mask != (map[line].what[b] & mask)) { \
				test = false; \
			} \
		} else { \
			if (map[line].what[b] & mask) { \
				test = true; \
			} \
		} \
		map[line].what[b] &= ~mask; \
\
		ocf_metadata_bit_next_byte; \
	} \
\
	return test; \
} \

ocf_metadata_bit_struct(u8, 1);			/* 4k */
ocf_metadata_bit_struct(u16, 1);		/* 8k */
ocf_metadata_bit_struct(u32, 1);		/* 16k */
ocf_metadata_bit_struct(u64, 1);		/* 32k */
ocf_metadata_bit_struct(u128, 1);		/* 64k */
ocf_metadata_bit_struct(u128, 2);		/* 128k */
ocf_metadata_bit_struct(u128, 4);		/* 256k */
ocf_metadata_bit_struct(u128, 8);		/* 512k */
ocf_metadata_bit_struct(u128, 16);		/* 1024k */
ocf_metadata_bit_struct(u128, 32);		/* 2048k */

ocf_metadata_bit_func(dirty, u8, 1);
ocf_metadata_bit_func(dirty, u16, 1);
ocf_metadata_bit_func(dirty, u32, 1);
ocf_metadata_bit_func(dirty, u64, 1);
ocf_metadata_bit_func(dirty, u128, 1);
ocf_metadata_bit_func(dirty, u128, 2);
ocf_metadata_bit_func(dirty, u128, 4);
ocf_metadata_bit_func(dirty, u128, 8);
ocf_metadata_bit_func(dirty, u128, 16);
ocf_metadata_bit_func(dirty, u128, 32);

ocf_metadata_bit_func(valid, u8, 1);
ocf_metadata_bit_func(valid, u16, 1);
ocf_metadata_bit_func(valid, u32, 1);
ocf_metadata_bit_func(valid, u64, 1);
ocf_metadata_bit_func(valid, u128, 1);
ocf_metadata_bit_func(valid, u128, 2);
ocf_metadata_bit_func(valid, u128, 4);
ocf_metadata_bit_func(valid, u128, 8);
ocf_metadata_bit_func(valid, u128, 16);
ocf_metadata_bit_func(valid, u128, 32);

