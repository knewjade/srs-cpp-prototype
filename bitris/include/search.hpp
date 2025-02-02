#pragma once

#include <utility>
#include <bitset>

#include "rows.hpp"
#include "pieces.hpp"
#include "rotate.hpp"
#include "kicks.hpp"
#include "data.hpp"
#include "free_spaces.hpp"
#include "search_core.hpp"

namespace s {
    template<typename Data, Shape Shape>
    struct searcher {
        using data_t = data<Data>;
        using type = typename data_t::type;
        using AlignedBoard = data_t::AlignedBoard;
        static constexpr auto N = free_spaces<Data, Shape>::N;

//    private:
//        template<typename U>
//        static constexpr std::array<Data, N * 10> search_casted(
//                const AlignedBoard &board,
//                const Orientation spawn_orientation,
//                const uint8_t spawn_cx,
//                const uint8_t spawn_cy
//        ) {
//            const auto board_u = data_t::template load<U>(board);
//
//            const auto goals = searcher<U, Shape>::execute(
//                    board_u, spawn_orientation, spawn_cx, spawn_cy
//            );
//
//            alignas(32) std::array<Data, N * 10> array{};
//            static_for_t<N>([&]<size_t SrcIndex>() {
//                const auto converted = data<U>::template to<Data>(goals[SrcIndex]);
//                static_for_t<10>([&]<size_t DestIndex>() {
//                    array[SrcIndex * 10 + DestIndex] = converted[DestIndex];
//                });
//            });
//            return array;
//        }

    public:
        static constexpr std::array<Data, N * 10> search(
                const AlignedBoard &board,
                const Orientation spawn_orientation,
                const uint8_t spawn_cx,
                const uint8_t spawn_cy
        ) {
            const auto board_t = data_t::load(board);
            const auto goals = execute(
                    board_t, spawn_orientation, spawn_cx, spawn_cy
            );

            alignas(32) std::array<Data, N * 10> array{};
            static_for_t<N>([&]<size_t SrcIndex>() {
                static_for_t<10>([&]<size_t DestIndex>() {
                    array[SrcIndex * 10 + DestIndex] = goals[SrcIndex][DestIndex];
                });
            });

            return array;
        }

        static constexpr std::array<type, N> execute(
                const type &board,
                const Orientation spawn_orientation,
                const uint8_t spawn_cx,
                const uint8_t spawn_cy
        ) {
            if constexpr (N == 1) {
                constexpr auto orientation = Orientation::North;
                return search_core::core_so<Data, Shape, orientation>::execute(
                        board, spawn_cx, spawn_cy
                );
            }

            switch (spawn_orientation) {
                case Orientation::North: {
                    constexpr auto orientation = Orientation::North;
                    return search_core::core_so<Data, Shape, orientation>::execute(
                            board, spawn_cx, spawn_cy
                    );
                }
                case Orientation::East: {
                    constexpr auto orientation = Orientation::East;
                    return search_core::core_so<Data, Shape, orientation>::execute(
                            board, spawn_cx, spawn_cy
                    );
                }
                case Orientation::South: {
                    constexpr auto orientation = Orientation::South;
                    return search_core::core_so<Data, Shape, orientation>::execute(
                            board, spawn_cx, spawn_cy
                    );
                }
                case Orientation::West: {
                    constexpr auto orientation = Orientation::West;
                    return search_core::core_so<Data, Shape, orientation>::execute(
                            board, spawn_cx, spawn_cy
                    );
                }
            }
            std::unreachable();
        }
    };
}
