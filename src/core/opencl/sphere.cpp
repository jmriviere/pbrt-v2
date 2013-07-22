#include <Sphere.hpp>

namespace obj {

sphere::sphere( void ) noexcept :
	m_radius( 0 )
{
    m_center.clear( );
}

sphere::sphere( uint32_t radius, vect< uint32_t, 3 > const & p ) noexcept :
	m_radius( radius ), m_center( p )
{

}

sphere::sphere(
    uint32_t radius, uint32_t center_x, uint32_t center_y, uint32_t center_z
) noexcept :
	m_radius( radius ), m_center( center_x, center_y, center_z )
{

}

sphere::sphere( sphere const &  m ) noexcept :
	m_radius( m.m_radius ), m_center( m.m_center )
{

}

sphere::sphere( sphere &&  m ) noexcept :
	m_radius( m.m_radius ), m_center( std::move( m.m_center ) )
{

}

sphere::~sphere( void ) noexcept
{

}



} //namespace obj
