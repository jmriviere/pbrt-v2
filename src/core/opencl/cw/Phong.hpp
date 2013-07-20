#ifndef PHONG_HPP_INCLUDED
#define PHONG_HPP_INCLUDED

#include <AdvancedGraphicsConfig.hpp>
#include <HDRimage.hpp>
#include <Sphere.hpp>
#include <Random.hpp>

namespace phong {

class Phong {

public:

    Phong( uint32_t, float = 0.5, float = 0.5 ) noexcept;
    ~Phong( ) noexcept;

    void generateSamples( uint32_t, uint32_t ) noexcept;

    INLINE uint32_t getTheta( uint32_t, uint32_t ) const noexcept;
    INLINE uint32_t getPhi  ( uint32_t, uint32_t ) const noexcept;

    INLINE bool isInitialised( void ) const noexcept;

private:

    void sample(
        float&, float&,
        rnd::Uniform< float, rnd::Haynes, 6364136223846793005UL, 1UL >&
    ) noexcept;
    void diffuseSample(
        float&, float&,
        rnd::Uniform< float, rnd::Haynes, 6364136223846793005UL, 1UL >&
    ) noexcept;
    void specularSample(
        float&, float&,
        rnd::Uniform< float, rnd::Haynes, 6364136223846793005UL, 1UL >&
    ) noexcept;

    float                  m_ks;
    float                  m_kd;
    uint32_t               m_s;
    obj::vect< float, 2 >* m_samples;
};

bool
Phong::isInitialised( void ) const noexcept
{
    return ( m_samples != nullptr );
}

uint32_t
Phong::getTheta( uint32_t i, uint32_t height ) const noexcept
{
    return static_cast< uint32_t >(
        m_samples[ i ][ 0 ] / static_cast< float >( M_PI ) * height
    );
}

uint32_t
Phong::getPhi( uint32_t i, uint32_t width ) const noexcept
{
    return static_cast< uint32_t >(
        m_samples[ i ][ 1 ] / ( 2 * static_cast< float >( M_PI ) ) * width
    );
}

}

#endif // PHONG_HPP_INCLUDED
