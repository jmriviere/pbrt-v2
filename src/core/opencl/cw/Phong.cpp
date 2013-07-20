#include <AdvancedGraphicsConfig.hpp>
#include <iostream>
#include <Phong.hpp>

namespace phong {

Phong::Phong( uint32_t s, float m_ks, float m_kd ) noexcept :
    m_ks( m_ks ), m_kd( m_kd ), m_s( s )
{
    m_samples = nullptr;
}

Phong::~Phong( ) {
    delete [] m_samples;
}

void
Phong::generateSamples( uint32_t n_samples, uint32_t seed ) noexcept
{
    if ( m_ks + m_kd > 1 ) {
        return;
    }
    m_samples = new ( std::nothrow ) obj::vect< float, 2 >[ n_samples ];
    if ( m_samples == nullptr ) {
        return;
    }
    rnd::Uniform< float, rnd::Haynes, 6364136223846793005UL, 1UL > rng( seed );
    for (uint32_t i = 0; i < n_samples; ++i)
    {
        sample( m_samples[ i ][ 0 ], m_samples[ i ][ 1 ], rng );
    }
}

void
Phong::sample( float& theta, float& phi,
    rnd::Uniform< float, rnd::Haynes, 6364136223846793005UL, 1UL >& rng )
noexcept {
    while ( true ) {
        float rand = rng( 0.f, 1.f );
        if ( ( rand >= 0 ) && ( rand < m_kd ) ) {
            diffuseSample( theta, phi, rng );
            return;
        }
        else if ( ( rand >= m_kd ) && ( rand < m_kd + m_ks ) ) {
            specularSample( theta, phi, rng );
            return;
        }
    }
}

void
Phong::diffuseSample( float& theta, float& phi,
    rnd::Uniform< float, rnd::Haynes, 6364136223846793005UL, 1UL >& rng )
noexcept {
    theta = std::acos( 1.f - std::sqrt( rng( 0.f, 1.f ) ) );
    phi = 2 * static_cast< float >( M_PI ) * rng( 0.f, 1.f );
}

void
Phong::specularSample( float& theta, float& phi,
    rnd::Uniform< float, rnd::Haynes, 6364136223846793005UL, 1UL >& rng )
noexcept {
    theta = std::acos(
        std::pow( 1.f - rng( 0.f, 1.f ), 1.f / ( m_s + 1.f ) )
    );
    phi = 2 * static_cast< float >( M_PI ) * rng( 0.f, 1.f );
}

} // Phong
