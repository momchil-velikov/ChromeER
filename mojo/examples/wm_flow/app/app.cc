// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/bind.h"
#include "mojo/application/application_runner_chromium.h"
#include "mojo/examples/wm_flow/app/embedder.mojom.h"
#include "mojo/examples/wm_flow/embedded/embeddee.mojom.h"
#include "mojo/public/c/system/main.h"
#include "mojo/public/cpp/application/application_connection.h"
#include "mojo/public/cpp/application/application_delegate.h"
#include "mojo/public/cpp/application/application_impl.h"
#include "mojo/public/cpp/application/connect.h"
#include "mojo/public/cpp/application/interface_factory_impl.h"
#include "mojo/public/cpp/application/service_provider_impl.h"
#include "mojo/public/interfaces/application/service_provider.mojom.h"
#include "mojo/services/public/cpp/view_manager/view.h"
#include "mojo/services/public/cpp/view_manager/view_manager.h"
#include "mojo/services/public/cpp/view_manager/view_manager_client_factory.h"
#include "mojo/services/public/cpp/view_manager/view_manager_context.h"
#include "mojo/services/public/cpp/view_manager/view_manager_delegate.h"
#include "mojo/services/public/cpp/view_manager/view_observer.h"

namespace examples {
namespace {

const SkColor kColors[] = { SK_ColorRED, SK_ColorGREEN, SK_ColorYELLOW };

class EmbedderImpl : public mojo::InterfaceImpl<Embedder> {
 public:
  EmbedderImpl() {}
  virtual ~EmbedderImpl() {}

 private:
  // Overridden from Embedder:
  virtual void HelloWorld(const mojo::Callback<void()>& callback) OVERRIDE {
    callback.Run();
  }

  DISALLOW_COPY_AND_ASSIGN(EmbedderImpl);
};

}  // namespace

// This app starts its life via Connect() rather than by being embed, so it does
// not start with a connection to the ViewManager service. It has to obtain a
// connection by connecting to the ViewManagerInit service and asking to be
// embed without a view context.
class WMFlowApp : public mojo::ApplicationDelegate,
                  public mojo::ViewManagerDelegate,
                  public mojo::ViewObserver {
 public:
  WMFlowApp() : embed_count_(0) {}
  virtual ~WMFlowApp() {}

 private:
  // Overridden from Application:
  virtual void Initialize(mojo::ApplicationImpl* app) override {
    view_manager_client_factory_.reset(
        new mojo::ViewManagerClientFactory(app->shell(), this));
    view_manager_context_.reset(new mojo::ViewManagerContext(app));
    OpenNewWindow();
    OpenNewWindow();
    OpenNewWindow();
  }
  virtual bool ConfigureIncomingConnection(
      mojo::ApplicationConnection* connection) override {
    connection->AddService(view_manager_client_factory_.get());
    return true;
  }

  void OnConnect(bool success) {}

  // Overridden from mojo::ViewManagerDelegate:
  virtual void OnEmbed(
      mojo::ViewManager* view_manager,
      mojo::View* root,
      mojo::ServiceProviderImpl* exported_services,
      scoped_ptr<mojo::ServiceProvider> imported_services) override {
    root->AddObserver(this);
    root->SetColor(kColors[embed_count_++ % arraysize(kColors)]);

    mojo::View* embed = mojo::View::Create(view_manager);
    root->AddChild(embed);
    gfx::Rect bounds = gfx::Rect(root->bounds().size());
    bounds.Inset(25, 25);
    embed->SetBounds(bounds);

    scoped_ptr<mojo::ServiceProviderImpl> registry(
        new mojo::ServiceProviderImpl);
    // Expose some services to the embeddee...
    registry->AddService(&embedder_factory_);
    scoped_ptr<mojo::ServiceProvider> imported =
        embed->Embed("mojo:mojo_wm_flow_embedded", registry.Pass());
    mojo::ConnectToService(imported.get(), &embeddee_);
    embeddee_->HelloBack(base::Bind(&WMFlowApp::HelloBackAck,
                                    base::Unretained(this)));
  }
  virtual void OnViewManagerDisconnected(
      mojo::ViewManager* view_manager) override {}

  // Overridden from mojo::ViewObserver:
  virtual void OnViewInputEvent(mojo::View* view,
                                const mojo::EventPtr& event) override {
    if (event->action == mojo::EVENT_TYPE_MOUSE_RELEASED &&
        event->flags & mojo::EVENT_FLAGS_LEFT_MOUSE_BUTTON) {
      OpenNewWindow();
    }
  }
  virtual void OnViewDestroyed(mojo::View* view) override {
    --embed_count_;
    view->RemoveObserver(this);
  }

  void HelloBackAck() {
    printf("HelloBack() ack'ed\n");
  }

  void OpenNewWindow() {
    view_manager_context_->Embed("mojo:mojo_wm_flow_app");
  }

  int embed_count_;
  scoped_ptr<mojo::ViewManagerClientFactory> view_manager_client_factory_;
  mojo::InterfaceFactoryImpl<EmbedderImpl> embedder_factory_;
  scoped_ptr<mojo::ViewManagerContext> view_manager_context_;
  EmbeddeePtr embeddee_;

  DISALLOW_COPY_AND_ASSIGN(WMFlowApp);
};

}  // namespace examples

MojoResult MojoMain(MojoHandle shell_handle) {
  mojo::ApplicationRunnerChromium runner(new examples::WMFlowApp);
  return runner.Run(shell_handle);
}
