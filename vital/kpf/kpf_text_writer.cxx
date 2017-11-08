/*ckwg +29
 * Copyright 2017 by Kitware, Inc.
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
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
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
 * \brief Text writers.
 *
 * Deprecated in favor of YAML.
 *
 */

#include "kpf_text_writer.h"

kwiver::vital::kpf::private_endl_t kwiver::vital::kpf::record_text_writer::endl;

namespace kwiver {
namespace vital {
namespace kpf {

record_text_writer&
operator<<( record_text_writer& w, const private_endl_t& )
{
  w.s << std::endl;
  return w;
}

record_text_writer&
operator<<( record_text_writer& w, const writer< canonical::id_t >& io)
{
  w.s << "id" << io.domain << ": " << io.id.d << " ";
  return w;
}

record_text_writer&
operator<<( record_text_writer& w, const writer< canonical::bbox_t >& io)
{
  w.s << "g" << io.domain << ": " << io.box.x1 << " " << io.box.y1 << " " << io.box.x2 << " " << io.box.y2 << " ";
  return w;
}

record_text_writer&
operator<<( record_text_writer& w, const writer< canonical::timestamp_t >& io)
{
  w.s << "ts" << io.domain << ": " << io.ts.d << " ";
  return w;
}

record_text_writer&
operator<<( record_text_writer& w, const writer< canonical::kv_t >& io)
{
  w.s << "kv: " << io.kv.key << " " << io.kv.val << " ";
  return w;
}

record_text_writer&
operator<<( record_text_writer& w, const writer< canonical::conf_t >& io)
{
  w.s << "conf" << io.domain << ": " << io.conf.d << " ";
  return w;
}

record_text_writer&
operator<<( record_text_writer& w, const writer< canonical::poly_t >& io)
{
  w.s << "poly" << io.domain << ": " << io.poly.xy.size() << " ";
  for (const auto& p : io.poly.xy )
  {
    w.s << p.first << " " << p.second << " ";
  }
  return w;
}

record_text_writer&
operator<<( record_text_writer& w, const writer< canonical::meta_t >& io)
{
  w.s << "meta: " << io.meta.txt;
  return w;
}


} // ...kpf
} // ...vital
} // ...kwiver