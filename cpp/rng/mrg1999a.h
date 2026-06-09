/*
 * JaamSim Discrete Event Simulation
 * Copyright (C) 2014 Ausenco Engineering Canada Inc.
 * Copyright (C) 2018-2022 JaamSim Software Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Translated from Java: com.jaamsim.rng.MRG1999a
 * Combined MRG based on L'Ecuyer (1999a).
 * Ported from the ANSI C version provided in Simulation Modeling and
 * Analysis 5th Ed., Averill M. Law (Appendix 7B).
 */

#ifndef JAAMSIM_RNG_MRG1999A_H
#define JAAMSIM_RNG_MRG1999A_H

#include <array>
#include <cstdint>
#include <stdexcept>
#include <string>

namespace jaamsim::rng {

// ---------------------------------------------------------------------------
// MRG1999a — Combined Multiple Recursive Generator
//
// Generates U(0,1) random variates. Supports independent random streams
// and substreams via advance tables.
//
// Thread safety: each instance is independent. Instances are not safe for
// concurrent use from multiple threads without external locking.
// ---------------------------------------------------------------------------

class MRG1999a {
public:
    // -------------------------------------------------------------------
    // Construction
    // -------------------------------------------------------------------

    /// Default constructor: stream 0, substream 0.
    MRG1999a();

    /// Construct seeded from the given stream/substream.
    explicit MRG1999a(int stream, int substream);

    /// Construct with explicit seed values (must satisfy modulo constraints).
    MRG1999a(uint32_t s0, uint32_t s1, uint32_t s2,
             uint32_t s3, uint32_t s4, uint32_t s5);

    // -------------------------------------------------------------------
    // Seed control
    // -------------------------------------------------------------------

    /// Return the stream number used to seed this generator.
    int stream_number() const noexcept { return stream_; }

    /// Seed the generator from a stream/substream pair.
    void set_seed_stream(int stream, int substream);

    /// Seed the generator with explicit values.
    /// @throws std::invalid_argument if seed constraints are violated
    void set_seed(uint32_t s0, uint32_t s1, uint32_t s2,
                  uint32_t s3, uint32_t s4, uint32_t s5);

    // -------------------------------------------------------------------
    // Random variate generation
    // -------------------------------------------------------------------

    /// Return the next U(0,1) uniformly distributed double.
    double next_uniform();

    /// Return the internal state as a comma-separated string.
    std::string to_string() const;

private:
    // -------------------------------------------------------------------
    // Constants
    // -------------------------------------------------------------------

    static constexpr uint64_t m1   = 4294967087ULL;                // 2^32 - 209
    static constexpr uint64_t m2   = 4294944443ULL;                // 2^32 - 22853
    static constexpr double   norm = 2.328306549295727688e-10;     // 1.0 / (m1 + 1)

    // -------------------------------------------------------------------
    // State
    // -------------------------------------------------------------------

    uint32_t s0_ = 12345;
    uint32_t s1_ = 12345;
    uint32_t s2_ = 12345;
    uint32_t s3_ = 12345;
    uint32_t s4_ = 12345;
    uint32_t s5_ = 12345;

    int stream_ = -1;
    int substream_ = 0;
    std::array<uint32_t, 6> init_seeds_ = {{}};

    // -------------------------------------------------------------------
    // Advance tables (3 columns × 6 rows)
    // -------------------------------------------------------------------

    static constexpr std::array<std::array<uint64_t, 3>, 6> stream_advance_table = {{
        {{ 2427906178ULL, 3580155704ULL,  949770784ULL }},
        {{  226153695ULL, 1230515664ULL, 3580155704ULL }},
        {{ 1988835001ULL,  986791581ULL, 1230515664ULL }},
        {{ 1464411153ULL,  277697599ULL, 1610723613ULL }},
        {{   32183930ULL, 1464411153ULL, 1022607788ULL }},
        {{ 2824425944ULL,   32183930ULL, 2093834863ULL }}
    }};

