/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2020 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "DIALServer.h"

#include "interfaces/IBrowser.h"
#include "interfaces/ISwitchBoard.h"

namespace WPEFramework {
namespace DIALHandlers {

    class YouTube : public Plugin::DIALServer::Default {
    private:
        YouTube() = delete;
        YouTube(const YouTube&) = delete;
        YouTube& operator=(const YouTube&) = delete;

    public:
        YouTube(PluginHost::IShell* service, const Plugin::DIALServer::Config::App& config, Plugin::DIALServer* parent)
            : Default(service, config, parent)
            , _browser(nullptr)
            , _hidden(false)
            , _notification(this)
        {
        }
        virtual ~YouTube()
        {
            Stopped({});
        }

    public:
        uint32_t Start(const string& params) override {
            Core::SystemInfo::SetEnvironment(_T("DIAL_START_URL"), params.c_str());
            return Default::Start(params);
        }

        virtual void Started(const string& data)
        {
            _browser = Plugin::DIALServer::Default::QueryInterface<Exchange::IBrowser>();
            if (_browser != nullptr) {
                _browser->Register(&_notification);
                _browser->SetURL(data);
            }
        }
        virtual void Stopped(const string& data)
        {
            if (_browser != nullptr) {
                _browser->Unregister(&_notification);
                _browser->Release();
                _browser = nullptr;
            }
        }

        bool HasHideAndShow() const override {
            return _browser != nullptr;
        }

        uint32_t Show(const string& params) override {
            _browser->Hide(false);
            return Core::ERROR_NONE;
        }

        void Hide() override {
            _browser->Hide(true);
        }

        bool IsHidden() const override {
            return _hidden;
        }

    private:
        struct Notification : public Exchange::IBrowser::INotification {
            explicit Notification(YouTube* parent) : _parent(parent) {}
            void LoadFinished(const string& URL) override {}
            void URLChanged(const string& URL) override {}
            void Hidden(const bool hidden) override { _parent->_hidden = hidden; }
            void Closure() override {}

            BEGIN_INTERFACE_MAP(YouTube)
                INTERFACE_ENTRY(Exchange::IBrowser::INotification)
            END_INTERFACE_MAP

            YouTube* _parent;
        };
        Exchange::IBrowser* _browser;
        bool _hidden;
        Core::Sink<Notification> _notification;
    };

    static Plugin::DIALServer::ApplicationRegistrationType<YouTube> _youTubeHandler;
}
}
