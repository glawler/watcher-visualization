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
#include <boost/thread.hpp>
#include <boost/enable_shared_from_this.hpp>

namespace boost {
    namespace asio {
	class io_service;
    }
}

namespace watcher {
class ReplayState; //fwd decl
class Watcherd;

class SharedStream : public boost::enable_shared_from_this<SharedStream> {
    public:
	SharedStream(Watcherd&, boost::asio::io_service&);

	// has the effect of automatically invoking .subscribe()
	SharedStream(Watcherd&, boost::asio::io_service&, ServerConnectionPtr);

	void seek(const event::SeekMessagePtr& m);
	void start();
	void stop();
	void speed(const event::SpeedMessagePtr& m);
	void range(event::PlaybackTimeRangeMessagePtr m);

	/// state variables for Live and Replay tracking
	bool isPlaying_;
	bool isLive_;
	boost::shared_ptr<ReplayState> replay;

	void subscribe(ServerConnectionPtr);
	void unsubscribe(ServerConnectionPtr);
	void sendMessage(event::MessagePtr);
	void sendMessage(const std::vector<event::MessagePtr>&);

	std::string description;
	Watcherd& watcher;

	uint32_t getUID() const { return uid; }

    private:
	boost::mutex lock;
	std::list<ServerConnectionWeakPtr> clients; // clients subscribed to this stream

	uint32_t uid;

	DECLARE_LOGGER();

	void init(boost::asio::io_service&);
};

} // namespace

#endif