    static constexpr std::array<std::array<uint64_t, 3>, 6> substream_advance_table = {{
        {{   82758667ULL, 1871391091ULL, 4127413238ULL }},
        {{ 3672831523ULL,   69195019ULL, 1871391091ULL }},
        {{ 3672091415ULL, 3528743235ULL,   69195019ULL }},
        {{ 1511326704ULL, 3759209742ULL, 1610795712ULL }},
        {{ 4292754251ULL, 1511326704ULL, 3889917532ULL }},
        {{ 3859662829ULL, 4292754251ULL, 3708466080ULL }}
    }};

    // -------------------------------------------------------------------
    // Seed cache
    // -------------------------------------------------------------------

    static constexpr int seed_cache_size      = 20;
    static constexpr int seed_cache_increment = 5000;

    /// Lazily initialized cache of seeds after every 'seed_cache_increment'
    /// stream advances. Entry [i] holds seeds for stream i * 5000.
    static const std::array<std::array<uint64_t, 6>, seed_cache_size>& seed_cache();

    // -------------------------------------------------------------------
    // Static helpers — advance logic
    // -------------------------------------------------------------------

    /// Unsigned modular multiplication that handles 64-bit wrapping,
    /// matching Java's signed-long overflow → unsigned conversion.
    static uint64_t ulong_mod(int64_t val, uint64_t mod) noexcept;

    /// Mix the first three seeds through one row of the advance table, mod m1.
    static uint64_t mix_half1(const uint64_t a[3], const uint64_t s[6]) noexcept;

    /// Mix the last three seeds through one row of the advance table, mod m2.
    static uint64_t mix_half2(const uint64_t a[3], const uint64_t s[6]) noexcept;

    /// Advance the seed vector by one stream.
    static void advance_stream(uint64_t seeds[6]) noexcept;

