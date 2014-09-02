/*  This file is part of the Vc library. {{{

    Copyright (C) 2013 Matthias Kretz <kretz@kde.org>

    Vc is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as
    published by the Free Software Foundation, either version 3 of
    the License, or (at your option) any later version.

    Vc is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with Vc.  If not, see <http://www.gnu.org/licenses/>.

}}}*/

#ifndef VC_COMMON_SIMD_ARRAY_DATA_H
#define VC_COMMON_SIMD_ARRAY_DATA_H

#include "subscript.h"
#include "macros.h"

namespace Vc_VERSIONED_NAMESPACE
{
namespace Common
{

/// \addtogroup simdarray
/// @{

namespace Operations/*{{{*/
{
struct tag {};
#define Vc_DEFINE_OPERATION(name__)                                                                \
    struct name__ : public tag                                                                     \
    {                                                                                              \
        template <typename V, typename... Args>                                                    \
        Vc_INTRINSIC void operator()(V &v, Args &&... args)                                        \
        {                                                                                          \
            v.name__(std::forward<Args>(args)...);                                                 \
        }                                                                                          \
    }
Vc_DEFINE_OPERATION(gather);
Vc_DEFINE_OPERATION(scatter);
Vc_DEFINE_OPERATION(load);
Vc_DEFINE_OPERATION(store);
Vc_DEFINE_OPERATION(setZero);
Vc_DEFINE_OPERATION(setZeroInverted);
Vc_DEFINE_OPERATION(assign);
#undef Vc_DEFINE_OPERATION
#define Vc_DEFINE_OPERATION(name__, code__)                                              \
    struct name__ : public tag                                                           \
    {                                                                                    \
        template <typename V> Vc_INTRINSIC void operator()(V & v) { code__; }            \
    }
Vc_DEFINE_OPERATION(increment, ++v);
Vc_DEFINE_OPERATION(decrement, --v);
Vc_DEFINE_OPERATION(random, v = V::Random());
#undef Vc_DEFINE_OPERATION
#define Vc_DEFINE_OPERATION(name__, code__)                                                        \
    struct name__ : public tag                                                                     \
    {                                                                                              \
        template <typename V, typename... Args>                                                    \
        Vc_INTRINSIC void operator()(V &v, Args &&... args)                                        \
        {                                                                                          \
            code__;                                                                                \
        }                                                                                          \
    }
Vc_DEFINE_OPERATION(Abs, v = abs(std::forward<Args>(args)...));
Vc_DEFINE_OPERATION(Isnan, v = isnan(std::forward<Args>(args)...));
Vc_DEFINE_OPERATION(Frexp, v = frexp(std::forward<Args>(args)...));
Vc_DEFINE_OPERATION(Ldexp, v = ldexp(std::forward<Args>(args)...));
#undef Vc_DEFINE_OPERATION
template<typename T> using is_operation = std::is_base_of<tag, T>;
}  // namespace Operations }}}

/**
 * \internal
 * Helper type to statically communicate segmentation of one vector register into 2^n parts
 * (Pieces).
 */
template <typename T_, std::size_t Pieces_, std::size_t Index_> struct Segment/*{{{*/
{
    static_assert(Index_ < Pieces_, "You found a bug in Vc. Please report.");

    using type = T_;
    using type_decayed = typename std::decay<type>::type;
    static constexpr std::size_t Pieces = Pieces_;
    static constexpr std::size_t Index = Index_;

    type data;

    static constexpr std::size_t EntryOffset = Index * type_decayed::Size / Pieces;

    decltype(std::declval<type>()[0]) operator[](size_t i) { return data[i + EntryOffset]; }
    decltype(std::declval<type>()[0]) operator[](size_t i) const { return data[i + EntryOffset]; }
};/*}}}*/

/** \internal
  Template class that is used to attach an offset value to an existing type. It is used
  for IndexesFromZero construction in simdarray. The \c data1 constructor needs to know
  that the IndexesFromZero constructor requires an offset so that the whole data is
  constructed as a correct sequence from `0` to `Size - 1`.

  \tparam T The original type that needs the offset attached.
  \tparam Offset An integral value that determines the offset in the complete simdarray.
 */
template <typename T, std::size_t Offset> struct AddOffset
{
    constexpr AddOffset() = default;
};

/** \internal
  Helper type with static functions to generically adjust arguments for the \c data0 and
  \c data1 members of simdarray and simd_mask_array.

  \tparam secondOffset The offset in number of elements that \c data1 has in the simdarray
                       / simd_mask_array. This is essentially equal to the number of
                       elements in \c data0.
 */
template <std::size_t secondOffset> class Split/*{{{*/
{
    static constexpr AddOffset<VectorSpecialInitializerIndexesFromZero::IEnum, secondOffset> hiImpl(
        VectorSpecialInitializerIndexesFromZero::IEnum)
    {
        return {};
    }
    template <std::size_t Offset>
    static constexpr AddOffset<VectorSpecialInitializerIndexesFromZero::IEnum,
                               Offset + secondOffset>
        hiImpl(AddOffset<VectorSpecialInitializerIndexesFromZero::IEnum, Offset>)
    {
        return {};
    }

    // split composite simdarray
    template <typename U, std::size_t N, typename V, std::size_t M>
    static Vc_INTRINSIC auto loImpl(const simdarray<U, N, V, M> &x) -> decltype(internal_data0(x))
    {
        return internal_data0(x);
    }
    template <typename U, std::size_t N, typename V, std::size_t M>
    static Vc_INTRINSIC auto hiImpl(const simdarray<U, N, V, M> &x) -> decltype(internal_data1(x))
    {
        return internal_data1(x);
    }
    template <typename U, std::size_t N, typename V, std::size_t M>
    static Vc_INTRINSIC auto loImpl(simdarray<U, N, V, M> *x) -> decltype(&internal_data0(*x))
    {
        return &internal_data0(*x);
    }
    template <typename U, std::size_t N, typename V, std::size_t M>
    static Vc_INTRINSIC auto hiImpl(simdarray<U, N, V, M> *x) -> decltype(&internal_data1(*x))
    {
        return &internal_data1(*x);
    }

    template <typename U, std::size_t N, typename V>
    static Vc_INTRINSIC Segment<V, 2, 0> loImpl(const simdarray<U, N, V, N> &x)
    {
        return {internal_data(x)};
    }
    template <typename U, std::size_t N, typename V>
    static Vc_INTRINSIC Segment<V, 2, 1> hiImpl(const simdarray<U, N, V, N> &x)
    {
        return {internal_data(x)};
    }
    template <typename U, std::size_t N, typename V>
    static Vc_INTRINSIC Segment<V *, 2, 0> loImpl(const simdarray<U, N, V, N> *x)
    {
        return {&internal_data(*x)};
    }
    template <typename U, std::size_t N, typename V>
    static Vc_INTRINSIC Segment<V *, 2, 1> hiImpl(const simdarray<U, N, V, N> *x)
    {
        return {&internal_data(*x)};
    }

    // split composite simd_mask_array
    template <typename U, std::size_t N, typename V, std::size_t M>
    static Vc_INTRINSIC auto loImpl(const simd_mask_array<U, N, V, M> &x) -> decltype(internal_data0(x))
    {
        return internal_data0(x);
    }
    template <typename U, std::size_t N, typename V, std::size_t M>
    static Vc_INTRINSIC auto hiImpl(const simd_mask_array<U, N, V, M> &x) -> decltype(internal_data1(x))
    {
        return internal_data1(x);
    }

    template <typename U, std::size_t N, typename V>
    static Vc_INTRINSIC Segment<typename simd_mask_array<U, N, V, N>::mask_type, 2, 0> loImpl(
        const simd_mask_array<U, N, V, N> &x)
    {
        return {internal_data(x)};
    }
    template <typename U, std::size_t N, typename V>
    static Vc_INTRINSIC Segment<typename simd_mask_array<U, N, V, N>::mask_type, 2, 1> hiImpl(
        const simd_mask_array<U, N, V, N> &x)
    {
        return {internal_data(x)};
    }

    // split Vector<T> and Mask<T>
    template <typename T>
    static constexpr bool is_vector_or_mask(){
        return (Traits::is_simd_vector<T>::value && !Traits::is_simdarray<T>::value) ||
               (Traits::is_simd_mask<T>::value && !Traits::is_simd_mask_array<T>::value);
    }
    template <typename V>
    static Vc_INTRINSIC Segment<V, 2, 0> loImpl(V &&x, enable_if<is_vector_or_mask<V>()> = nullarg)
    {
        return {std::forward<V>(x)};
    }
    template <typename V>
    static Vc_INTRINSIC Segment<V, 2, 1> hiImpl(V &&x, enable_if<is_vector_or_mask<V>()> = nullarg)
    {
        return {std::forward<V>(x)};
    }

    // generically split Segments
    template <typename V, std::size_t Pieces, std::size_t Index>
    static Vc_INTRINSIC Segment<V, 2 * Pieces, Index *Pieces + 0> loImpl(const Segment<V, Pieces, Index> &x)
    {
        return {x.data};
    }
    template <typename V, std::size_t Pieces, std::size_t Index>
    static Vc_INTRINSIC Segment<V, 2 * Pieces, Index *Pieces + 1> hiImpl(const Segment<V, Pieces, Index> &x)
    {
        return {x.data};
    }

    /** \internal
     * \name Checks for existence of \c loImpl / \c hiImpl
     */
    //@{
    template <typename T, typename = decltype(loImpl(std::declval<T>()))>
    static std::true_type have_lo_impl(int);
    template <typename T> static std::false_type have_lo_impl(float);
    template <typename T> static constexpr bool have_lo_impl()
    {
        return decltype(have_lo_impl<T>(1))::value;
    }

    template <typename T, typename = decltype(hiImpl(std::declval<T>()))>
    static std::true_type have_hi_impl(int);
    template <typename T> static std::false_type have_hi_impl(float);
    template <typename T> static constexpr bool have_hi_impl()
    {
        return decltype(have_hi_impl<T>(1))::value;
    }
    //@}

public:
    /** \internal
     * \name with Operations tag
     *
     * These functions don't overload on the data parameter. The first parameter (the tag) clearly
     * identifies the intended function.
     */
    //@{
    template <typename U>
    static Vc_INTRINSIC const U *lo(Operations::gather, const U *ptr)
    {
        return ptr;
    }
    template <typename U>
    static Vc_INTRINSIC const U *hi(Operations::gather, const U *ptr)
    {
        return ptr + secondOffset;
    }
    template <typename U>
    static Vc_INTRINSIC const U *lo(Operations::scatter, const U *ptr)
    {
        return ptr;
    }
    template <typename U>
    static Vc_INTRINSIC const U *hi(Operations::scatter, const U *ptr)
    {
        return ptr + secondOffset;
    }
    //@}

    /** \internal
      \name without Operations tag

      These functions are not clearly tagged as to where they are used and therefore
      behave differently depending on the type of the parameter. Different behavior is
      implemented via overloads of \c loImpl and \c hiImpl. They are not overloads of \c
      lo and \c hi directly because it's hard to compete against a universal reference
      (i.e. an overload for `int` requires overloads for `int &`, `const int &`, and `int
      &&`. If one of them were missing `U &&` would win in overload resolution).
     */
    //@{
    template <typename U>
    static Vc_ALWAYS_INLINE decltype(loImpl(std::declval<U>())) lo(U &&x)
    {
        return loImpl(std::forward<U>(x));
    }
    template <typename U>
    static Vc_ALWAYS_INLINE decltype(hiImpl(std::declval<U>())) hi(U &&x)
    {
        return hiImpl(std::forward<U>(x));
    }

    template <typename U>
    static Vc_ALWAYS_INLINE enable_if<!have_lo_impl<U>(), U> lo(U &&x)
    {
        return std::forward<U>(x);
    }
    template <typename U>
    static Vc_ALWAYS_INLINE enable_if<!have_hi_impl<U>(), U> hi(U &&x)
    {
        return std::forward<U>(x);
    }
    //@}
};/*}}}*/

template <typename Op, typename U> static Vc_INTRINSIC U actual_value(Op, U &&x)
{
  return std::forward<U>(x);
}
template <typename Op, typename U, std::size_t M, typename V>
static Vc_INTRINSIC const V &actual_value(Op, const simdarray<U, M, V, M> &x)
{
  return internal_data(x);
}
template <typename Op, typename U, std::size_t M, typename V>
static Vc_INTRINSIC const V &actual_value(Op, simdarray<U, M, V, M> &&x)
{
  return internal_data(x);
}
template <typename Op, typename U, std::size_t M, typename V>
static Vc_INTRINSIC V *actual_value(Op, simdarray<U, M, V, M> *x)
{
  return &internal_data(*x);
}
template <typename Op, typename U, std::size_t M, typename V>
static Vc_INTRINSIC const typename V::Mask &actual_value(Op, const simd_mask_array<U, M, V, M> &x)
{
  return internal_data(x);
}
template <typename Op, typename U, std::size_t M, typename V>
static Vc_INTRINSIC const typename V::Mask &actual_value(Op, simd_mask_array<U, M, V, M> &&x)
{
  return internal_data(x);
}
template <typename Op, typename U, std::size_t M, typename V>
static Vc_INTRINSIC typename V::Mask *actual_value(Op, simd_mask_array<U, M, V, M> *x)
{
  return &internal_data(*x);
}

/// @}

}  // namespace Common
}  // namespace Vc

#include "undomacros.h"

#endif // VC_COMMON_SIMD_ARRAY_DATA_H

// vim: foldmethod=marker
