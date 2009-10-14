/* Copyright 2009 SPARTA, Inc., dba Cobham Analytic Solutions
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
/**
 * @file messageStatus.h
 * @author Geoff Lawler <geoff.lawler@cobham.com> 
 * @date 2009-07-15
 */
#ifndef STATUS_MESSAGE_H
#define STATUS_MESSAGE_H

#include "message.h"

namespace watcher {
    namespace event {
        class MessageStatus : public Message {
            public:

                typedef enum 
                {
                    status_ok,
                    status_error,
                    status_ack,
                    status_nack,
                    status_disconnected
                } Status;

                Status status;

                MessageStatus(const Status stat=status_ok);
                MessageStatus(const MessageStatus &other);

                virtual ~MessageStatus();

                bool operator==(const MessageStatus &other) const;
                MessageStatus &operator=(const MessageStatus &other);

                virtual std::ostream &toStream(std::ostream &out) const;
                std::ostream &operator<<(std::ostream &out) const { return toStream(out); }

                static const char *statusToString(const Status &stat);

            protected:
            private:
                DECLARE_LOGGER();

        };

        typedef boost::shared_ptr<MessageStatus> MessageStatusPtr;
        std::ostream &operator<<(std::ostream &out, const MessageStatus &mess);

    }
}

#endif // STATUS_MESSAGE_H
