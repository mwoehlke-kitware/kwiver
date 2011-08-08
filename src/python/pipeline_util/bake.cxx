/*ckwg +5
 * Copyright 2011 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include <python/helpers/pystream.h>

#include <vistk/pipeline/pipeline.h>

#include <vistk/pipeline_util/pipe_bakery.h>
#include <vistk/pipeline_util/pipe_bakery_exception.h>

#include <boost/python.hpp>

#include <string>

/**
 * \file bake.cxx
 *
 * \brief Python bindings for baking pipelines.
 */

using namespace boost::python;

static vistk::pipeline_t bake_pipe_file(std::string const& path);
static vistk::pipeline_t bake_pipe(object stream, std::string const& inc_root = "");
static void translator(vistk::pipe_bakery_exception const& e);

BOOST_PYTHON_FUNCTION_OVERLOADS(bake_pipe_overloads, bake_pipe, 1, 2);

BOOST_PYTHON_MODULE(bake)
{
  register_exception_translator<
    vistk::pipe_bakery_exception>(translator);

  def("bake_pipe_file", &bake_pipe_file);
  def("bake_pipe", &bake_pipe);
  def("bake_pipe_blocks", &vistk::bake_pipe_blocks);
  def("extract_configuration", &vistk::extract_configuration);
}

vistk::pipeline_t
bake_pipe_file(std::string const& path)
{
  return vistk::bake_pipe_from_file(boost::filesystem::path(path));
}

vistk::pipeline_t
bake_pipe(object stream, std::string const& inc_root)
{
  pyistream istr(stream);

  return vistk::bake_pipe(istr, boost::filesystem::path(inc_root));
}

void
translator(vistk::pipe_bakery_exception const& e)
{
  PyErr_SetString(PyExc_RuntimeError, e.what());
}
