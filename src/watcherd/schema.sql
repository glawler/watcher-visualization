CREATE TABLE events (
	id 	INTEGER PRIMARY KEY,	-- unique event identifier
	t	INTEGER,		-- time at which event occurred (milliseconds)
	node	TEXT,			-- IP address of node
	data	BLOB			-- binary blob containing event payload
);

-- create an index on the time column to allow for speedier access
CREATE INDEX time ON events ( t ASC );
