/*****************************************************************************
 *
 *  PROJECT:     Multi Theft Auto
 *  LICENSE:     See LICENSE in the top level directory
 *  PURPOSE:     Router to resources for HTTP requests
 *
 *  Multi Theft Auto is available from https://multitheftauto.com/
 *
 *****************************************************************************/

#pragma once

#include "HTTPServer.h"
#include <string>

namespace mtasa
{
    class HTTPResourceRouter : public HTTPMiddleware
    {
    public:
        bool PreProcessRequest(HTTPRequest& request, HTTPResponse& response) override;

    public:
        void SetDefaultResource(const char* resourceName) { m_defaultResourceName = std::string(resourceName); }

    private:
        std::string m_defaultResourceName;
    };
}
