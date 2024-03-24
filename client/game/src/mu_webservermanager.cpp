#include "mu_precompiled.h"
#include "mu_webservermanager.h"
#include "mu_sessionmanager.h"
#include "mu_eventsmanager.h"
#include "mu_browsermanager.h"
#include "ui_noesisgui.h"
#include "ngui_context.h"

#if NEXTMU_HTTP_SERVER == 1
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/URI.h>

class NProcessingAuthResponseEvent : public NEventRequest
{
public:
    explicit NProcessingAuthResponseEvent(const Poco::URI &requestUri) : RequestURI(requestUri)
    {}

private:
    virtual void Process() override
	{
		MUSessionManager::ProcessAuthResponse(RequestURI);
    }

private:
    const Poco::URI RequestURI;
};

class NRequestHandler : public Poco::Net::HTTPRequestHandler
{
public:
    void handleInvalidMethod(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
	{
		response.setStatus(Poco::Net::HTTPResponse::HTTP_METHOD_NOT_ALLOWED);
		response.setContentType("text/html");

		auto &out = response.send();
		out << "<html style=\"background-color: white;\"><head><head><body><h1>Method Not Allowed</h1></body></html>";
		out.flush();
	}

	void handleNotFound(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response)
	{
		response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
		response.setContentType("text/html");

		auto &out = response.send();
		out << "<html style=\"background-color: white;\"><head><head><body><h1>Page Not Found</h1></body></html>";
		out.flush();
	}

    void handleAuthRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response, const Poco::URI &requestUri)
	{
        auto parameters = requestUri.getQueryParameters();
        auto authCode = std::find_if(parameters.begin(), parameters.end(), [](std::pair<std::string, std::string> &value) -> mu_boolean { return value.first.compare("code") == 0; });
        if (authCode == parameters.end() || MUSessionManager::CheckAuthCode(authCode->second) == false)
        {
            handleNotFound(request, response);
            return;
        }

        const mu_utf8string authUrl = MUSessionManager::GenerateAuthUrl();
        response.redirect(authUrl);
	}

	void handleAuthCallback(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response, const Poco::URI &requestUri)
	{
		response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
		response.setContentType("text/html");
		auto &out = response.send();
		out << "<html style=\"background-color: white;\"><head><head><body></body></html>";
		out.flush();

        MUEventsManager::AddEvent(std::make_unique<NProcessingAuthResponseEvent>(requestUri));
	}

    virtual void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
    {
		if (request.getMethod().compare("GET") != 0)
        {
            handleInvalidMethod(request, response);
            return;
        }

        const auto requestUri = Poco::URI(request.getURI());
        const auto requestPath = requestUri.getPath();

		if (requestPath.compare("/auth/login") == 0) handleAuthRequest(request, response, requestUri);
		else if (requestPath.compare("/auth/callback") == 0) handleAuthCallback(request, response, requestUri);
        else handleNotFound(request, response);
    }
};

class NRequestHandlerFactory : public Poco::Net::HTTPRequestHandlerFactory
{
public:
    virtual Poco::Net::HTTPRequestHandler *createRequestHandler(const Poco::Net::HTTPServerRequest &server) override
    {
        return new_nothrow NRequestHandler();
    }
};

namespace MUWebServerManager
{
    Poco::SharedPtr<Poco::Net::HTTPServer> Server;
    mu_utf8string BaseURL;

    const mu_boolean Initialize()
    {
        return true;
    }

    void Destroy()
    {
        StopListen();
    }

    const mu_boolean StartListen()
    {
        if (Server != nullptr) return true;

        Poco::SharedPtr<NRequestHandlerFactory> factory(new_nothrow NRequestHandlerFactory());
		if (factory == nullptr) return false;

		Poco::AutoPtr<Poco::Net::HTTPServerParams> serverParams(new_nothrow Poco::Net::HTTPServerParams());
		if (serverParams == nullptr) return false;
		serverParams->setMaxThreads(1);
		serverParams->setMaxQueued(5);

        Server.reset(new_nothrow Poco::Net::HTTPServer(factory, Poco::Net::ServerSocket(Poco::Net::SocketAddress("127.0.0.1", 0)), serverParams));
        if (Server == nullptr) return false;

        BaseURL = fmt::format("http://127.0.0.1:{}/", Server->port());
        Server->start();

        return true;
    }

    void StopListen()
    {
        if (Server == nullptr) return;
        Server->stopAll(true);
        Server.reset();
        BaseURL = "";
    }

    const mu_utf8string GetBaseURL()
    {
        return BaseURL;
    }
};
#endif