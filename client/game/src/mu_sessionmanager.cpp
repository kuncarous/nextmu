#include "mu_precompiled.h"
#include "mu_sessionmanager.h"
#include "mu_webservermanager.h"
#include "mu_browsermanager.h"
#include "ui_noesisgui.h"
#include "ngui_context.h"
#include "mu_webmanager.h"
#include "web_filedownload.h"
#include <Poco/Base64Encoder.h>
#include <Poco/Base64Decoder.h>
#include <Poco/Random.h>
#include <Poco/SHA2Engine.h>
#include <Poco/URI.h>
#include <Poco/Net/HTMLForm.h>
#include <jwt-cpp/jwt.h>

#define OIDC_MTLS_ENDPOINT_ALIASES_FIELDS \
	JSON_STRING_FIELD(token_endpoint); \
	JSON_STRING_FIELD(revocation_endpoint); \
	JSON_STRING_FIELD(introspection_endpoint); \
	JSON_STRING_FIELD(device_authorization_endpoint); \
	JSON_STRING_FIELD(registration_endpoint); \
	JSON_STRING_FIELD(userinfo_endpoint); \
	JSON_STRING_FIELD(pushed_authorization_request_endpoint); \
	JSON_STRING_FIELD(backchannel_authentication_endpoint); \

#define OIDC_CONFIGURATION_FIELDS \
    JSON_BOOLEAN_FIELD(frontchannel_logout_session_supported); \
    JSON_BOOLEAN_FIELD(frontchannel_logout_supported); \
    JSON_BOOLEAN_FIELD(claims_parameter_supported); \
    JSON_BOOLEAN_FIELD(request_parameter_supported); \
    JSON_BOOLEAN_FIELD(request_uri_parameter_supported); \
    JSON_BOOLEAN_FIELD(require_request_uri_registration); \
    JSON_BOOLEAN_FIELD(tls_client_certificate_bound_access_tokens); \
    JSON_BOOLEAN_FIELD(backchannel_logout_supported); \
    JSON_BOOLEAN_FIELD(backchannel_logout_session_supported); \
    JSON_BOOLEAN_FIELD(require_pushed_authorization_requests); \
    JSON_BOOLEAN_FIELD(authorization_response_iss_parameter_supported); \
    JSON_STRING_FIELD(issuer); \
    JSON_STRING_FIELD(authorization_endpoint); \
    JSON_STRING_FIELD(token_endpoint); \
    JSON_STRING_FIELD(introspection_endpoint); \
    JSON_STRING_FIELD(userinfo_endpoint); \
    JSON_STRING_FIELD(end_session_endpoint); \
    JSON_STRING_FIELD(jwks_uri); \
    JSON_STRING_FIELD(check_session_iframe); \
    JSON_STRING_FIELD(registration_endpoint); \
    JSON_STRING_FIELD(revocation_endpoint); \
    JSON_STRING_FIELD(device_authorization_endpoint); \
    JSON_STRING_FIELD(backchannel_authentication_endpoint); \
    JSON_STRING_FIELD(pushed_authorization_request_endpoint); \
    JSON_ARRAY_STRING_FIELD(grant_types_supported); \
    JSON_ARRAY_STRING_FIELD(acr_values_supported); \
    JSON_ARRAY_STRING_FIELD(response_types_supported); \
    JSON_ARRAY_STRING_FIELD(subject_types_supported); \
    JSON_ARRAY_STRING_FIELD(id_token_signing_alg_values_supported); \
    JSON_ARRAY_STRING_FIELD(id_token_encryption_alg_values_supported); \
    JSON_ARRAY_STRING_FIELD(id_token_encryption_enc_values_supported); \
    JSON_ARRAY_STRING_FIELD(userinfo_signing_alg_values_supported); \
    JSON_ARRAY_STRING_FIELD(userinfo_encryption_alg_values_supported); \
    JSON_ARRAY_STRING_FIELD(userinfo_encryption_enc_values_supported); \
    JSON_ARRAY_STRING_FIELD(request_object_signing_alg_values_supported); \
    JSON_ARRAY_STRING_FIELD(request_object_encryption_alg_values_supported); \
    JSON_ARRAY_STRING_FIELD(request_object_encryption_enc_values_supported); \
    JSON_ARRAY_STRING_FIELD(response_mode_supported); \
    JSON_ARRAY_STRING_FIELD(token_endpoint_auth_methods_supported); \
    JSON_ARRAY_STRING_FIELD(token_endpoint_auth_signing_alg_values_supported); \
    JSON_ARRAY_STRING_FIELD(introspection_endpoint_auth_methods_supported); \
    JSON_ARRAY_STRING_FIELD(introspection_endpoint_auth_signing_alg_values_supported); \
    JSON_ARRAY_STRING_FIELD(authorization_signing_alg_values_supported); \
    JSON_ARRAY_STRING_FIELD(authorization_encryption_alg_values_supported); \
    JSON_ARRAY_STRING_FIELD(authorization_encryption_enc_values_supported); \
    JSON_ARRAY_STRING_FIELD(claims_supported); \
    JSON_ARRAY_STRING_FIELD(claim_types_supported); \
    JSON_ARRAY_STRING_FIELD(scopes_supported); \
    JSON_ARRAY_STRING_FIELD(code_challenge_methods_supported); \
    JSON_ARRAY_STRING_FIELD(revocation_endpoint_auth_methods_supported); \
    JSON_ARRAY_STRING_FIELD(revocation_endpoint_auth_signing_alg_values_supported); \
    JSON_ARRAY_STRING_FIELD(backchannel_token_delivery_modes_supported); \
    JSON_ARRAY_STRING_FIELD(backchannel_authentication_request_signing_alg_values_supported); \
    JSON_OBJECT_FIELD(OIDCMtlsEndpointAliases, mtls_endpoint_aliases); \

