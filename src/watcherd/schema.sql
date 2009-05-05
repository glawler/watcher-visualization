CREATE TABLE events (
	ts	INTEGER NOT NULL,	-- time at which event occurred (milliseconds)
        evtype  INTEGER NOT NULL,       -- event type
	node	TEXT NOT NULL,		-- IP address of node
	data	BLOB NOT NULL		-- binary blob containing event payload
);

-- create an index on the timestamp column to allow for speedier access
CREATE INDEX time ON events ( ts ASC );
