#ifndef MICHELLMOORE_HPP_INCLUDED
#define MICHELLMOORE_HPP_INCLUDED

#include <AdvancedGraphicsConfig.hpp>

namespace rnd {

template <
	uint64_t MAX,
	uint64_t N_SEED
>
class MitchellMoore {
public:
	MitchellMoore( void ) noexcept :
		m_ptMM( 0 ), m_pt24MM( 0 ), m_pt55MM( 0 ) { };
    ~MitchellMoore( void ) { };

    MitchellMoore( MitchellMoore const & rng ) noexcept :
        m_ptMM( rng.m_ptMM ), m_pt24MM( rng.m_pt24MM ), m_pt55MM( rng.m_pt55MM )
    {
        for ( int i = 0; i < 0x40; ++i ) {
            m_rgiState[ i ] = rng.m_rgiState[ i ];
        }
    };

    MitchellMoore &
    operator=( MitchellMoore const & rng  ) noexcept {
        m_ptMM = rng.m_ptMM;
        m_pt24MM = rng.m_pt24MM;
        m_pt55MM = rng.m_pt55MM;
        for ( int i = 0; i < 0x40; ++i ) {
            m_rgiState[ i ] = rng.m_rgiState[ i ];
        }
        return *this;
    }

    void
    InitSeed( uint64_t seed ) noexcept {
		m_ptMM = 55;
		m_pt24MM = 31;
		m_pt55MM = 0;
		m_rgiState[ 0 ] = seed & MAX;
		/* Mersene Twister Init with control of values. */
		bool init = false;
		do {
			for ( uint64_t j = 1; j < 0x40; ++j ) {
				m_rgiState[ j ] = 0;
			}
			for ( uint64_t i = 1; i < 0x40; ++i ) {
				m_rgiState[ i ] = (
					N_SEED * ( m_rgiState[ i - 1 ] ^
					( m_rgiState[ i - 1 ] >> 30) ) + i
				);
				m_rgiState[ i ] &= MAX;
				init |= ( ( m_rgiState[ i ] % 2 ) == 1 );
			}
		} while( init == false );
	}

	uint32_t
	Rand32u( void ) noexcept {
		m_rgiState[ m_ptMM ] = m_rgiState[ m_pt24MM ] + m_rgiState[ m_pt55MM ];
		int last_m_ptMM = m_ptMM;

		m_ptMM   = ( m_ptMM   + 1 ) & 0x3F;
		m_pt24MM = ( m_pt24MM + 1 ) & 0x3F;
		m_pt55MM = ( m_pt55MM + 1 ) & 0x3F;

		return m_rgiState[ last_m_ptMM ] & 0xFFFFFFFFUL;
	}

	int32_t
	Rand32( void ) noexcept {
		m_rgiState[ m_ptMM ] = m_rgiState[ m_pt24MM ] + m_rgiState[ m_pt55MM ];
		int last_m_ptMM = m_ptMM;

		m_ptMM   = ( m_ptMM   + 1 ) & 0x3F;
		m_pt24MM = ( m_pt24MM + 1 ) & 0x3F;
		m_pt55MM = ( m_pt55MM + 1 ) & 0x3F;

		return m_rgiState[ last_m_ptMM ] & 0x7FFFFFFFUL;
	}

private:
	uint64_t m_rgiState[ 0x40 ];
	int m_ptMM;
	int m_pt24MM;
	int m_pt55MM;
};

} // rnd

#endif // MICHELLMOORE_HPP_INCLUDED
