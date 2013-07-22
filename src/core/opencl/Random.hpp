#ifndef RANDOM_HPP_INCLUDED
#define RANDOM_HPP_INCLUDED

#include <AdvancedGraphicsConfig.hpp>

#define RAND_PAR 3326976

namespace {

template < size_t size >
struct sa_realof_core {
    typedef float type;
};

template < >
struct sa_realof_core < 8 > {
    typedef double type;
};

template < >
struct sa_realof_core < 16 > {
    typedef long double type;
};

template < typename TE >
struct realof {
    typedef typename sa_realof_core< sizeof ( TE ) >::type type;
};

template < size_t size >
struct sa_intof_core {
    typedef uint32_t type;
};

template < >
struct sa_intof_core < 8 > {
    typedef uint64_t type;
};

template < typename TE >
struct intof {
    typedef typename sa_intof_core< sizeof ( TE ) >::type type;
};

}

namespace rnd {

template <
    uint64_t N = 624,
    uint64_t M = 397,
    uint64_t A = 0x9908B0DFUL,
    uint64_t UPPER_MASK = 0x80000000UL,
    uint64_t LOWER_MASK = 0x7FFFFFFFUL,
    uint64_t MAX = 0xFFFFFFFFFFFFFFFFUL,
    uint64_t N_SEED = 1812433253UL
>
class MerseneTwister;

template <
    uint64_t a = 6364136223846793005UL,
    uint64_t b = 1UL
>
class Haynes;

template <
    uint64_t a = 16807UL
>
class ParkMiller;

template < uint64_t max_instance = 1 >
class Standard;

template <
    uint64_t MAX = 0xFFFFFFFFFFFFFFFFUL,
    uint64_t N_SEED = 1812433253UL
>
class MitchellMoore;

template <
    typename TE,
    template < uint64_t ... > class RND = rnd::Haynes,
    uint64_t ... Params
>
class Density {
public:
    virtual void InitSeed( uint64_t seed ) = 0;
    virtual TE   Rand( void ) = 0;
private:
};

} // rnd

#define SA_REALOF( TE ) typename realof< TE >::type
#define SA_INTOF( TE )  typename  intof< TE >::type

#include <./Uniform.hpp>

#endif // RANDOM_HPP_INCLUDED
