#pragma once

#include <ranges>
#include <iostream>
#include <experimental/simd>

#include "bits.hpp"
#include "kicks.hpp"

namespace stdx = std::experimental;

template<typename T>
struct data {
    using type = stdx::simd<T, stdx::simd_abi::fixed_size<10> >;
    using bits_t = bits<T>;

    struct AlignedBoard {
        alignas(32) std::array<T, 10> columns;
    };

    [[gnu::always_inline]]
    static inline type load(const AlignedBoard &board) {
        return type{board.columns.data(), stdx::vector_aligned};
    }

    [[gnu::always_inline]]
    static inline type make_zero() {
        return make_square<0>();
    }

    template<T Value>
    [[gnu::always_inline]]
    static inline type make_square() {
        return type{Value};
    }

    [[gnu::always_inline]]
    static inline type make_square(const T value) {
        return type{value};
    }

    [[gnu::always_inline]]
    static inline type make_spawn(
            const type &free_space,
            const uint8_t spawn_cx,
            const uint8_t spawn_cy
    ) {
        if (is_continuous_line(free_space, spawn_cy)) {
            return calculate_spawn_area(free_space, spawn_cy);
        }

        alignas(32) std::array<T, 10> b{};
        b[spawn_cx] = bits_t::one << spawn_cy;
        return type{b.data(), stdx::vector_aligned};
    }

    [[gnu::always_inline]]
    static inline bool is_continuous_line(
            const type &free_space,
            const uint8_t y
    ) {
        if (bits_t::bit_size <= y) {
            return true;
        }

        size_t count = 0;
        auto prev = bits_t::zero;
        const auto m = bits_t::one << y;
        for (size_t index = 0; index < 10; ++index) {
            const auto bit = free_space[index] & m;
            if (prev && (prev ^ bit)) {
                count++;
            }
            prev = bit;
        }
        count += prev ? 1 : 0;
        return count <= 1;
    }

    [[gnu::always_inline]]
    static inline type calculate_spawn_area(
            const type &free_space,
            const uint8_t spawn_cy
    ) {
        // 上にブロック（屋根）がある空白を1とする
        const auto roof_blocks = (~free_space >> 1) & free_space;

        // 屋根のある行を1とするビット列
        const auto roof_rows = static_fold_t<10>([&]<size_t Index>(const auto acc) {
            return acc | roof_blocks[Index];
        }, bits_t::zero);

        // spawn位置より下を1とするビット列
        const int spawn_offset = bits_t::bit_size - spawn_cy - 1;
        const auto below_spawn_mask =
                0 < spawn_offset ? static_cast<T>(bits_t::full << spawn_offset) >> spawn_offset : bits_t::full;

        if (roof_rows == 0) {
            return make_square(below_spawn_mask) & free_space;
        }

        // 移動できそうな範囲で、屋根のある行の上までを1とするビット列
        const auto most_significant_index = bits_t::most_significant_index(roof_rows & (below_spawn_mask >> 1));
        const auto shift_amount = most_significant_index + 1;
        const auto above_roof_mask = (bits_t::full >> shift_amount) << shift_amount;

        return make_square(below_spawn_mask & above_roof_mask) & free_space;
    }

    template<size_t Down, bool CeilOpen = false>
    [[gnu::always_inline]]
    static inline type shift_down(const type &data) {
        if constexpr (Down == 0) {
            return data;
        }
        if constexpr (bits_t::bit_size <= Down) {
            make_square<CeilOpen ? bits_t::full : 0>();
        }
        return CeilOpen ? ~(~data >> Down) : data >> Down;
    }

    template<size_t Up>
    [[gnu::always_inline]]
    static inline type shift_up(const type &data) {
        if constexpr (Up == 0) {
            return data;
        }
        if constexpr (bits_t::bit_size <= Up) {
            return make_square<0>();
        }
        return data << Up;
    }

    template<size_t Right>
    [[gnu::always_inline]]
    static inline type shift_right(const type &data) {
        if constexpr (Right == 0) {
            return data;
        }
        if constexpr (10 <= Right) {
            return make_square<0>();
        }

        return type([=](const auto i) {
            if constexpr (constexpr auto index = static_cast<int>(i) - static_cast<int>(Right); index < 0) {
                return 0;
            } else {
                return data[index];
            }
        });
    }

    template<size_t Left>
    [[gnu::always_inline]]
    static inline type shift_left(const type &data) {
        if constexpr (Left == 0) {
            return data;
        }

        if constexpr (10 <= Left) {
            return make_square<0>();
        }

        return type([=](const auto i) {
            if constexpr (constexpr size_t index = i + Left; index >= 10) {
                return 0;
            } else {
                return data[index];
            }
        });
    }

    template<Offset Offset>
    [[gnu::always_inline]]
    static inline type shift(const type &data) {
        constexpr auto shift_vertical = [](const auto &v) {
            if constexpr (0 < Offset.y) {
                constexpr auto Up = static_cast<size_t>(Offset.y);
                return shift_up<Up>(v);
            } else if constexpr (Offset.y < 0) {
                constexpr auto Down = static_cast<size_t>(-Offset.y);
                return shift_down<Down>(v);
            } else {
                return v;
            }
        };

        if constexpr (0 < Offset.x) {
            constexpr auto Right = static_cast<size_t>(Offset.x);
            return shift_vertical(shift_right<Right>(data));
        } else if constexpr (Offset.x < 0) {
            constexpr auto Left = static_cast<size_t>(-Offset.x);
            return shift_vertical(shift_left<Left>(data));
        } else {
            return shift_vertical(data);
        }
    }

    [[gnu::always_inline]]
    static inline bool is_equal_to(const type &left, const type &right) {
        return all_of(left == right);
    }

    [[gnu::always_inline]]
    static inline bool is_equal_to(const type &left, const T right) {
        return all_of(left == right);
    }

    [[gnu::always_inline]]
    static inline bool is_not_equal_to(const type &left, const type &right) {
        return !all_of(left == right);
    }

    static inline void show(const type &data, const int height = bits_t::bit_size) {
        std::string str;
        for (int y = height - 1; y >= 0; --y) {
            for (int x = 0; x < 10; ++x) {
                auto is_occupied_at = data[x] & (bits_t::one << y);
                str += is_occupied_at ? '#' : '.';
            }
            str += '\n';
        }
        std::cout << str << std::endl;
    }

    static inline std::optional<type> from_str(const std::string &str) {
        type board = make_zero();
        const int ceiling = bits_t::bit_size;
        int index = 0;

        for (const char ch: std::ranges::reverse_view(str)) {
            switch (ch) {
                case '#':
                case 'X':
                    if (index >= 10 * ceiling) {
                        return std::nullopt; // ExceedBoardCeiling
                    }
                    board[9 - (index % 10)] |= bits_t::one << (index / 10);
                    ++index;
                    break;

                case '.':
                case '_':
                    ++index;
                    break;

                default: {
                    // noop
                }
            }
        }

        if (index % 10 != 0) {
            return std::nullopt; // MismatchedWidth
        }

        return std::make_optional(board);
    }
};
