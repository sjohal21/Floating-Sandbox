#include "Utils.h"

#include <GameLib/SysSpecifics.h>

#include <benchmark/benchmark.h>

#include <algorithm>
#include <limits>

static constexpr size_t SampleSize = 20000000;

static void VectorNormalization_Naive_NoLength(benchmark::State& state) 
{
    auto const size = MakeSize(SampleSize);

    std::vector<vec2f> points;
    std::vector<Spring> springs;
    MakeGraph(size, points, springs);

    std::vector<vec2f> results;
    results.resize(size);
    
    for (auto _ : state)
    {
        for (size_t i = 0; i < size; ++i)
        {
            vec2f const posA = points[springs[i].PointAIndex];
            vec2f const posB = points[springs[i].PointBIndex];
            vec2f v(posB - posA);
            vec2f const normalizedVector = v.normalise();

            results[i] = normalizedVector;
        }
    }

    benchmark::DoNotOptimize(results);
}
BENCHMARK(VectorNormalization_Naive_NoLength);

static void VectorNormalization_Naive_NoLength_RestrictPointers(benchmark::State& state)
{
    auto const size = MakeSize(SampleSize);

    std::vector<vec2f> points;
    std::vector<Spring> springs;
    MakeGraph(size, points, springs);

    std::vector<vec2f> results;
    results.resize(size);

    vec2f const * restrict pointData = points.data();
    Spring const * restrict springData = springs.data();
    vec2f * restrict resultData = results.data();

    for (auto _ : state)
    {
        for (size_t i = 0; i < size; ++i)
        {
            vec2f const posA = pointData[springData[i].PointAIndex];
            vec2f const posB = pointData[springData[i].PointBIndex];
            vec2f v(posB - posA);
            vec2f const normalizedVector = v.normalise();

            resultData[i] = normalizedVector;
        }
    }

    benchmark::DoNotOptimize(results);
}
BENCHMARK(VectorNormalization_Naive_NoLength_RestrictPointers);

static void VectorNormalization_Naive_AndLength_RestrictPointers(benchmark::State& state)
{
    auto const size = MakeSize(SampleSize);

    std::vector<vec2f> points;
    std::vector<Spring> springs;
    MakeGraph(size, points, springs);

    std::vector<vec2f> results;
    results.resize(size);

    std::vector<float> lengths;
    lengths.resize(size);

    vec2f const * restrict pointData = points.data();
    Spring const * restrict springData = springs.data();
    vec2f * restrict resultData = results.data();
    float * restrict lengthData = lengths.data();

    for (auto _ : state)
    {
        for (size_t i = 0; i < size; ++i)
        {
            vec2f const posA = pointData[springData[i].PointAIndex];
            vec2f const posB = pointData[springData[i].PointBIndex];
            vec2f v(posB - posA);
            float length = v.length();
            vec2f const normalizedVector = v.normalise(length);

            resultData[i] = normalizedVector;
            lengthData[i] = length;
        }
    }

    benchmark::DoNotOptimize(results);
    benchmark::DoNotOptimize(lengths);
}
BENCHMARK(VectorNormalization_Naive_AndLength_RestrictPointers);

////////////////////////////////////////////////////////////////////////////////////////

// TODO: move to GameLib's LibSimdPp.h
#define SIMDPP_ARCH_X86_SSSE3
#include "simdpp/simd.h"

