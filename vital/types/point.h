/*ckwg +29
 * Copyright 2019 by Kitware, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 *  * Neither name of Kitware, Inc. nor the names of any contributors may be used
 *    to endorse or promote products derived from this software without specific
 *    prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * \file
 * \brief This file contains the interface to a simple point.
 *
 * This point class pairs a eigen vector with a covariance matrix.
 */

#ifndef KWIVER_VITAL_POINT_H_
#define KWIVER_VITAL_POINT_H_

#include <vital/vital_config.h>
#include <vital/vital_export.h>
#include <vital/types/covariance.h>
#include <vital/types/vector.h>

#include <memory>

namespace kwiver {
namespace vital {

template < unsigned N, typename T >
class point : protected Eigen::Matrix< T, N, 1 >
{
public:
  using vector_type = Eigen::Matrix< T, N, 1 >;
  using covariance_type = covariance_<N, float>;

  point() : vector_type{ vector_type::Zero() } {}
  point( vector_type v, covariance_type c )
    : vector_type{ v }, m_covariance{ c } {}
  virtual ~point() = default;

  using vector_type::vector_type;

  vector_type value() const { return *this; }
  void set_value(vector_type v) { *this = v; }

  covariance_type covariance() const { return m_covariance; }
  void set_covariance(covariance_type v)
  {
    m_covariance = v;
  }

protected:
  covariance_type m_covariance;
};

// Aliases for common types.
using point_2i = point< 2, int >;
using point_2d = point< 2, double >;
using point_2f = point< 2, float >;
using point_3d = point< 3, double >;
using point_3f = point< 3, float >;
using point_4d = point< 4, double >;
using point_4f = point< 4, float >;

VITAL_EXPORT ::std::ostream& operator<< ( ::std::ostream& str, point_2i const& obj );
VITAL_EXPORT ::std::ostream& operator<< ( ::std::ostream& str, point_2d const& obj );
VITAL_EXPORT ::std::ostream& operator<< ( ::std::ostream& str, point_2f const& obj );
VITAL_EXPORT ::std::ostream& operator<< ( ::std::ostream& str, point_3d const& obj );
VITAL_EXPORT ::std::ostream& operator<< ( ::std::ostream& str, point_3f const& obj );
VITAL_EXPORT ::std::ostream& operator<< ( ::std::ostream& str, point_4d const& obj );
VITAL_EXPORT ::std::ostream& operator<< ( ::std::ostream& str, point_4f const& obj );

} } // end namespace

#endif
