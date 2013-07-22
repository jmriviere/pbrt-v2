#ifndef MERSENETWISTER_HPP_INCLUDED
#define MERSENETWISTER_HPP_INCLUDED

#include <AdvancedGraphicsConfig.hpp>

namespace rnd {

template <
    uint64_t N, uint64_t M, uint64_t A,
    uint64_t UPPER_MASK, uint64_t LOWER_MASK,
    uint64_t MAX, uint64_t N_SEED
>
class MerseneTwister {
public:
    MerseneTwister( void ) noexcept : m_pos_pool( 0 ) { };
    ~MerseneTwister( void ) noexcept { };

    MerseneTwister( MerseneTwister const & rng ) noexcept : m_pos_pool( rng.m_pos_pool )
    {
        for ( uint64_t i = 0; i < N; ++i ) {
            m_x[ i ] = rng.m_x[ i ];
        }
    };

    MerseneTwister &
    operator=( MerseneTwister const & rng  ) noexcept {
        m_pos_pool = rng.m_pos_pool;
        for ( uint64_t i = 0; i < N; ++i ) {
            m_x[ i ] = rng.m_x[ i ];
        }
        return *this;
    }

    void
    InitSeed( uint64_t seed ) noexcept {
        m_x[ 0 ] = seed & MAX;
        for ( uint64_t j = 1; j < N; ++j ) {
            m_x[ j ] = 0;
        }
        for ( uint64_t i = 1; i < N; ++i ) {
            m_x[ i ] = (
                N_SEED * ( m_x[ i - 1 ] ^
                ( m_x[ i - 1 ] >> 30) ) + i
            );
            m_x[ i ] &= MAX;
        }
    }

    uint32_t
    Rand32u( void ) noexcept {
        uint64_t rand;
        if ( m_pos_pool == N ) {
            uint64_t a;
            for ( uint64_t i = 0; i < N - 1; ++i ) {
                rand = ( m_x[ i ] & UPPER_MASK ) |
                       ( m_x[ i + 1 ] & LOWER_MASK );
                a = ( ( rand & 0x1UL ) ? A : 0x0UL );
                m_x[ i ] =
                    m_x[ ( i + M ) % N ] ^ ( rand >> 1 ) ^ a;
            }
            rand = ( m_x[ N - 1 ] & UPPER_MASK ) |
                   ( m_x[ 0 ] & LOWER_MASK );
            a = ( ( rand & 0x1UL ) ? A : 0x0UL );
            m_x[ N - 1 ] = m_x[ M - 1 ] ^ ( rand >> 1 ) ^ a;
            m_pos_pool = 0;
        }
        rand = m_x[ m_pos_pool++ ];
        rand ^= ( rand >> 11 );
        rand ^= ( rand << 7 ) & 0x9D2C5680UL;
        rand ^= ( rand << 15 ) & 0xEFC60000UL;
        rand ^= ( rand >> 18 );
        return rand;
    }

    int32_t
    Rand32( void ) noexcept {
        uint64_t rand;
        if ( m_pos_pool == N ) {
            uint64_t a;
            for ( uint64_t i = 0; i < N - 1; ++i ) {
                rand = ( m_x[ i ] & UPPER_MASK ) |
                       ( m_x[ i + 1 ] & LOWER_MASK );
                a = ( ( rand & 0x1UL ) ? A : 0x0UL );
                m_x[ i ] =
                    m_x[ ( i + M ) % N ] ^ ( rand >> 1 ) ^ a;
            }
            rand = ( m_x[ N - 1 ] & UPPER_MASK ) |
                   ( m_x[ 0 ] & LOWER_MASK );
            a = ( ( rand & 0x1UL ) ? A : 0x0UL );
            m_x[ N - 1 ] = m_x[ M - 1 ] ^ ( rand >> 1 ) ^ a;
            m_pos_pool = 0;
        }
        rand = m_x[ m_pos_pool++ ];
        rand ^= ( rand >> 11 );
        rand ^= ( rand << 7 ) & 0x9D2C5680UL;
        rand ^= ( rand << 15 ) & 0xEFC60000UL;
        rand ^= ( rand >> 18 );
        return rand & 0x7FFFFFFFUL;;
    }

private:
    uint64_t m_x[ N ];
    int m_pos_pool;
};

} // rnd

#endif // MERSENETWISTER_HPP_INCLUDED
