#ifndef HDRIMAGE_HPP_INCLUDED
#define HDRIMAGE_HPP_INCLUDED

#include <AdvancedGraphicsConfig.hpp>
#include <Random.hpp>
#include <Sphere.hpp>
#include <BRDF.hpp>

#define MIN( x, y ) ( x ) < ( y ) ? ( x ) : ( y )

#define PFM_LITTLE_ENDIAN( x ) ( ( x ) < 0 )

#define HDR_LT 0.005
#define HDR_HT 0.92

namespace details {

// Swap n values from the parameter pack to an output iterator
template < typename OutputIterator >
void swap_n( size_t, OutputIterator )
{
}

template < typename OutputIterator, typename T, typename... Args >
void swap_n( size_t n, OutputIterator out, T const & value, Args... args )
{
  if ( n > 0 ) {
    std::swap( *out, *const_cast< T* >( &value ) );
    swap_n( n - 1, ++out, args... );
  }
}

// Swap n values from the parameter pack to an output iterator, starting at
// the "beginth" element
template < typename OutputIterator >
void swap_range( size_t, size_t, OutputIterator )
{
}

template < typename OutputIterator, typename T, typename... Args >
void swap_range(
    size_t begin, size_t size,
    OutputIterator out, T const & value, Args... args
) {
    if ( begin == 0 ) {
        swap_n( size, out, value, args... );
    }
    else {
        swap_range( begin - 1, size, out, args... );
    }
}

template < class CWF >
static void
merge_block(
    float pix_red  , float pix_green  , float pix_blue,
    float pix_n_red, float pix_n_green, float pix_n_blue,
    float & acc_wred, float & acc_wgreen, float & acc_wblue,
    float & acc_red , float & acc_green , float & acc_blue,
    uint32_t exposure_time, int32_t balance[ 3 ]
) noexcept {

    CWF w;
    if (
        ( pix_red > HDR_LT ) && ( pix_red < HDR_HT ) &&
        ( pix_red <= pix_n_red )
    ) {
        float w_red = w( pix_red );
        acc_red  += std::log( pix_red / exposure_time ) * w_red;
        acc_wred += w_red;
    }
    else {
        balance[ 0 ] += ( pix_red > HDR_LT ) - ( pix_red < HDR_HT );
    }
    if (
        ( pix_green > HDR_LT ) && ( pix_green < HDR_HT ) &&
        ( pix_green <= pix_n_green )
    ) {
        float w_green = w( pix_green );
        acc_green  += std::log( pix_green / exposure_time ) * w_green;
        acc_wgreen += w_green;
    }
    else {
        balance[ 1 ] += ( pix_green > HDR_LT ) - ( pix_green < HDR_HT );
    }
    if (
        ( pix_blue > HDR_LT ) && ( pix_blue < HDR_HT ) &&
        ( pix_blue <= pix_n_blue )
    ) {
        float w_blue = w( pix_blue );
        acc_blue  += std::log( pix_blue / exposure_time ) * w_blue;
        acc_wblue += w_blue;
    }
    else {
        balance[ 2 ] += ( pix_blue > HDR_LT ) - ( pix_blue < HDR_HT );
    }
}

} // namespace details

#define B_SIZE 8

namespace hdr {

typedef struct {
    /* Define a block of pixel to vectorize code.
     * AVX format
     */
    float r[ 8 ];
    float g[ 8 ];
    float b[ 8 ];
} pixelBlock;


enum saveFormat : uint32_t {
    ASCIIBitmap    = '1',
    ASCIIGreymap   = '2',
    ASCIIColormap  = '3',
    BinaryBitmap   = '4',
    BinaryGreymap  = '5',
    BinaryColormap = '6'
};

#if defined( INTEL_CXX_COMPILER )
enum chanel : uint32_t {
    red   = 0,
    blue  = 1,
    green = 2,
    all   = 3
};
#else
enum class chanel : uint32_t {
    red   = 0,
    blue  = 1,
    green = 2,
    all   = 3
};
#endif

class image {

/* Row major matrix of pixelBlock */
public:
    image( void ) noexcept;
    image( uint32_t, uint32_t, float = 1 ) noexcept;
    image( image const &  ) noexcept;
    image( image       && ) noexcept;

    image & operator= ( image const &  ) noexcept;
    image & operator= ( image       && ) noexcept;

    image & create( uint32_t, uint32_t, float = 1 ) noexcept;
    image & copy  ( image const & ) noexcept;

    image & swap( image & ) noexcept;

    INLINE uint32_t getWidth     ( void ) const noexcept;
    INLINE uint32_t getHeight    ( void ) const noexcept;
    INLINE size_t   getPadd      ( void ) const noexcept;
    INLINE float    getMaxChanel ( void ) const noexcept;
    INLINE float    getMinChanel ( void ) const noexcept;

