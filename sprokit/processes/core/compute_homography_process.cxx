// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include "compute_homography_process.h"

#include <vital/vital_types.h>
#include <vital/types/timestamp.h>
#include <vital/types/timestamp_config.h>
#include <vital/types/image_container.h>
#include <vital/types/feature_track_set.h>
#include <vital/types/homography.h>

#include <vital/algo/track_features.h>
#include <vital/algo/compute_ref_homography.h>

#include <kwiver_type_traits.h>

#include <sprokit/pipeline/process_exception.h>

namespace algo = kwiver::vital::algo;

namespace kwiver
{

create_algorithm_name_config_trait( homography_generator );

// ----------------------------------------------------------------
/**
 * \class compute_homography_process
 *
 * \brief Wrapper process around compute_ref_homography algorithm/
 *
 * \process This process instantiates a concrete implementation of the
 * \b compute_ref_homography algorithm.
 *
 * \iports
 *
 * \iport{timestamp} time stamp for incoming images.
 *
 * \iport{feature_track_set} track set to be used for calculating
 * homography.
 *
 * \oports
 *
 * \oport{homography_src_to_ref} Resulting homography.
 *
 * \configs
 *
 * \config{homography_generator} Algorithm name for homography generator to use.
 *
 * Other config parameters depend on the actual algorithm selected.
 */

//----------------------------------------------------------------
// Private implementation class
class compute_homography_process::priv
{
public:
  priv();
  ~priv();

  // Configuration values

  // There are many config items for the tracking and stabilization that go directly to
  // the algo.

  algo::compute_ref_homography_sptr m_compute_homog;
}; // end priv class

// ================================================================

compute_homography_process
::compute_homography_process( kwiver::vital::config_block_sptr const& config )
  : process( config ),
    d( new compute_homography_process::priv )//
{
  make_ports();
  make_config();
}

compute_homography_process
::~compute_homography_process()
{
}

// ----------------------------------------------------------------
void compute_homography_process
::_configure()
{
  scoped_configure_instrumentation();

  kwiver::vital::config_block_sptr algo_config = get_config();

  // Check config so it will give run-time diagnostic of config problems
  if ( ! algo::compute_ref_homography::check_nested_algo_configuration_using_trait(
         homography_generator,
         algo_config ) )
  {
    VITAL_THROW( sprokit::invalid_configuration_exception, name(), "Configuration check failed." );
  }

  algo::compute_ref_homography::set_nested_algo_configuration_using_trait(
    homography_generator,
    algo_config,
    d->m_compute_homog );
  if ( ! d->m_compute_homog )
  {
    VITAL_THROW( sprokit::invalid_configuration_exception, name(),
             "Unable to create compute_ref_homography" );
  }

  algo::compute_ref_homography::get_nested_algo_configuration_using_trait(
    homography_generator,
    algo_config,
    d->m_compute_homog );
}

// ----------------------------------------------------------------
void
compute_homography_process
::_step()
{
  kwiver::vital::f2f_homography_sptr src_to_ref_homography;

  kwiver::vital::timestamp frame_time = grab_from_port_using_trait( timestamp );
  vital::feature_track_set_sptr tracks = grab_from_port_using_trait( feature_track_set );

  {
    scoped_step_instrumentation();

    // LOG_DEBUG - this is a good thing to have in all processes that handle frames.
    LOG_DEBUG( logger(), "Processing frame " << frame_time );

    // Get stabilization homography
    src_to_ref_homography = d->m_compute_homog->estimate( frame_time.get_frame(), tracks );
  }

  // return by value
  push_to_port_using_trait( homography_src_to_ref, *src_to_ref_homography );
}

// ----------------------------------------------------------------
void compute_homography_process
::make_ports()
{
  // Set up for required ports
  sprokit::process::port_flags_t optional;
  sprokit::process::port_flags_t required;
  required.insert( flag_required );

  // -- input --
  declare_input_port_using_trait( timestamp, required );
  declare_input_port_using_trait( feature_track_set, required );

  // -- output --
  declare_output_port_using_trait( homography_src_to_ref, optional );
}

// ----------------------------------------------------------------
void compute_homography_process
::make_config()
{
  declare_config_using_trait( homography_generator );
}

// ================================================================
compute_homography_process::priv
::priv()
{
}

compute_homography_process::priv
::~priv()
{
}

} // end namespace