#define OIDC_TOKEN_ENDPOINT_RESPONSE_FIELDS \
	JSON_STRING_FIELD(token_type); \
	JSON_STRING_FIELD(access_token); \
	JSON_STRING_FIELD(id_token); \
	JSON_STRING_FIELD(refresh_token); \
	JSON_STRING_FIELD(scope); \

#define JSON_BOOLEAN_FIELD(name) mu_boolean name
#define JSON_INT32_FIELD(name) mu_int32 name
#define JSON_STRING_FIELD(name) mu_utf8string name
#define JSON_ARRAY_STRING_FIELD(name) std::vector<mu_utf8string> name
#define JSON_OBJECT_FIELD(type, name) type name

struct OIDCMtlsEndpointAliases
{
	OIDC_MTLS_ENDPOINT_ALIASES_FIELDS;

	const mu_boolean Parse(nlohmann::json &document);
};

struct OIDCConfiguration
{
    OIDC_CONFIGURATION_FIELDS;

    const mu_boolean Parse(nlohmann::json &document);
};

struct OIDCTokenEndpointResponse
{
	OIDC_TOKEN_ENDPOINT_RESPONSE_FIELDS;

	const mu_boolean Parse(nlohmann::json &document);
};

#undef JSON_BOOLEAN_FIELD
#undef JSON_INT32_FIELD
#undef JSON_STRING_FIELD
#undef JSON_ARRAY_STRING_FIELD
#undef JSON_OBJECT_FIELD