    INLINE void setMaxChanel( float ) noexcept;
    INLINE void setMinChanel( float ) noexcept;

    void updateMaxChanel( void ) noexcept;
    void updateMinChanel( void ) noexcept;

    INLINE bool isEmpty( void ) const noexcept;

    void free ( void ) noexcept;

    void fill     ( float val      ) noexcept;
    void normalise( float = 1      ) noexcept;
    void troncate ( float, float   ) noexcept;
    void gamma    ( float          ) noexcept;

    float maxPixelValue( chanel c = chanel::all ) const noexcept;
    float minPixelValue( chanel c = chanel::all ) const noexcept;
    float dynamicRange ( chanel c = chanel::all ) const noexcept;

    INLINE
    void setPixel(
        uint32_t, uint32_t, float  , float  , float
    )  noexcept;
    INLINE
    void getPixel(
        uint32_t, uint32_t, float &, float &, float &
    ) const noexcept;

    void
    negatif ( void ) noexcept;

    obj::vect< uint32_t, 2 >*
    sampleEM(
        uint32_t,
        rnd::Uniform< float, rnd::Haynes, 6364136223846793005UL, 1UL > & rng,
        obj::vect< uint32_t, 2 >* = nullptr, float* = nullptr, float** = nullptr
    ) const noexcept;

    void median  ( image const &, uint32_t = 3 ) noexcept;
    void median  (                uint32_t = 3 ) noexcept;

    void circle(
        obj::sphere const & s,
        float, float, float
    ) noexcept;
    void line(
        uint32_t, uint32_t, uint32_t, uint32_t,
        float, float, float
    ) noexcept;
    void circleFilled(
        obj::sphere const & s,
        float, float, float
    ) noexcept;

    int32_t loadPNM(
        std::string const &
    ) noexcept;
    int32_t savePNM(
        std::string const &, saveFormat = BinaryColormap
    ) const noexcept;

    int32_t loadPFM(
        std::string const &
    ) noexcept;
    int32_t savePFM(
        std::string const &, saveFormat = BinaryColormap
    ) const noexcept;

    template < class CWF, class... Args >
    int32_t createHDR(
        float const *, image const & im1, Args const &... args
    ) noexcept;
    template < class CWF >
    int32_t createHDR(
        float const *, image const *, uint32_t
    ) noexcept;

    void YCDF( float* , uint32_t, uint32_t           ) const noexcept;
    void XCDF( float* , uint32_t, uint32_t, uint32_t ) const noexcept;
    void XCDF( float**,           uint32_t, uint32_t ) const noexcept;

    void linearToneMap( float stops ) noexcept;
    void histEqToneMap( uint32_t = 256 ) noexcept;

    void reflectanceSphere(
        obj::sphere const &, obj::vect< float, 3 > const &
    ) noexcept;

    void latlong2sphere(
        obj::sphere const &, obj::vect< float, 3 > const &, image const &,
        uint32_t = 0, uint32_t = 0
    ) noexcept;

    void renderBiased(
        obj::sphere const &, image const &,
        obj::vect< float, 3 > const &, uint32_t,
        brdf::model const &,
        rnd::Uniform< float, rnd::Haynes, 6364136223846793005UL, 1UL > &
    ) noexcept;

    void render(
        obj::sphere const &, image const &,
        obj::vect< float, 3 > const &, uint32_t,
        brdf::model const &,
        rnd::Uniform< float, rnd::Haynes, 6364136223846793005UL, 1UL > &
    ) noexcept;

    float integrate( void ) const noexcept;

    ~image( void ) noexcept;

protected:
    void alloc( uint32_t, uint32_t ) noexcept;

    void plot8Points(
        uint32_t, uint32_t, uint32_t, uint32_t, float, float, float
    ) noexcept;
    void plot4Points(
        uint32_t, uint32_t, uint32_t, uint32_t, float, float, float
    ) noexcept;

    void plot4Lines(
        uint32_t, uint32_t, uint32_t, uint32_t, float, float, float
    ) noexcept;
    void plot2Lines(
        uint32_t, uint32_t, uint32_t, uint32_t, float, float, float
    ) noexcept;

private:
    uint32_t      m_width;
    uint32_t      m_height;
    size_t        m_padd;
    float         m_max_pixel_chanel;
    float         m_min_pixel_chanel;
    pixelBlock**  m_data_2D;
    pixelBlock*   m_data_1D;
};

class pol_cwf {
public:

    pol_cwf ( void ) = default;
    pol_cwf ( pol_cwf const & ) = default;
    ~pol_cwf( void ) = default;

