
// Copyright (c) 2022 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef FLUX_OP_MINMAX_HPP_INCLUDED
#define FLUX_OP_MINMAX_HPP_INCLUDED

#include <flux/core.hpp>

#include <flux/op/slice.hpp>

namespace flux {

template <typename T>
struct minmax_result {
    T min;
    T max;
};

namespace detail {

struct min_op {
    template <sequence Seq, strict_weak_order_for<Seq> Cmp = std::ranges::less>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq, Cmp cmp = Cmp{}) const
        -> flux::optional<value_t<Seq>>
    {
        return flux::fold_first(FLUX_FWD(seq), [&](auto min, auto&& elem) -> value_t<Seq> {
            if (std::invoke(cmp, elem, min)) {
                return value_t<Seq>(FLUX_FWD(elem));
            } else {
                return min;
            }
        });
    }
};

struct max_op {
    template <sequence Seq, strict_weak_order_for<Seq> Cmp = std::ranges::less>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq, Cmp cmp = Cmp{}) const
        -> flux::optional<value_t<Seq>>
    {
        return flux::fold_first(FLUX_FWD(seq), [&](auto max, auto&& elem) -> value_t<Seq> {
            if (!std::invoke(cmp, elem, max)) {
                return value_t<Seq>(FLUX_FWD(elem));
            } else {
                return max;
            }
        });
    }
};

struct minmax_op {
    template <sequence Seq, strict_weak_order_for<Seq> Cmp = std::ranges::less>
    [[nodiscard]]
    constexpr auto operator()(Seq&& seq, Cmp cmp = Cmp{}) const
        -> flux::optional<minmax_result<value_t<Seq>>>
    {
        using R = minmax_result<value_t<Seq>>;

        auto cur = flux::first(seq);
        if (flux::is_last(seq, cur)) {
            return std::nullopt;
        }

        R init = R{value_t<Seq>(flux::read_at(seq, cur)),
                   value_t<Seq>(flux::read_at(seq, cur))};

        auto fold_fn = [&](R mm, auto&& elem) -> R {
            if (std::invoke(cmp, elem, mm.min)) {
                mm.min = value_t<Seq>(elem);
            }
            if (!std::invoke(cmp, elem, mm.max)) {
                mm.max = value_t<Seq>(FLUX_FWD(elem));
            }
            return mm;
        };

        return flux::optional<R>(std::in_place,
                                flux::fold(flux::slice(seq, std::move(cur), flux::last), fold_fn, std::move(init)));
    }
};

} // namespace detail

inline constexpr auto min = detail::min_op{};
inline constexpr auto max = detail::max_op{};
inline constexpr auto minmax = detail::minmax_op{};

template <typename Derived>
template <typename Cmp>
    requires strict_weak_order_for<Cmp, Derived>
constexpr auto inline_sequence_base<Derived>::max(Cmp cmp)
{
    return flux::max(derived(), std::move(cmp));
}

template <typename Derived>
template <typename Cmp>
    requires strict_weak_order_for<Cmp, Derived>
constexpr auto inline_sequence_base<Derived>::min(Cmp cmp)
{
    return flux::min(derived(), std::move(cmp));
}

template <typename Derived>
template <typename Cmp>
    requires strict_weak_order_for<Cmp, Derived>
constexpr auto inline_sequence_base<Derived>::minmax(Cmp cmp)
{
    return flux::minmax(derived(), std::move(cmp));
}

} // namespace flux

#endif // FLUX_OP_MINMAX_HPP_INCLUDED
