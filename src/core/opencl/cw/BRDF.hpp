#ifndef BRDF_HPP_INCLUDED
#define BRDF_HPP_INCLUDED

#include <AdvancedGraphicsConfig.hpp>
#include <Sphere.hpp>

namespace brdf {

class model {

public:
    model( void ) noexcept = delete;
    model( obj::vect< float, 3 >, float, float, float ) noexcept;
    model( float, float, float, float, float, float   ) noexcept;

    model( model const &  ) noexcept;
    model( model       && ) noexcept;

    ~model( void ) noexcept;

    INLINE void setS ( float ) noexcept;
    INLINE void setKS( float ) noexcept;
    INLINE void setKD( float ) noexcept;
    INLINE void setW0( obj::vect< float, 3 > const & ) noexcept;
    INLINE void setW0( float, float, float ) noexcept;

    INLINE float getS ( void ) const noexcept;
    INLINE float getKS( void ) const noexcept;
    INLINE float getKD( void ) const noexcept;

    INLINE obj::vect< float, 3 > const &
        getW0( void ) const noexcept;

    INLINE float
    phong(
        obj::vect< float, 3 > const & wi,
        obj::vect< float, 3 > const & wr,
        obj::vect< float, 3 > const & n
    ) const noexcept;

    INLINE float
    blinnPhong(
        obj::vect< float, 3 > const & wi,
        obj::vect< float, 3 > const & wr,
        obj::vect< float, 3 > const & n
    ) const noexcept;

    /* Add new models here */

private:
    obj::vect< float, 3 > m_w0;
    float                 m_s;
    float                 m_ks;
    float                 m_kd;
};

    void
    model::setS( float s ) noexcept {
        m_s = s;
    }

    void
    model::setKS( float ks ) noexcept {
        m_ks = ks;
    }

    void
    model::setKD( float kd ) noexcept {
        m_kd = kd;
    }

    void
    model::setW0( obj::vect< float, 3 > const & w0 ) noexcept {
        m_w0 = w0;
    }

    void
    model::setW0( float x, float y, float z ) noexcept {
        m_w0[ 0 ] = x;
        m_w0[ 1 ] = y;
        m_w0[ 2 ] = z;
    }

    float
    model::getS( void ) const noexcept {
        return m_s;
    }

    float
    model::getKS( void ) const noexcept {
        return m_ks;
    }

    float
    model::getKD( void ) const noexcept {
        return m_kd;
    }

    obj::vect< float, 3 > const &
    model::getW0( void ) const noexcept {
        return m_w0;
    }

    float
    model::phong(
        obj::vect< float, 3 > const & wi,
        obj::vect< float, 3 > const & wr,
        obj::vect< float, 3 > const & n
    ) const noexcept {
        return m_kd / static_cast< float >( M_PI ) +
            m_ks * std::pow( wr.dot( m_w0 ), m_s ) / n.dot( wi );
    }

    float
    model::blinnPhong(
        obj::vect< float, 3 > const & wi,
        obj::vect< float, 3 > const &   ,
        obj::vect< float, 3 > const & n
    ) const noexcept {
        obj::vect< float, 3 > wh;
        for ( uint32_t i = 0; i < 3; ++i ) {
            wh[ i ] = wi[ i ] + m_w0[ 0 ];
        }
        float norm = wh.norm( );
        for ( uint32_t i = 0; i < 3; ++i ) {
            wh[ i ] /= norm;
        }
        return m_kd / static_cast< float >( M_PI ) +
            m_ks * std::pow( n.dot( wh ), m_s ) / n.dot( wi );
    }

} // brdf

#endif // BRDF_HPP_INCLUDED