    float operator( ) ( float x, float c1 = 0, float c2 = 1 ) {

        if ( ( x <= c1 ) || ( x >= c2 ) ) {
            return 0;
        }
        float c1_x  = c1 - x;
        float c2_x  = c2 - x;
        float c1_05 = ( c1 - c2 ) / 2;
        float c2_05 = ( c2 - c1 ) / 2;
        float norm = c1_05 * c1_05 * c2_05 * c2_05;
        return ( 1 / norm ) * c1_x * c1_x * c2_x * c2_x;
    }

private:
};

class trig_cwf {
public:

    trig_cwf ( void ) = default;
    trig_cwf ( trig_cwf const & ) = default;
    ~trig_cwf( void ) = default;

    float operator( ) ( float x ) {

        return static_cast< float >( std::sin( x * 2 * M_PI - M_PI / 2 ) + 1 );
    }

private:
};

uint32_t
image::getWidth( void ) const noexcept
{
    return m_width;
}

uint32_t
image::getHeight( void ) const noexcept
{
    return m_height;
}

size_t
image::getPadd( void ) const noexcept
{
    return m_padd;
}

bool
image::isEmpty( void ) const noexcept
{
    return m_data_1D == nullptr;
}

float
image::getMaxChanel( void ) const noexcept
{
    return m_max_pixel_chanel;
}

void
image::setMaxChanel( float val ) noexcept
{
    m_max_pixel_chanel = val;
}

float
image::getMinChanel( void ) const noexcept
{
    return m_min_pixel_chanel;
}

void
image::setMinChanel( float val ) noexcept
{
    m_min_pixel_chanel = val;
}

void
image::setPixel(
    uint32_t x, uint32_t y, float r, float g, float b
) noexcept {
    uint32_t x_block = x / 8;
    uint32_t x_index = x % 8;
    m_data_2D[ y ][ x_block ].r[ x_index ] = r;
    m_data_2D[ y ][ x_block ].g[ x_index ] = g;
    m_data_2D[ y ][ x_block ].b[ x_index ] = b;
}

void
image::getPixel(
    uint32_t x, uint32_t y, float & r, float & g, float & b
) const noexcept {
    uint32_t x_block = x / 8;
    uint32_t x_index = x % 8;
    r = m_data_2D[ y ][ x_block ].r[ x_index ];
    g = m_data_2D[ y ][ x_block ].g[ x_index ];
    b = m_data_2D[ y ][ x_block ].b[ x_index ];
}

template < class CWF, class... Args >
int32_t
image::createHDR(
    float const * exposure, image const & im1, Args const &... args
) noexcept {

    uint32_t constexpr N( sizeof... ( Args ) + 1 );
    std::array< hdr::image, N > image_set;

    details::swap_range( 0, N, image_set.begin( ), im1, args... );
    int32_t err = createHDR< CWF >( exposure, image_set.begin( ), N );
    details::swap_range( 0, N, image_set.begin( ), im1, args... );

    return err;
}

template < class CWF >
int32_t
image::createHDR(
    float const * exposure, image const * image_set, uint32_t N
) noexcept {

    if ( ( exposure == nullptr ) || ( image_set == nullptr ) ) {
       return -1;
    }
    uint32_t width  = image_set[ 0 ].m_width;
    uint32_t height = image_set[ 0 ].m_height;
    for ( uint32_t l = 1; l < N; ++l ) {
        if (
            ( image_set[ l ].m_width  != width ) ||
            ( image_set[ l ].m_height != height )
        ) {
            return -1;
        }
    }

    uint32_t width_block_number( ( width - 1 ) / 8 );
    uint32_t width_block_end( width_block_number * 8 );

    create( width, height, 1 );

#if defined( GNU_CXX_COMPILER )
#pragma omp parallel for
#endif
    for ( uint32_t i = 0; i < height; ++i ) {
        for ( uint32_t j = 0; j < width_block_number; ++j ) {
            float acc_red   [ 8 ] = { 0, 0, 0, 0, 0, 0, 0, 0 };
            float acc_green [ 8 ] = { 0, 0, 0, 0, 0, 0, 0, 0 };
            float acc_blue  [ 8 ] = { 0, 0, 0, 0, 0, 0, 0, 0 };
            float acc_wred  [ 8 ] = { 0, 0, 0, 0, 0, 0, 0, 0 };
            float acc_wgreen[ 8 ] = { 0, 0, 0, 0, 0, 0, 0, 0 };
            float acc_wblue [ 8 ] = { 0, 0, 0, 0, 0, 0, 0, 0 };
            int32_t balance[ 3 ] = { 0, 0, 0 };
            for ( uint32_t l = 0; l < N; ++l ) {
                if ( exposure[ l ] > 0 ) {
                    uint32_t ln = MIN( l + 1, N - 1 );
                    for ( uint32_t k = 0; k < 8; ++k ) {
                        details::merge_block< CWF >(
                            image_set[ l  ].m_data_2D[ i ][ j ].r[ k ],
                            image_set[ l  ].m_data_2D[ i ][ j ].g[ k ],
                            image_set[ l  ].m_data_2D[ i ][ j ].b[ k ],
                            image_set[ ln ].m_data_2D[ i ][ j ].r[ k ],
                            image_set[ ln ].m_data_2D[ i ][ j ].g[ k ],
                            image_set[ ln ].m_data_2D[ i ][ j ].b[ k ],
                            acc_wred[ k ], acc_wgreen[ k ], acc_wblue[ k ],
                            acc_red [ k ], acc_green [ k ], acc_blue [ k ],
                            exposure[ l ], balance
                        );
                    }
                }
            }
            for ( uint32_t k = 0; k < 8; ++k ) {
                m_data_2D[ i ][ j ].r[ k ] =
                    ( acc_wred  [ k ] == 0 ) ?
                    (
                        balance[ 0 ] > 0 ?
                        m_max_pixel_chanel : m_min_pixel_chanel
                    ) :
                    std::exp( acc_red  [ k ] / acc_wred  [ k ] );
                m_data_2D[ i ][ j ].g[ k ] =
                    ( acc_wgreen[ k ] == 0 ) ?
                    (
                        balance[ 1 ] > 0 ?
                        m_max_pixel_chanel : m_min_pixel_chanel
                    ) :
                    std::exp( acc_green[ k ] / acc_wgreen[ k ] );
                m_data_2D[ i ][ j ].b[ k ] =
                    ( acc_wblue[ k ] == 0 ) ?
                    (
                        balance[ 2 ] > 0 ?
                        m_min_pixel_chanel : m_min_pixel_chanel
                    ) :
                    std::exp( acc_blue[ k ] / acc_wblue[ k ] );
            }
        }
        float acc_red   [ 8 ] = { 0, 0, 0, 0, 0, 0, 0, 0 };
        float acc_green [ 8 ] = { 0, 0, 0, 0, 0, 0, 0, 0 };
        float acc_blue  [ 8 ] = { 0, 0, 0, 0, 0, 0, 0, 0 };
        float acc_wred  [ 8 ] = { 0, 0, 0, 0, 0, 0, 0, 0 };
        float acc_wgreen[ 8 ] = { 0, 0, 0, 0, 0, 0, 0, 0 };
        float acc_wblue [ 8 ] = { 0, 0, 0, 0, 0, 0, 0, 0 };
        int32_t balance[ 3 ] = { 0, 0, 0 };
        for ( uint32_t l = 0; l < N; ++l ) {
            if ( exposure[ l ] > 0 ) {
                uint32_t ln = MIN( l + 1, N - 1 );
                for ( uint32_t k = 0; width_block_end + k < width; ++k ) {
                    details::merge_block< CWF >(
                        image_set[ l  ].m_data_2D[ i ][ width_block_number ].r[ k ],
                        image_set[ l  ].m_data_2D[ i ][ width_block_number ].g[ k ],
                        image_set[ l  ].m_data_2D[ i ][ width_block_number ].b[ k ],
                        image_set[ ln ].m_data_2D[ i ][ width_block_number ].r[ k ],
                        image_set[ ln ].m_data_2D[ i ][ width_block_number ].g[ k ],
                        image_set[ ln ].m_data_2D[ i ][ width_block_number ].b[ k ],
                        acc_wred[ k ], acc_wgreen[ k ], acc_wblue[ k ],
                        acc_red [ k ], acc_green [ k ], acc_blue [ k ],
                        exposure[ l ], balance
                    );
                }
            }
        }
        for ( uint32_t k = 0; width_block_end + k < width; ++k ) {
                m_data_2D[ i ][ width_block_number ].r[ k ] =
                    ( acc_wred  [ k ] == 0 ) ?
                    (
                        balance[ 0 ] > 0 ?
                        m_max_pixel_chanel : m_min_pixel_chanel
                    ) :
                    std::exp( acc_red  [ k ] / acc_wred[ k ] );
                m_data_2D[ i ][ width_block_number ].g[ k ] =
                    ( acc_wgreen[ k ] == 0 ) ?
                    (
                        balance[ 1 ] > 0 ?
                        m_max_pixel_chanel : m_min_pixel_chanel
                    ) :
                    std::exp( acc_green[ k ] / acc_wgreen[ k ] );
                m_data_2D[ i ][ width_block_number ].b[ k ] =
                    ( acc_wblue[ k ] == 0 ) ?
                    (
                        balance[ 2 ] > 0 ?
                        m_max_pixel_chanel : m_min_pixel_chanel
                    ) :
                    std::exp( acc_blue[ k ] / acc_wblue[ k ] );
        }
    }
    return 0;
}

} // namespace hdr

namespace std {

static INLINE void
swap( hdr::image & im1, hdr::image & im2 ) noexcept
{
    im1.swap( im2 );
}

} // namespace std

#endif
