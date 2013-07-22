#ifndef STDRAND_HPP_INCLUDED
#define STDRAND_HPP_INCLUDED

#include <AdvancedGraphicsConfig.hpp>

namespace rnd {

/* Not suitable for parallel applications.
 */
template < uint64_t max_instance >
class Standard {
public:
	Standard( void ) noexcept { m_instance++; };
	~Standard( void )noexcept  { };

	Standard( Standard const & rng ) noexcept {
        static_assert( m_instance < max_instance, "too many instatiation of standard RNG" );
	    m_instance++;
    }

    Standard &
    operator=( Standard const & rng  ) noexcept {
        m_instance++;
        return *this;
    }

	void
	InitSeed( uint64_t seed ) noexcept {
	    if ( m_instance == 1 ) {
            srand( seed );
	    }
	}

	uint32_t
	Rand32u( void ) noexcept {
		return std::rand( );
	}

	int32_t
	Rand32( void ) noexcept {
		return std::rand( );
	}

private:
    static int m_instance;
};

template <
    uint64_t max_instance
>
int Standard< max_instance >::m_instance = 1;

} // rnd

#endif // STDRAND_HPP_INCLUDED
