#ifndef HAYNES_HPP_INCLUDED
#define HAYNES_HPP_INCLUDED


namespace rnd {

template <
	uint64_t a, uint64_t b
>
class Haynes {
public:
	Haynes( void ) { };
    ~Haynes( void ) { };

	Haynes( Haynes const & rng ) noexcept : m_rand( rng.m_rand ) { };

    Haynes &
    operator=( Haynes const & rng  ) noexcept {
        m_rand = rng.m_rand;
    	return *this;
    }

	void
	InitSeed( uint64_t seed ) noexcept {
		m_rand = a * seed + b;
	}

	uint32_t
	Rand32u( void ) noexcept {
		m_rand = ( a * m_rand + b );
        return m_rand & 0xFFFFFFFFUL;
	}

	int32_t
	Rand32( void ) noexcept {
		m_rand = ( a * m_rand + b ) & 0x7FFFFFFFUL;
		return m_rand;
	}

private:
	uint64_t m_rand;
};

} // rnd

#endif // HAYNES_HPP_INCLUDED
