#ifndef CONGRUENCIAL_HPP_INCLUDED
#define CONGRUENCIAL_HPP_INCLUDED

#include <AdvancedGraphicsConfig.hpp>

namespace rnd {

template <
	uint64_t a
>
class ParkMiller {
public:
	ParkMiller( void ) noexcept { };
    ~ParkMiller( void ) noexcept { };

	ParkMiller( ParkMiller const & rng ) noexcept : m_rand( rng.m_rand )
	{
	};

    ParkMiller &
    operator=( ParkMiller const & rng  )
    {
        m_rand = rng.m_rand;
        return *this;
    }

	void
	InitSeed( uint64_t seed ) noexcept
	{
		uint32_t lower = a * ( seed & 0xFFFFUL );
		uint32_t upper = a * ( seed >> 16 );
		uint32_t middle = ( lower >> 16 ) + upper;
		lower = ( ( lower & 0xFFFFUL ) | ( ( middle & 0x7FFFUL ) << 16 ) ) +
			  ( middle >> 15 );
		if ( ( lower & 0x80000000UL ) != 0 ) {
				lower = ( lower + 1 ) & 0x7FFFFFFFUL;
		}
		m_rand = lower;
	}

	uint32_t
	Rand32u( void ) noexcept
	{
		uint32_t lower = a * ( m_rand & 0xFFFFUL );
		uint32_t upper = a * ( m_rand >> 16 );
		uint32_t middle = ( lower >> 16 ) + upper;
		lower = ( ( lower & 0xFFFFUL ) | ( ( middle & 0x7FFFUL ) << 16 ) ) +
			  ( middle >> 15 );
		m_rand = lower;
		return m_rand;
	}

	int32_t
	Rand32( void ) noexcept
	{
		uint32_t lower = a * ( m_rand & 0xFFFFUL );
		uint32_t upper = a * ( m_rand >> 16 );
		uint32_t middle = ( lower >> 16 ) + upper;
		lower = ( ( lower & 0xFFFFUL ) | ( ( middle & 0x7FFFUL ) << 16 ) ) +
			  ( middle >> 15 );
		if ( ( lower & 0x80000000UL ) != 0 ) {
				lower = ( lower + 1 ) & 0x7FFFFFFFUL;
		}
		m_rand = lower;
		return m_rand;
	}

private:
	uint32_t m_rand;
};

} // rnd

#endif
