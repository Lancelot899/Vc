/*  This file is part of the Vc library. {{{
Copyright © 2015 Matthias Kretz <kretz@kde.org>

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the names of contributing organizations nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

}}}*/

#ifndef VC_COMMON_VECTORABI_H_
#define VC_COMMON_VECTORABI_H_

namespace Vc_VERSIONED_NAMESPACE
{
namespace VectorAbi
{
struct Scalar {};
struct Sse {};
struct Avx {};
struct Mic {};
template <typename T>
using Avx1Abi = typename std::conditional<std::is_integral<T>::value, VectorAbi::Sse,
                                          VectorAbi::Avx>::type;
template <typename T>
using Best = typename std::conditional<
    CurrentImplementation::is(ScalarImpl), Scalar,
    typename std::conditional<
        CurrentImplementation::is_between(SSE2Impl, SSE42Impl), Sse,
        typename std::conditional<
            CurrentImplementation::is(AVXImpl), Avx1Abi<T>,
            typename std::conditional<
                CurrentImplementation::is(AVX2Impl), Avx,
                typename std::conditional<CurrentImplementation::is(MICImpl), Mic,
                                          void>::type>::type>::type>::type>::type;
#ifdef Vc_IMPL_AVX2
static_assert(std::is_same<Best<float>, Avx>::value, "");
static_assert(std::is_same<Best<int>, Avx>::value, "");
#elif defined Vc_IMPL_AVX
static_assert(std::is_same<Best<float>, Avx>::value, "");
static_assert(std::is_same<Best<int>, Sse>::value, "");
#elif defined Vc_IMPL_SSE
static_assert(CurrentImplementation::is_between(SSE2Impl, SSE42Impl), "");
static_assert(std::is_same<Best<float>, Sse>::value, "");
static_assert(std::is_same<Best<int>, Sse>::value, "");
#elif defined Vc_IMPL_MIC
static_assert(std::is_same<Best<float>, Mic>::value, "");
static_assert(std::is_same<Best<int>, Mic>::value, "");
#elif defined Vc_IMPL_Scalar
static_assert(std::is_same<Best<float>, Scalar>::value, "");
static_assert(std::is_same<Best<int>, Scalar>::value, "");
#endif
}  // namespace VectorAbi
}  // namespace Vc

#endif  // VC_COMMON_VECTORABI_H_

// vim: foldmethod=marker
