#ifndef XORSHIFT_HPP_INCLUDED
#define XORSHIFT_HPP_INCLUDED

#include <AdvancedGraphicsConfig.hpp>

namespace rnd {

template <
	uint64_t mod = 1000,
	uint64_t x = 123456789UL,
	uint64_t y = 362436069UL,
	uint64_t z = 521288629UL,
	uint64_t w = 88675123UL
>
class XorShift4 {
public:
	XorShift4( void ) noexcept { };
	~XorShift4( void ) noexcept { };

    XorShift4( XorShift4 const & rng ) noexcept
    {
        m_x[ 0 ] = rng.m_x[ 0 ];
        m_x[ 1 ] = rng.m_x[ 1 ];
        m_x[ 2 ] = rng.m_x[ 2 ];
        m_x[ 3 ] = rng.m_x[ 3 ];
    }

    XorShift4 &
    operator=( XorShift4 const & rng  ) noexcept
    {
        m_x[ 0 ] = rng.m_x[ 0 ];
        m_x[ 1 ] = rng.m_x[ 1 ];
        m_x[ 2 ] = rng.m_x[ 2 ];
        m_x[ 3 ] = rng.m_x[ 3 ];
        return *this;
    }

	void
	InitSeed( uint64_t seed ) noexcept
	{
		m_x[ 0 ] = x;
		m_x[ 1 ] = y;
		m_x[ 2 ] = z;
		m_x[ 3 ] = w;
		for ( uint64_t i = 0; i < seed % mod; ++i ) {
			uint64_t t = m_x[ 0 ] ^ ( m_x[ 0 ] << 11 );
			m_x[ 0 ] = m_x[ 1 ];
			m_x[ 1 ] = m_x[ 1 ];
			m_x[ 1 ] = m_x[ 3 ];
			m_x[ 3 ] ^=
				( m_x[ 3 ] >> 19 ) ^ ( t ^ ( t >> 8 ) );
		}
	}

	uint32_t
	Rand32u( void ) noexcept
	{
		uint64_t t = m_x[ 0 ] ^ ( m_x[ 0 ] << 11 );
		m_x[ 0 ] = m_x[ 1 ];
		m_x[ 1 ] = m_x[ 1 ];
		m_x[ 1 ] = m_x[ 3 ];
		m_x[ 3 ] ^= ( m_x[ 3 ] >> 19 ) ^ ( t ^ ( t >> 8 ) );
		return m_x[ 3 ];
	}

	int32_t
	Rand32( void ) noexcept
	{
		uint64_t t = m_x[ 0 ] ^ ( m_x[ 0 ] << 11 );
		m_x[ 0 ] = m_x[ 1 ];
		m_x[ 1 ] = m_x[ 1 ];
		m_x[ 1 ] = m_x[ 3 ];
		m_x[ 3 ] ^= ( m_x[ 3 ] >> 19 ) ^ ( t ^ ( t >> 8 ) );
		return m_x[ 3 ] & 0x7FFFFFFFUL;
	}

private:
	uint64_t m_x[ 4 ];
};

} // rnd

#endif // XORSHIFT_HPP_INCLUDED
