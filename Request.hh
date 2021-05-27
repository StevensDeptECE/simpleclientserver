#pragma once

class Request {
public:
	virtual void handleServer(int returnsckt);
	virtual void handle(int returnsckt);
};