    /// Advance the seed vector by one substream.
    static void advance_substream(uint64_t seeds[6]) noexcept;
};

// ===========================================================================
// Inline / template implementations
// ===========================================================================

inline uint64_t MRG1999a::ulong_mod(int64_t val, uint64_t mod) noexcept {
    // Replicate Java's behaviour: convert a signed 64-bit long (which may have
    // wrapped on overflow) into an unsigned value before modulo.
    if (val < 0) {
        val &= INT64_MAX;                           // clear sign bit
        val = (val % static_cast<int64_t>(mod))
            - (INT64_MIN % static_cast<int64_t>(mod));
    }
    return static_cast<uint64_t>(val % static_cast<int64_t>(mod));
}

inline uint64_t MRG1999a::mix_half1(const uint64_t a[3],
                                     const uint64_t s[6]) noexcept {
    // Use 128-bit intermediates to avoid unsigned overflow in the sum.
    // Falls back to Java-style signed wrapping on compilers without __int128.
#if defined(__SIZEOF_INT128__) || defined(__int128)
    __uint128_t tmp = static_cast<__uint128_t>(a[0]) * s[0];
    tmp = static_cast<__uint128_t>(a[1]) * s[1] + tmp;
    tmp = static_cast<__uint128_t>(a[2]) * s[2] + tmp;
    return static_cast<uint64_t>(tmp % m1);
#else
    int64_t tmp = ulong_mod(static_cast<int64_t>(
        static_cast<uint64_t>(a[0]) * static_cast<uint64_t>(s[0])), m1);
    tmp = ulong_mod(static_cast<int64_t>(
        static_cast<uint64_t>(a[1]) * static_cast<uint64_t>(s[1])) + tmp, m1);
    tmp = ulong_mod(static_cast<int64_t>(
        static_cast<uint64_t>(a[2]) * static_cast<uint64_t>(s[2])) + tmp, m1);
    return static_cast<uint64_t>(tmp);
#endif
}

inline uint64_t MRG1999a::mix_half2(const uint64_t a[3],
                                     const uint64_t s[6]) noexcept {
#if defined(__SIZEOF_INT128__) || defined(__int128)
    __uint128_t tmp = static_cast<__uint128_t>(a[0]) * s[3];
    tmp = static_cast<__uint128_t>(a[1]) * s[4] + tmp;
    tmp = static_cast<__uint128_t>(a[2]) * s[5] + tmp;
    return static_cast<uint64_t>(tmp % m2);
#else
    int64_t tmp = ulong_mod(static_cast<int64_t>(
        static_cast<uint64_t>(a[0]) * static_cast<uint64_t>(s[3])), m2);
    tmp = ulong_mod(static_cast<int64_t>(
        static_cast<uint64_t>(a[1]) * static_cast<uint64_t>(s[4])) + tmp, m2);
    tmp = ulong_mod(static_cast<int64_t>(
        static_cast<uint64_t>(a[2]) * static_cast<uint64_t>(s[5])) + tmp, m2);
    return static_cast<uint64_t>(tmp);
#endif
}

inline void MRG1999a::advance_stream(uint64_t seeds[6]) noexcept {
    const uint64_t s0 = mix_half1(stream_advance_table[0].data(), seeds);
    const uint64_t s1 = mix_half1(stream_advance_table[1].data(), seeds);
    const uint64_t s2 = mix_half1(stream_advance_table[2].data(), seeds);

    const uint64_t s3 = mix_half2(stream_advance_table[3].data(), seeds);
    const uint64_t s4 = mix_half2(stream_advance_table[4].data(), seeds);
    const uint64_t s5 = mix_half2(stream_advance_table[5].data(), seeds);

    seeds[0] = s0; seeds[1] = s1; seeds[2] = s2;
    seeds[3] = s3; seeds[4] = s4; seeds[5] = s5;
}

inline void MRG1999a::advance_substream(uint64_t seeds[6]) noexcept {
    const uint64_t s0 = mix_half1(substream_advance_table[0].data(), seeds);
    const uint64_t s1 = mix_half1(substream_advance_table[1].data(), seeds);
    const uint64_t s2 = mix_half1(substream_advance_table[2].data(), seeds);

    const uint64_t s3 = mix_half2(substream_advance_table[3].data(), seeds);
    const uint64_t s4 = mix_half2(substream_advance_table[4].data(), seeds);
    const uint64_t s5 = mix_half2(substream_advance_table[5].data(), seeds);

    seeds[0] = s0; seeds[1] = s1; seeds[2] = s2;
    seeds[3] = s3; seeds[4] = s4; seeds[5] = s5;
}

// ===========================================================================
// Constructor implementations
// ===========================================================================

inline MRG1999a::MRG1999a() {
    set_seed_stream(0, 0);
}

inline MRG1999a::MRG1999a(int stream, int substream) {
    set_seed_stream(stream, substream);
}

inline MRG1999a::MRG1999a(uint32_t s0, uint32_t s1, uint32_t s2,
                           uint32_t s3, uint32_t s4, uint32_t s5) {
    set_seed(s0, s1, s2, s3, s4, s5);
}

// ===========================================================================
// seed_cache — lazy static initialisation
// ===========================================================================

inline const std::array<std::array<uint64_t, 6>, MRG1999a::seed_cache_size>&
MRG1999a::seed_cache() {
    static const auto cache = [] {
        std::array<std::array<uint64_t, 6>, seed_cache_size> table{};
        std::array<uint64_t, 6> seeds = {{ 12345, 12345, 12345,
                                           12345, 12345, 12345 }};

        for (int i = 0; i <= (seed_cache_size - 1) * seed_cache_increment; ++i) {
            if (i % seed_cache_increment == 0) {
                int idx = i / seed_cache_increment;
                for (int j = 0; j < 6; ++j)
                    table[static_cast<size_t>(idx)][j] = seeds[j];
            }
            advance_stream(seeds.data());
        }
        return table;
    }();
    return cache;
}

// ===========================================================================
// set_seed — validate and store seeds
// ===========================================================================

inline void MRG1999a::set_seed(uint32_t s0, uint32_t s1, uint32_t s2,
                                uint32_t s3, uint32_t s4, uint32_t s5) {
    if (s0 == 0 && s1 == 0 && s2 == 0)
        throw std::invalid_argument(
            "MRG1999a: The first three seeds cannot all be 0");
    if (s3 == 0 && s4 == 0 && s5 == 0)
        throw std::invalid_argument(
            "MRG1999a: The last three seeds cannot all be 0");
    if (s0 >= m1 || s1 >= m1 || s2 >= m1)
        throw std::invalid_argument(
            "MRG1999a: The first three seeds must be < " + std::to_string(m1));
    if (s3 >= m2 || s4 >= m2 || s5 >= m2)
        throw std::invalid_argument(
            "MRG1999a: The last three seeds must be < " + std::to_string(m2));

    s0_ = s0; s1_ = s1; s2_ = s2;
    s3_ = s3; s4_ = s4; s5_ = s5;
}

// ===========================================================================
// set_seed_stream
// ===========================================================================

inline void MRG1999a::set_seed_stream(int stream, int substream) {
    if (stream < 0)
        throw std::invalid_argument(
            "MRG1999a: Stream numbers must be positive");
    if (substream < 0)
        throw std::invalid_argument(
            "MRG1999a: Substream numbers must be positive");

    std::array<uint64_t, 6> seeds{};
    int init_substream = 0;

    // If the same stream is used, start from the saved substream
    if (stream == stream_ && substream >= substream_) {
        init_substream = substream_;
        for (int j = 0; j < 6; ++j)
            seeds[j] = init_seeds_[j];
    }
    else {
        // Find the cached seed with stream closest to, but not exceeding,
        // the requested stream.
        int cache_idx = std::min(stream / seed_cache_increment,
                                 seed_cache_size - 1);
        for (int j = 0; j < 6; ++j)
            seeds[j] = seed_cache()[static_cast<size_t>(cache_idx)][j];

        for (int i = cache_idx * seed_cache_increment; i < stream; ++i)
            advance_stream(seeds.data());
    }

    for (int i = init_substream; i < substream; ++i)
        advance_substream(seeds.data());

    set_seed(static_cast<uint32_t>(seeds[0]),
             static_cast<uint32_t>(seeds[1]),
             static_cast<uint32_t>(seeds[2]),
             static_cast<uint32_t>(seeds[3]),
             static_cast<uint32_t>(seeds[4]),
             static_cast<uint32_t>(seeds[5]));

    // Save the initial state
    stream_    = stream;
    substream_ = substream;
    for (int j = 0; j < 6; ++j)
        init_seeds_[j] = static_cast<uint32_t>(seeds[j]);
}

// ===========================================================================
// next_uniform — core generation algorithm
// ===========================================================================

inline double MRG1999a::next_uniform() {
    // Mix the first half of the state
    int64_t p1 = 1403580LL * static_cast<int64_t>(s1_)
               -  810728LL * static_cast<int64_t>(s0_);
    p1 = p1 % static_cast<int64_t>(m1);
    if (p1 < 0) p1 += static_cast<int64_t>(m1);
    s0_ = s1_; s1_ = s2_; s2_ = static_cast<uint32_t>(p1);

    // Mix the second half of the state
    int64_t p2 = 527612LL * static_cast<int64_t>(s5_)
               - 1370589LL * static_cast<int64_t>(s3_);
    p2 = p2 % static_cast<int64_t>(m2);
    if (p2 < 0) p2 += static_cast<int64_t>(m2);
    s3_ = s4_; s4_ = s5_; s5_ = static_cast<uint32_t>(p2);

    int64_t p = p1 - p2;
    if (p <= 0) p += static_cast<int64_t>(m1);
    return static_cast<double>(p) * norm;
}

// ===========================================================================
// to_string
// ===========================================================================

inline std::string MRG1999a::to_string() const {
    return std::to_string(s0_) + ", " + std::to_string(s1_) + ", "
         + std::to_string(s2_) + ", " + std::to_string(s3_) + ", "
         + std::to_string(s4_) + ", " + std::to_string(s5_);
}

}  // namespace jaamsim::rng

#endif  // JAAMSIM_RNG_MRG1999A_H