#define JSON_BOOLEAN_FIELD(name) if (document.contains(#name)) name = document[#name].get<mu_boolean>()
#define JSON_INT32_FIELD(name) if (document.contains(#name)) name = document[#name].get<mu_int32>()
#define JSON_STRING_FIELD(name) if (document.contains(#name)) name = document[#name].get<mu_utf8string>()
#define JSON_ARRAY_STRING_FIELD(name) \
    if (document.contains(#name)) \
    { \
        const auto &jsource = document[#name]; \
        name.reserve(jsource.size()); \
        for (const auto &jvalue : jsource) \
        { \
            name.push_back(jvalue.get<mu_utf8string>()); \
        } \
    }
#define JSON_OBJECT_FIELD(type, name) name.Parse(document[#name])

const mu_boolean OIDCMtlsEndpointAliases::Parse(nlohmann::json &document)
{
	OIDC_MTLS_ENDPOINT_ALIASES_FIELDS;
	return true;
}

const mu_boolean OIDCConfiguration::Parse(nlohmann::json &document)
{
	OIDC_CONFIGURATION_FIELDS;
	return true;
}

const mu_boolean OIDCTokenEndpointResponse::Parse(nlohmann::json &document)
{
	OIDC_TOKEN_ENDPOINT_RESPONSE_FIELDS;
	return true;
}

#undef JSON_BOOLEAN_FIELD
#undef JSON_INT32_FIELD
#undef JSON_STRING_FIELD
#undef JSON_ARRAY_STRING_FIELD
#undef JSON_OBJECT_FIELD

struct OIDCAuthRequestConfig
{
	mu_utf8string CodeChallengeMethod;
	mu_utf8string CodeChallenge;
	mu_utf8string CodeVerifier;
	mu_utf8string Nonce;
    mu_utf8string State;
};

namespace MUSessionManager
{
    mu_utf8string RealmURL;
    std::shared_ptr<WEBFileDownloadRequest> Request;
    Noesis::Ptr<NGPopup> Popup;

    std::unique_ptr<OIDCConfiguration> Configuration;
    std::unique_ptr<OIDCAuthRequestConfig> AuthRequestConfig;
    std::unique_ptr<OIDCTokenEndpointResponse> TokenEndpointResponse;
    mu_utf8string RedirectURI;

    std::mutex AuthMutex;
    mu_utf8string AuthCode;

	const mu_boolean RequestOpenIDConfiguration();
	const mu_boolean ProcessOpenIDConfiguration(const mu_utf8string response);

    class NGOpenIdConfigurationFailed final : public NGPopup
    {
	public:
		explicit NGOpenIdConfigurationFailed(
			EPopupID id,
			EPopupType type,
			EPopupPriority priority,
			mu_utf8string message,
			mu_utf8string accept = "Popup.Buttons.Accept",
			mu_utf8string cancel = "Popup.Buttons.Cancel"
		) : NGPopup(id, type, priority, message, accept, cancel)
        {}

    protected:
        void OnAcceptClicked(Noesis::BaseComponent *param) override
        {
            RequestOpenIDConfiguration();
        }
    };

    const mu_boolean Initialize(mu_utf8string url)
    {
        if (url.ends_with("/") == false) url += '/';
        RealmURL = url;

        if (RequestOpenIDConfiguration() == false)
        {
            return false;
        }

        return true;
	}

	const mu_boolean RequestOpenIDConfiguration()
	{
        if (Request != nullptr)
        {
            return false;
        }

        auto *popupContext = UINoesis::GetContext()->GetPopup();
        auto popup = Noesis::MakePtr<NGPopup>(EPopupID::eRetrieveOpenIdConfiguration, EPopupType::eMessageOnly, EPopupPriority::ePopupHigh, "Popup.Message.RetrieveOpenIdConfiguration");
		popupContext->InsertPopup(Popup);

		auto request = std::make_shared<WEBFileDownloadRequest>(fmt::format("{}.well-known/openid-configuration", RealmURL));
		request->SetRequestResponseCallback(
			[](const WEBFileDownloadRequest *request) {
				UINoesis::GetContext()->GetPopup()->RemovePopup(Popup);
				Popup.Reset();

                if (request->GetResponseCode() != 200 || ProcessOpenIDConfiguration(mu_utf8string(reinterpret_cast<mu_char*>(request->GetBuffer()), request->GetBufferSize())) == false)
				{
					Popup = Noesis::MakePtr<NGOpenIdConfigurationFailed>(EPopupID::eParseOpenIdConfigurationFailed, EPopupType::eAccept, EPopupPriority::ePopupHigh, "Popup.Message.ParseOpenIdConfigurationFailed", "Popup.Buttons.Retry");
					UINoesis::GetContext()->GetPopup()->InsertPopup(Popup);
				}

				Request.reset();
			}
		);
		request->SetRequestFailedCallback(
            [](const WEBFileDownloadRequest *request) {
				UINoesis::GetContext()->GetPopup()->RemovePopup(Popup);
				Popup = Noesis::MakePtr<NGOpenIdConfigurationFailed>(EPopupID::eRetrieveOpenIdConfigurationFailed, EPopupType::eAccept, EPopupPriority::ePopupHigh, "Popup.Message.RetrieveOpenIdConfigurationFailed", "Popup.Buttons.Retry");
				UINoesis::GetContext()->GetPopup()->InsertPopup(Popup);
                Request.reset();
            }
        );
		request->SetRequestAbortedCallback(
			[](const WEBFileDownloadRequest *request) {
				UINoesis::GetContext()->GetPopup()->RemovePopup(Popup);
				Popup.Reset();
				Request.reset();
			}
        );

		auto result = MUWebManager::AddRequest(request);
		if (result != CURLM_OK)
		{
            popupContext->RemovePopup(popup);
            return false;
		}

        Request = std::move(request);
        Popup = std::move(popup);

		return true;
	}

    std::vector<mu_uint8> GenerateRandomBytes(const mu_uint32 bytesCount = 32)
    {
        Poco::Random random;

        std::vector<mu_uint32> bytesAsUint32(bytesCount / 4u + static_cast<mu_uint32>(((bytesCount % 4u) > 0u)));
        for (auto iter = bytesAsUint32.begin(); iter != bytesAsUint32.end(); ++iter)
            *iter = random.next();
        
        return std::vector<mu_uint8>(reinterpret_cast<mu_uint8*>(bytesAsUint32.data()), reinterpret_cast<mu_uint8*>(bytesAsUint32.data()) + bytesCount);
	}

	mu_utf8string Base64UrlEncode(mu_utf8string data)
	{
		std::stringstream ss;
		Poco::Base64Encoder encoder(ss, Poco::BASE64_URL_ENCODING | Poco::BASE64_NO_PADDING);
        encoder << data;
		encoder.close();
		return ss.str();
	}

    mu_utf8string Base64UrlEncode(std::vector<mu_uint8> data)
	{
		std::stringstream ss;
        Poco::Base64Encoder encoder(ss, Poco::BASE64_URL_ENCODING | Poco::BASE64_NO_PADDING);
        for (mu_uint8 &byte : data)
            encoder << byte;
        encoder.close();
        return ss.str();
	}

	std::vector<mu_uint8> Base64UrlDecode(mu_utf8string data)
	{
		std::stringstream is, os;
        is << data;
		Poco::Base64Decoder decoder(is, Poco::BASE64_URL_ENCODING | Poco::BASE64_NO_PADDING);
		copy(std::istreambuf_iterator<char>(decoder),
			std::istreambuf_iterator<char>(),
			std::ostreambuf_iterator<char>(os));
        mu_utf8string output = os.str();
		return std::vector<mu_uint8>(reinterpret_cast<mu_uint8*>(output.data()), reinterpret_cast<mu_uint8*>(output.data()) + output.size());
	}

	mu_utf8string GenerateRandomCodeVerifier()
	{
		return Base64UrlEncode(GenerateRandomBytes());
	}

	mu_utf8string GenerateRandomNonce()
	{
		return Base64UrlEncode(GenerateRandomBytes());
	}

    mu_utf8string CalculatePKCECodeChallenge(mu_utf8string codeVerifier)
    {
        Poco::SHA2Engine256 engine;
        engine.update(codeVerifier.data(), codeVerifier.size());
        auto &result = engine.digest();
        return Base64UrlEncode(result);
    }

    const mu_boolean ProcessOpenIDConfiguration(const mu_utf8string response)
	{
		auto document = nlohmann::json::parse(response.c_str());
		if (document.is_discarded() == true)
		{
			return false;
		}

        auto configuration = std::make_unique<OIDCConfiguration>();
        if (configuration == nullptr)
        {
            return false;
        }

        if (configuration->Parse(document) == false)
        {
            return false;
        }

        Configuration = std::move(configuration);
        UINoesis::GetContext()->GetLogin()->SetShowLogin(true);

        return true;
	}

    const mu_utf8string GenerateAuthCode()
    {
        std::lock_guard lock(AuthMutex);
        AuthCode = GenerateRandomCodeVerifier();
        return AuthCode;
	}

	const mu_boolean CheckAuthCode(const mu_utf8string code)
	{
		std::lock_guard lock(AuthMutex);
        if (AuthCode.empty() == true) return false;
        const mu_boolean isValid = AuthCode.compare(code) == 0;
        AuthCode = "";
		return isValid;
	}
    
    const mu_utf8string GenerateAuthUrl()
	{
		Poco::URI authUri(Configuration->authorization_endpoint);
		authUri.addQueryParameter("client_id", NEXTMU_OPENID_CLIENT_ID);
		authUri.addQueryParameter("redirect_uri", MUWebServerManager::GetBaseURL() + "auth/callback");
		authUri.addQueryParameter("response_type", "code");
		authUri.addQueryParameter("scope", "openid profile email");
		authUri.addQueryParameter("prompt", "login");

		AuthRequestConfig = std::make_unique<OIDCAuthRequestConfig>();

        auto codeChallengeMethod = std::find_if(Configuration->code_challenge_methods_supported.begin(), Configuration->code_challenge_methods_supported.end(), [](mu_utf8string &value) -> mu_boolean { return value.compare("S256") == 0; });
        if (codeChallengeMethod != Configuration->code_challenge_methods_supported.end())
		{
			AuthRequestConfig->CodeChallengeMethod = "S256";
			AuthRequestConfig->CodeVerifier = GenerateRandomCodeVerifier();
			AuthRequestConfig->CodeChallenge = CalculatePKCECodeChallenge(AuthRequestConfig->CodeVerifier);
			authUri.addQueryParameter("code_challenge_method", "S256");
			authUri.addQueryParameter("code_challenge", AuthRequestConfig->CodeChallenge);
        }
        else
		{
			AuthRequestConfig->CodeChallengeMethod = "nonce";
			AuthRequestConfig->Nonce = GenerateRandomNonce();
			authUri.addQueryParameter("nonce", AuthRequestConfig->Nonce);
        }

        return authUri.toString();
    }

    const mu_boolean ValidateAuthResponse(const Poco::URI &requestUri)
    {
        const auto &parameters = requestUri.getQueryParameters();

		const auto issuer = std::find_if(parameters.begin(), parameters.end(), [](const std::pair<std::string, std::string> &value) -> mu_boolean { return value.first.compare("iss") == 0; });
        if (issuer->second.empty() == true && Configuration->authorization_response_iss_parameter_supported == true)
        {
            /*
            * issuer field is missing when shouldn't
            */
            return false;
        }
        
        if (issuer->second.compare(Configuration->issuer) != 0)
        {
            /*
            * issuer field doesn't match
            */
            return false;
        }

		const auto state = std::find_if(parameters.begin(), parameters.end(), [](const std::pair<std::string, std::string> &value) -> mu_boolean { return value.first.compare("state") == 0; });
        if (AuthRequestConfig->State.empty() == true)
        {
            if (state != parameters.end())
            {
                /*
                * state field shouldn't exists
                */
                return false;
            }
        }
        else
        {
            if (state == parameters.end())
            {
                /*
                * state field is missing
                */
                return false;
            }
            
            if (state->second.compare(AuthRequestConfig->State) != 0)
            {
                /*
                * state field doesn't match
                */
                return false;
            }
        }

		const auto error = std::find_if(parameters.begin(), parameters.end(), [](const std::pair<std::string, std::string> &value) -> mu_boolean { return value.first.compare("error") == 0; });
        if (error != parameters.end())
        {
            /*
            * error field received
            */
            return false;
		}

		const auto idToken = std::find_if(parameters.begin(), parameters.end(), [](const std::pair<std::string, std::string> &value) -> mu_boolean { return value.first.compare("id_token") == 0; });
		const auto token = std::find_if(parameters.begin(), parameters.end(), [](const std::pair<std::string, std::string> &value) -> mu_boolean { return value.first.compare("token") == 0; });
        if (idToken != parameters.end() || token != parameters.end())
        {
            /*
            * implicit or hybrid flow found, not supported.
            */
            return false;
        }

        return true;
    }

    struct AuthorizationCodeGrantRequest
    {
		mu_utf8string Url;
		curl_slist *Headers = nullptr;
        mu_utf8string Body;
    };

    AuthorizationCodeGrantRequest GenerateAuthorizationCodeGrant(const mu_utf8string code)
	{
        AuthorizationCodeGrantRequest request;
		request.Url = Configuration->token_endpoint;

		request.Headers = curl_slist_append(request.Headers, "Content-Type: application/x-www-form-urlencoded;charset=UTF-8");
		request.Headers = curl_slist_append(request.Headers, "Accept: application/json");
		//request.Headers = curl_slist_append(request.Headers, fmt::format("Authorization: Basic {}", Base64UrlEncode(NEXTMU_OPENID_CLIENT_ID)).c_str());

        Poco::Net::HTMLForm form;
        form.add("client_id", NEXTMU_OPENID_CLIENT_ID);
		form.add("grant_type", "authorization_code");
		form.add("redirect_uri", RedirectURI);
        if (AuthRequestConfig->CodeChallengeMethod.compare("S256") == 0)
		    form.add("code_verifier", AuthRequestConfig->CodeVerifier);
		form.add("code", code);
		std::stringstream ss;
        form.write(ss);
        request.Body = ss.str();

        return request;
    }

	const mu_boolean ProcessAuthorizationCodeGrantResponse(const mu_utf8string response);
    void ProcessAuthResponse(const Poco::URI &requestUri)
	{
		const auto &parameters = requestUri.getQueryParameters();
        RedirectURI = MUWebServerManager::GetBaseURL() + "auth/callback";

		MUBrowserManager::DestroyBrowser();
		MUWebServerManager::StopListen();

		auto *context = UINoesis::GetContext();
        auto *popupContext = context->GetPopup();

        if (Popup != nullptr)
        {
            popupContext->RemovePopup(Popup);
        }

        auto popup = Noesis::MakePtr<NGPopup>(EPopupID::eProcessingAuthResponse, EPopupType::eMessageOnly, EPopupPriority::ePopupHigh, "Popup.Message.ProcessingAuthResponse");
		popupContext->InsertPopup(popup);

        if (ValidateAuthResponse(requestUri) == false)
		{
			popupContext->RemovePopup(popup);
			popupContext->InsertPopup(Noesis::MakePtr<NGPopup>(EPopupID::eInvalidAuthResponse, EPopupType::eAccept, EPopupPriority::ePopupHigh, "Popup.Message.InvalidAuthResponse", "Popup.Buttons.Ok"));
			context->GetLogin()->SetShowLogin(true);
            return;
        }

		const auto code = std::find_if(parameters.begin(), parameters.end(), [](const std::pair<std::string, std::string> &value) -> mu_boolean { return value.first.compare("code") == 0; });
        if (code == parameters.end())
		{
			popupContext->RemovePopup(popup);
			popupContext->InsertPopup(Noesis::MakePtr<NGPopup>(EPopupID::eInvalidAuthResponse, EPopupType::eAccept, EPopupPriority::ePopupHigh, "Popup.Message.InvalidAuthResponse", "Popup.Buttons.Ok"));
			context->GetLogin()->SetShowLogin(true);
			return;
        }

        const auto requestInfo = GenerateAuthorizationCodeGrant(code->second);
		auto request = std::make_shared<WEBFileDownloadRequest>(requestInfo.Url, nullptr, WEBRequestMethodType::ePOST);
        request->SetHeaders(requestInfo.Headers);
        request->SetBody(requestInfo.Body);
		request->SetRequestResponseCallback(
			[](const WEBFileDownloadRequest *request) {
				UINoesis::GetContext()->GetPopup()->RemovePopup(Popup);
                Popup.Reset();

				if (request->GetResponseCode() != 200 || ProcessAuthorizationCodeGrantResponse(mu_utf8string(reinterpret_cast<mu_char *>(request->GetBuffer()), request->GetBufferSize())) == false)
				{
					UINoesis::GetContext()->GetPopup()->InsertPopup(Noesis::MakePtr<NGPopup>(EPopupID::eParseAuthorizationCodeGrantFailed, EPopupType::eAccept, EPopupPriority::ePopupHigh, "Popup.Message.ParseAuthorizationCodeGrantFailed", "Popup.Buttons.Ok"));
					UINoesis::GetContext()->GetLogin()->SetShowLogin(true);
				}

				Request.reset();
			}
		);
		request->SetRequestFailedCallback(
			[](const WEBFileDownloadRequest *request) {
				UINoesis::GetContext()->GetPopup()->RemovePopup(Popup);
				UINoesis::GetContext()->GetPopup()->InsertPopup(Noesis::MakePtr<NGPopup>(EPopupID::eRetrieveAuthorizationCodeGrantFailed, EPopupType::eAccept, EPopupPriority::ePopupHigh, "Popup.Message.RetrieveAuthorizationCodeGrantFailed", "Popup.Buttons.Ok"));
				UINoesis::GetContext()->GetLogin()->SetShowLogin(true);
				Request.reset();
			}
		);
		request->SetRequestAbortedCallback(
			[](const WEBFileDownloadRequest *request) {
				UINoesis::GetContext()->GetPopup()->RemovePopup(Popup);
				UINoesis::GetContext()->GetLogin()->SetShowLogin(true);
				Popup.Reset();
				Request.reset();
			}
		);

		auto result = MUWebManager::AddRequest(request);
		if (result != CURLM_OK)
		{
			popupContext->RemovePopup(popup);
			popupContext->InsertPopup(Noesis::MakePtr<NGPopup>(EPopupID::eProcessingAuthResponse, EPopupType::eAccept, EPopupPriority::ePopupHigh, "Popup.Message.FailedRetrieveAuthorizationCode"));
			context->GetLogin()->SetShowLogin(true);
			return;
		}

		Request = std::move(request);
		Popup = std::move(popup);
    }

    const mu_boolean ProcessAuthorizationCodeGrantResponse(const mu_utf8string response)
	{
		auto document = nlohmann::json::parse(response.c_str());
		if (document.is_discarded() == true)
		{
			return false;
		}

		auto tokenEndpointResponse = std::make_unique<OIDCTokenEndpointResponse>();
		if (tokenEndpointResponse == nullptr)
		{
			return false;
		}

		if (tokenEndpointResponse->Parse(document) == false)
		{
			return false;
		}

        if (tokenEndpointResponse->token_type.compare("Bearer") != 0)
		{
			return false;
		}

		if (
            tokenEndpointResponse->access_token.empty() ||
            tokenEndpointResponse->id_token.empty() ||
            tokenEndpointResponse->refresh_token.empty()
        )
		{
			return false;
		}

		TokenEndpointResponse = std::move(tokenEndpointResponse);
		UINoesis::GetContext()->GetPopup()->InsertPopup(Noesis::MakePtr<NGPopup>(EPopupID::eRetrieveServersList, EPopupType::eMessageOnly, EPopupPriority::ePopupHigh, "Popup.Message.RetrieveServersList"));
		UINoesis::GetContext()->GetLogin()->SetShowServers(true);

        return true;
    }

    void DestroySession()
    {
        
    }
};