static void VectorNormalization_Vectorized_AndLength_VSizeGnostic_Load1(benchmark::State& state)
{
    auto const size = MakeSize(SampleSize);

    std::vector<vec2f> points;
    std::vector<Spring> springs;
    MakeGraph(size, points, springs);

    std::vector<vec2f> results;
    results.resize(size);

    std::vector<float> lengths;
    lengths.resize(size);

    vec2f const * restrict pointData = points.data();
    Spring const * restrict springData = springs.data();
    vec2f * restrict resultData = results.data();
    float * restrict lengthData = lengths.data();

    for (auto _ : state)
    {
        for (size_t s = 0; s < size; s += 4)
        {
            simdpp::float32<4> pAx = simdpp::make_float(
                pointData[springData[s + 0].PointAIndex].x,
                pointData[springData[s + 1].PointAIndex].x,
                pointData[springData[s + 2].PointAIndex].x,
                pointData[springData[s + 3].PointAIndex].x);

            simdpp::float32<4> pAy = simdpp::make_float(
                pointData[springData[s + 0].PointAIndex].y,
                pointData[springData[s + 1].PointAIndex].y,
                pointData[springData[s + 2].PointAIndex].y,
                pointData[springData[s + 3].PointAIndex].y);

            simdpp::float32<4> pBx = simdpp::make_float(
                pointData[springData[s + 0].PointBIndex].x,
                pointData[springData[s + 1].PointBIndex].x,
                pointData[springData[s + 2].PointBIndex].x,
                pointData[springData[s + 3].PointBIndex].x);

            simdpp::float32<4> pBy = simdpp::make_float(
                pointData[springData[s + 0].PointBIndex].y,
                pointData[springData[s + 1].PointBIndex].y,
                pointData[springData[s + 2].PointBIndex].y,
                pointData[springData[s + 3].PointBIndex].y);

            simdpp::float32<4> displacementX = pBx - pAx;
            simdpp::float32<4> displacementY = pBy - pAy;

            simdpp::float32<4> _dx = displacementX * displacementX;
            simdpp::float32<4> _dy = displacementY * displacementY;

            simdpp::float32<4> _dxy = _dx + _dy;

            simdpp::float32<4> springLength = simdpp::sqrt(_dxy);

            displacementX = displacementX / springLength;
            displacementY = displacementY / springLength;

            simdpp::mask_float32<4> validMask = (springLength != 0.0f);

            displacementX = simdpp::bit_and(displacementX, validMask);
            displacementY = simdpp::bit_and(displacementY, validMask);

            simdpp::store(lengthData + s, springLength);
            simdpp::store_packed2(resultData + s, displacementX, displacementY);
        }
    }

    benchmark::DoNotOptimize(results);
    benchmark::DoNotOptimize(lengths);
}
BENCHMARK(VectorNormalization_Vectorized_AndLength_VSizeGnostic_Load1);

static void VectorNormalization_Vectorized_AndLength_VSizeGnostic_Load2(benchmark::State& state)
{
    auto const size = MakeSize(SampleSize);

    std::vector<vec2f> points;
    std::vector<Spring> springs;
    MakeGraph(size, points, springs);

    std::vector<vec2f> results;
    results.resize(size);

    std::vector<float> lengths;
    lengths.resize(size);

    vec2f const * restrict pointData = points.data();
    Spring const * restrict springData = springs.data();
    vec2f * restrict resultData = results.data();
    float * restrict lengthData = lengths.data();

    for (auto _ : state)
    {
        for (size_t s = 0; s < size; s += 4)
        {
            float pAx_[4] = {
                pointData[springData[s + 0].PointAIndex].x,
                pointData[springData[s + 1].PointAIndex].x,
                pointData[springData[s + 2].PointAIndex].x,
                pointData[springData[s + 3].PointAIndex].x
            };

            simdpp::float32<4> pAx = simdpp::load(pAx_);

            float pAy_[4] = {
                pointData[springData[s + 0].PointAIndex].y,
                pointData[springData[s + 1].PointAIndex].y,
                pointData[springData[s + 2].PointAIndex].y,
                pointData[springData[s + 3].PointAIndex].y
            };

            simdpp::float32<4> pAy = simdpp::load(pAy_);

            float pBx_[4] = {
                pointData[springData[s + 0].PointBIndex].x,
                pointData[springData[s + 1].PointBIndex].x,
                pointData[springData[s + 2].PointBIndex].x,
                pointData[springData[s + 3].PointBIndex].x
            };

            simdpp::float32<4> pBx = simdpp::load(pBx_);

            float pBy_[4] = {
                pointData[springData[s + 0].PointBIndex].y,
                pointData[springData[s + 1].PointBIndex].y,
                pointData[springData[s + 2].PointBIndex].y,
                pointData[springData[s + 3].PointBIndex].y
            };

            simdpp::float32<4> pBy = simdpp::load(pBy_);



            simdpp::float32<4> displacementX = pBx - pAx;
            simdpp::float32<4> displacementY = pBy - pAy;

            simdpp::float32<4> _dx = displacementX * displacementX;
            simdpp::float32<4> _dy = displacementY * displacementY;

            simdpp::float32<4> _dxy = _dx + _dy;

            simdpp::float32<4> springLength = simdpp::sqrt(_dxy);

            displacementX = displacementX / springLength;
            displacementY = displacementY / springLength;

            simdpp::mask_float32<4> validMask = (springLength != 0.0f);

            displacementX = simdpp::bit_and(displacementX, validMask);
            displacementY = simdpp::bit_and(displacementY, validMask);

            simdpp::store(lengthData + s, springLength);
            simdpp::store_packed2(resultData + s, displacementX, displacementY);
        }
    }

    benchmark::DoNotOptimize(results);
    benchmark::DoNotOptimize(lengths);
}
BENCHMARK(VectorNormalization_Vectorized_AndLength_VSizeGnostic_Load2);

