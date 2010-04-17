/* Copyright 2010 SPARTA, Inc., dba Cobham Analytic Solutions
 * 
 * This file is part of WATCHER.
 * 
 *     WATCHER is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU Affero General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 * 
 *     WATCHER is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU Affero General Public License for more details.
 * 
 *     You should have received a copy of the GNU Affero General Public License
 *     along with Watcher.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef shared_stream_h
#define shared_stream_h

#include "logger.h"
#include "libwatcher/watcherMessageFwd.h"
#include "sharedStreamFwd.h"
#include "serverConnectionFwd.h"

#include <string>
#include <vector>
#include <stdint.h>
#include <boost/enable_shared_from_this.hpp>
#include <boost/scoped_ptr.hpp>

/*
namespace boost {
    namespace asio {
	class io_service;
    }
} */

namespace watcher {
class ReplayState; //fwd decl
class Watcherd;
class SharedStreamImpl;

/** Allows multiple clients to watch the same stream of live or replayed events. */
class SharedStream : public boost::enable_shared_from_this<SharedStream> {
    public:
	SharedStream(Watcherd&);
	~SharedStream();

	void seek(const event::SeekMessagePtr& m);
	void start();
	void stop();
	void speed(const event::SpeedMessagePtr& m);
	void range(event::PlaybackTimeRangeMessagePtr m);

	/** Add a client to the list which gets events for this stream. */
	void subscribe(ServerConnectionPtr);

	/** Remove a client to the list which gets events for this stream. */
	void unsubscribe(ServerConnectionPtr);

	/** send a message to all clients watching this stream. */
	void sendMessage(event::MessagePtr);

	/** send messages to all clients watching this stream. */
	void sendMessage(const std::vector<event::MessagePtr>&);

	/// state variables for Live and Replay tracking
	bool isPlaying_;
	bool isLive_;
	std::string description_;

	uint32_t getUID() const;

    private:
	boost::scoped_ptr<SharedStreamImpl> impl_;

	DECLARE_LOGGER();
};

} // namespace

#endif
