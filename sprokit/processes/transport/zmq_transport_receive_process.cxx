/*ckwg +29
 * Copyright 2018 by Kitware, Inc.
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

#include "zmq_transport_receive_process.h"

#include <sprokit/pipeline/process_exception.h>

#include <kwiver_type_traits.h>

namespace kwiver {

// (config-key, value-type, default-value, description )
create_config_trait( port, int, "5550",
                     "Port number to connect/bind to");

create_config_trait( num_publishers, int, "1",
                     "Number of publishers to subscribe to. ");

//----------------------------------------------------------------
// Private implementation class
class zmq_transport_receive_process::priv
{
public:
  priv();
  ~priv();
  void connect();

  // Configuration values
  int m_port;
  int m_num_publishers;

  //+ any other connection related data goes here
  zmq::context_t m_context;
  zmq::socket_t m_sub_socket;
  std::vector< std::shared_ptr<zmq::socket_t> > m_sync_sockets;

  vital::logger_handle_t m_logger; // for logging in priv methods

}; // end priv class

// ================================================================

zmq_transport_receive_process
::zmq_transport_receive_process( kwiver::vital::config_block_sptr const& config )
  : process( config ),
    d( new zmq_transport_receive_process::priv )
{
  make_ports();
  make_config();
  d->m_logger = logger();
}


zmq_transport_receive_process
::~zmq_transport_receive_process()
{
}


// ----------------------------------------------------------------
void zmq_transport_receive_process
::_configure()
{
  scoped_configure_instrumentation();

  // Get process config entries
  d->m_port = config_value_using_trait( port );
  d->m_num_publishers = config_value_using_trait( num_publishers );

  int major, minor, patch;
  zmq_version(&major, &minor, &patch);
  LOG_DEBUG( logger(), "ZeroMQ Version: " << major << "." << minor << "." << patch );
}

// ----------------------------------------------------------------
void zmq_transport_receive_process
::_init()
{
  d->connect();
}


// ----------------------------------------------------------------
void zmq_transport_receive_process
::_step()
{
  LOG_TRACE( logger(), "Waiting for datagram..." );

  //+ Get binary buffer from port and assign to serialized_message
	zmq::message_t datagram;
	d->m_sub_socket.recv(&datagram);
  auto msg = std::make_shared< std::string >(static_cast<char *>(datagram.data()), datagram.size());
  LOG_TRACE( logger(), "Received datagram of size " << msg->size() );

  // We know that the message is a pointer to a std::string
  push_to_port_using_trait( serialized_message, msg );
}


// ----------------------------------------------------------------
void zmq_transport_receive_process
::make_ports()
{
  // Set up for required ports
  sprokit::process::port_flags_t required;
  required.insert( flag_required );

  declare_output_port_using_trait( serialized_message, required );
}


// ----------------------------------------------------------------
void zmq_transport_receive_process
::make_config()
{
  declare_config_using_trait( port );
  declare_config_using_trait( num_publishers );
}


// ================================================================
zmq_transport_receive_process::priv
::priv() : m_context( 1 ), m_sub_socket( m_context, ZMQ_SUB )
{
}

void
zmq_transport_receive_process::priv
::connect()
{
  LOG_DEBUG( m_logger, "Number of publishers " << m_num_publishers );
  m_sub_socket.setsockopt(ZMQ_SUBSCRIBE,"",0);

  // We start with our base port.  Even ports are the pub/sub socket
  // Odd ports (pub/sub + 1) are the sync sockets
  for ( int i = 0; i < m_num_publishers * 2; i+=2 )
  {
    std::shared_ptr< zmq::socket_t > sync_socket = std::make_shared< zmq::socket_t >( m_context, ZMQ_REQ );
    m_sync_sockets.push_back(sync_socket);

    std::ostringstream sub_connect_string;
    sub_connect_string << "tcp://localhost:" << m_port + i;
    LOG_TRACE( m_logger, "SUB Connect for " << sub_connect_string.str() );
    m_sub_socket.connect( sub_connect_string.str() );

    std::ostringstream sync_connect_string;
    sync_connect_string << "tcp://localhost:" << ( m_port + i + 1);
    LOG_TRACE( m_logger, "SYNC Connect for " << sync_connect_string.str() );
    sync_socket->connect( sync_connect_string.str() );

    zmq::message_t datagram;
    sync_socket->send(datagram);

    zmq::message_t datagram_i;
    LOG_TRACE( m_logger, "Waiting for SYNC reply, pub: " << i << " at " << m_port + i + 1 );
    sync_socket->recv(&datagram_i);
    LOG_TRACE( m_logger, "SYNC reply received, pub: " << i << " at " <<  m_port + i + 1 );
  }
}

zmq_transport_receive_process::priv
::~priv()
{
}

} // end namespace
