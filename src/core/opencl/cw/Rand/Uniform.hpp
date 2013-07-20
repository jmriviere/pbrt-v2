#ifndef UNIFORM_HPP_INCLUDED
#define UNIFORM_HPP_INCLUDED

#include <AdvancedGraphicsConfig.hpp>
#include <StdRand/StdRand.hpp>
#include <Congruential/ParkMiller.hpp>
#include <Congruential/Haynes.hpp>
#include <LaggedFibonnacci/MichellMoore.hpp>
#include <MerseneTwister/MerseneTwister.hpp>
#include <XorShift/XorShift.hpp>

namespace rnd {

template <
    typename TE = float,
    template < uint64_t ... > class RND = rnd::Haynes,
    uint64_t ... Params
>
class Uniform : virtual private Density< TE, RND, Params... > {
public:
    Uniform( void ) noexcept : m_rng( )
    {
    }

    Uniform( uint64_t seed ) noexcept : m_rng( )
    {
        InitSeed( seed );
    }

    Uniform( TE s, TE e, uint64_t seed ) noexcept :
        m_start( s ), m_end( e )
    {
        InitSeed( seed );
    }

    Uniform( RND< Params ... > const & rng, TE s = 0, TE e = 1 ) noexcept :
         m_start( s ), m_end( e ), m_rng( rng )
    {

    }

    Uniform( Uniform const & rng ) noexcept  :
        m_start( rng.m_start ), m_end( rng.m_end ), m_rng( rng.m_rng )
    {

    }

    template <
        template < uint64_t ... Params_B > class RND_B
    >
    Uniform( RND_B< Params ... > const & rng, TE s = 0, TE e = 1 ) noexcept :
        m_rng( rng ), m_start( s ), m_end( e )
    {

    }

    ~Uniform( void ) { };

    Uniform & operator=( Uniform const & rng  ) noexcept {
        m_start = rng.m_start;
        m_end = rng.m_end;
        m_rng = rng.m_rng;
    }

    void
    InitSeed( uint64_t seed ) noexcept {
        m_rng.InitSeed( seed );
    }

    void
    Sup( TE end ) noexcept {
        m_end = end;
    }

    void
    Inf( TE start ) noexcept {
        m_start = m_start;
    }

    void
    Sup( void )  {
        return m_end;
    }

    void
    Inf( void ) noexcept {
        return m_start;
    }

    uint64_t
    GetSeed( void ) noexcept {
        return m_rng.Rand32u( );
    }

    uint32_t
    Rand32u( void ) noexcept {
        return m_rng.Rand32u( );
    }

    int32_t
    Rand32( void ) noexcept {
        return m_rng.Rand32( );
    }

    TE
    Rand( void ) noexcept {
        return ( m_end - m_start ) *
            static_cast< SA_REALOF( TE ) >( m_rng.Rand32( ) )
            / 0x7FFFFFFF + m_start;
    }

    TE
    Rand( TE s, TE e ) noexcept {
        return ( e - s ) *
            static_cast< SA_REALOF( TE ) >( m_rng.Rand32( ) ) / 0x7FFFFFFF + s;
    }

    TE
    operator ()( TE s, TE e ) noexcept {
        return ( e - s ) *
            static_cast< SA_REALOF( TE ) >( m_rng.Rand32( ) ) / 0x7FFFFFFF + s;
    }

    TE
    operator ()( void ) noexcept {
        return ( m_end - m_start ) *
            static_cast< SA_REALOF( TE ) >( m_rng.Rand32( ) )
            / 0x7FFFFFFF + m_start;
    }

    static int const n_args = 2;

private:
    TE m_start;
    TE m_end;
    RND< Params ... > m_rng;
};

} /* End namespace rnd (Rand). */

#endif // UNIFORM_HPP_INCLUDED
