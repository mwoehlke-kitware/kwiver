// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/**
 * \file
 * \brief PROJ geo_conversion functor implementation
 */

#include "geo_conv.h"

#include <proj.h>

#include <memory>
#include <string>

namespace kwiver {

namespace arrows {

namespace proj {

namespace {

using props_t = std::unordered_map< std::string, std::string >;

// ----------------------------------------------------------------------------
struct proj_cleanup
{
  void
  operator()( PJ_CONTEXT* context ) const
  {
    proj_context_destroy( context );
  }

  void
  operator()( PJ* p ) const
  {
    proj_destroy( p );
  }
};

// ----------------------------------------------------------------------------
using pj_context_uptr_t = std::unique_ptr< PJ_CONTEXT, proj_cleanup >;
using pj_uptr_t = std::unique_ptr< PJ, proj_cleanup >;

// ----------------------------------------------------------------------------
PJ_CONTEXT*
context()
{
  static thread_local pj_context_uptr_t the_context;

  if( !the_context )
  {
    the_context.reset( proj_context_create() );
    proj_context_use_proj4_init_rules( the_context.get(), 1 );
  }

  return the_context.get();
}

// ----------------------------------------------------------------------------
pj_uptr_t
projection( int crs )
{
  auto const crs_str = std::to_string( crs );
  auto const arg = std::string{ "EPSG:" } + crs_str;
  auto const p = proj_create( context(), arg.c_str() );

  if( !p )
  {
    auto const msg =
      "Failed to construct PROJ projection for EPSG:" + crs_str;
    throw std::runtime_error( msg );
  }

  return pj_uptr_t{ p };
}

// ----------------------------------------------------------------------------
PJ*
projection( int crs_from, int crs_to )
{
  using key_t = std::pair< int, int >;

  struct hash_t
  {
    size_t // TODO(C++14) make this constexpr
    operator()( key_t const& key ) const
    {
      constexpr auto hash = std::hash< int64_t >{};

      return hash( static_cast< int64_t >( key.first ) << 32 | key.second );
    }
  };

  using map_t = std::unordered_map< key_t, pj_uptr_t, hash_t >;

  static thread_local map_t projections;

  auto const key = std::make_pair( crs_from, crs_to );
  auto const i = projections.find( key );

  if( i == projections.end() )
  {
    auto const crs_from_str = std::to_string( crs_from );
    auto const crs_to_str = std::to_string( crs_to );
    auto const arg_from = std::string{ "EPSG:" } + crs_from_str;
    auto const arg_to = std::string{ "EPSG:" } + crs_to_str;
    auto const p = proj_create_crs_to_crs( context(), arg_from.c_str(),
                                           arg_to.c_str(), nullptr );

    if( !p )
    {
      auto const msg =
        "Failed to construct PROJ projection"
        " from EPSG:" + crs_from_str;
        " to EPSG:" + crs_to_str;
      throw std::runtime_error( msg );
    }

#if PROJ_VERSION_MAJOR > 5
    // PROJ 6 sometimes swaps the coordinates from the conventional easting,
    // northing order; this extra step ensures that the coordinate order is
    // consistent
    auto const np = proj_normalize_for_visualization( context(), p );
    proj_destroy( p );

    if( !np )
    {
      auto const msg =
        "Failed to construct normalized PROJ projection"
        " from EPSG:" + crs_from_str;
        " to EPSG:" + crs_to_str;
      throw std::runtime_error( msg );
    }

    projections.emplace( key, np );
    return np;
#else
    projections.emplace( key, p );
    return p;
#endif
  }

  return i->second.get();
}

// ----------------------------------------------------------------------------
void
extract_props( props_t& props, PJ* proj )
{
  auto text =
    std::string{ proj_as_proj_string( context(), proj, PJ_PROJ_5, nullptr ) };

  while( text.size() )
  {
    auto const i = text.find( ' ' );
    auto tok = text.substr( 0, i );
    text = ( i == std::string::npos ? std::string{} : text.substr( i + 1 ) );

    if( tok.size() && tok[ 0 ] == '+' )
    {
      auto const j = tok.find( '=' );
      if( j == std::string::npos )
      {
        props.emplace( tok.substr( 1 ), std::string{} );
      }
      else
      {
        props.emplace( tok.substr( 1, j - 1 ), tok.substr( j + 1 ) );
      }
    }
  }
}

} // namespace <anonymous>

// ----------------------------------------------------------------------------
char const*
geo_conversion
::id() const
{
  return "proj";
}

// ----------------------------------------------------------------------------
vital::geo_crs_description_t
geo_conversion
::describe( int crs )
{
  static const auto prop_map =
    std::unordered_map< std::string, std::string >{
      { "datum", "datum" },
      { "ellps", "ellipse" },
      { "proj", "projection" },
      { "units", "units" },
    };

  // Get CRS init string
  auto const proj = projection( crs );

  // Parse init string into property key/value pairs
  auto props = props_t{};
  extract_props( props, proj.get() );
  extract_props( props, proj_get_ellipsoid( context(), proj.get() ) );

  // Convert to human-readable result
  vital::geo_crs_description_t result;
  for( auto const& item : props )
  {
    if( item.first == "zone" )
    {
      result.emplace( "zone", item.second );
      result.emplace( "hemisphere",
                      props.count( "south" ) ? "south" : "north" );
    }
    else
    {
      auto const prop_map_iter = prop_map.find( item.first );

      if( prop_map_iter != prop_map.end() )
      {
        result.emplace( prop_map_iter->second, item.second );
      }
    }
  }

  return result;
}

// ----------------------------------------------------------------------------
vital::vector_2d
geo_conversion
::operator()( vital::vector_2d const& point, int from, int to )
{
  auto const proj = projection( from, to );

  auto c = PJ_COORD{ { point[ 0 ], point[ 1 ], 0.0, 0.0 } };

  c = proj_trans( proj, PJ_FWD, c );
  if( auto const err = proj_errno( proj ) )
  {
    auto const msg =
      "PROJ conversion failed: error " + std::to_string( err ) +
      ": " + proj_errno_string( err );
    throw std::runtime_error( msg );
  }

  return { c.v[ 0 ], c.v[ 1 ] };
}

// ----------------------------------------------------------------------------
vital::vector_3d
geo_conversion
::operator()( vital::vector_3d const& point, int from, int to )
{
  auto const proj = projection( from, to );

  auto c = PJ_COORD{ { point[ 0 ], point[ 1 ], point[ 2 ], 0.0 } };

  c = proj_trans( proj, PJ_FWD, c );
  if( auto const err = proj_errno( proj ) )
  {
    auto const msg =
      "PROJ conversion failed: error " + std::to_string( err ) +
      ": " + proj_errno_string( err );
    throw std::runtime_error( msg );
  }

  return { c.v[ 0 ], c.v[ 1 ], c.v[ 2 ] };
}

} // namespace proj

} // namespace arrows

} // namespace kwiver