static void VectorNormalization_Vectorized_AndLength_VSizeGnostic_LoadInstrinsics(benchmark::State& state)
{
    auto const size = MakeSize(SampleSize);

    std::vector<vec2f> points;
    std::vector<Spring> springs;
    MakeGraph(size, points, springs);

    std::vector<vec2f> results;
    results.resize(size);

    std::vector<float> lengths;
    lengths.resize(size);

    vec2f const * restrict pointData = points.data();
    Spring const * restrict springData = springs.data();
    vec2f * restrict resultData = results.data();
    float * restrict lengthData = lengths.data();

    for (auto _ : state)
    {
        for (size_t s = 0; s < size; s += 4)
        {
            __m128 vecA0 = _mm_castpd_ps(_mm_load_sd(reinterpret_cast<double const *>(pointData + springData[s + 0].PointAIndex)));
            __m128 vecA1 = _mm_castpd_ps(_mm_load_sd(reinterpret_cast<double const *>(pointData + springData[s + 1].PointAIndex)));
            __m128 vecA01 = _mm_movelh_ps(vecA0, vecA1);

            __m128 vecB0 = _mm_castpd_ps(_mm_load_sd(reinterpret_cast<double const *>(pointData + springData[s + 0].PointBIndex)));
            __m128 vecB1 = _mm_castpd_ps(_mm_load_sd(reinterpret_cast<double const *>(pointData + springData[s + 1].PointBIndex)));
            __m128 vecB01 = _mm_movelh_ps(vecB0, vecB1);

            __m128 vecA2 = _mm_castpd_ps(_mm_load_sd(reinterpret_cast<double const *>(pointData + springData[s + 2].PointAIndex)));
            __m128 vecA3 = _mm_castpd_ps(_mm_load_sd(reinterpret_cast<double const *>(pointData + springData[s + 3].PointAIndex)));
            __m128 vecA23 = _mm_movelh_ps(vecA2, vecA3);

            __m128 vecB2 = _mm_castpd_ps(_mm_load_sd(reinterpret_cast<double const *>(pointData + springData[s + 2].PointBIndex)));
            __m128 vecB3 = _mm_castpd_ps(_mm_load_sd(reinterpret_cast<double const *>(pointData + springData[s + 3].PointBIndex)));
            __m128 vecB23 = _mm_movelh_ps(vecB2, vecB3);

            simdpp::float32<4> displacement01 = simdpp::float32<4>(vecB01) - simdpp::float32<4>(vecA01); // x0,y0,x1,y1
            simdpp::float32<4> displacement23 = simdpp::float32<4>(vecB23) - simdpp::float32<4>(vecA23); // x2,y2,x3,y3

            simdpp::float32<4> displacementX = simdpp::unzip4_lo(displacement01, displacement23); // x0,x1,x2,x3
            simdpp::float32<4> displacementY = simdpp::unzip4_hi(displacement01, displacement23); // y0,y1,y2,y3


            simdpp::float32<4> const springLength = simdpp::sqrt(displacementX * displacementX + displacementY * displacementY);

            displacementX = displacementX / springLength;
            displacementY = displacementY / springLength;

            simdpp::mask_float32<4> const validMask = (springLength != 0.0f);

            displacementX = simdpp::bit_and(displacementX, validMask);
            displacementY = simdpp::bit_and(displacementY, validMask);

            simdpp::store(lengthData + s, springLength);
            simdpp::store_packed2(resultData + s, displacementX, displacementY);
        }
    }

    benchmark::DoNotOptimize(results);
    benchmark::DoNotOptimize(lengths);
}
BENCHMARK(VectorNormalization_Vectorized_AndLength_VSizeGnostic_LoadInstrinsics);