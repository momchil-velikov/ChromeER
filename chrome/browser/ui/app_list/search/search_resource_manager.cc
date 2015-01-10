// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/app_list/search/search_resource_manager.h"

#include "chrome/browser/ui/app_list/start_page_service.h"
#include "grit/components_scaled_resources.h"
#include "grit/generated_resources.h"
#include "grit/theme_resources.h"
#include "ui/app_list/search_box_model.h"
#include "ui/app_list/speech_ui_model.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/resource/resource_bundle.h"

namespace app_list {

SearchResourceManager::SearchResourceManager(Profile* profile,
                                             SearchBoxModel* search_box,
                                             SpeechUIModel* speech_ui)
    : search_box_(search_box),
      speech_ui_(speech_ui) {
  speech_ui_->AddObserver(this);

  ui::ResourceBundle& bundle = ui::ResourceBundle::GetSharedInstance();
  search_box_->SetIcon(*bundle.GetImageSkiaNamed(IDR_OMNIBOX_SEARCH));
  StartPageService* service = StartPageService::Get(profile);
  if (service && service->GetSpeechRecognitionContents()) {
    search_box_->SetSpeechRecognitionButton(
        scoped_ptr<SearchBoxModel::SpeechButtonProperty>(
            new SearchBoxModel::SpeechButtonProperty(
                *bundle.GetImageSkiaNamed(IDR_OMNIBOX_MIC_SEARCH),
                l10n_util::GetStringUTF16(
                    IDS_APP_LIST_HOTWORD_LISTENING),
                *bundle.GetImageSkiaNamed(IDR_APP_LIST_MIC_HOTWORD_OFF),
                l10n_util::GetStringUTF16(
                    IDS_APP_LIST_START_SPEECH_RECOGNITION))));
  }
  OnSpeechRecognitionStateChanged(speech_ui_->state());
}

SearchResourceManager::~SearchResourceManager() {
  speech_ui_->RemoveObserver(this);
}

void SearchResourceManager::OnSpeechRecognitionStateChanged(
    SpeechRecognitionState new_state) {
  search_box_->SetHintText(l10n_util::GetStringUTF16(
      (new_state == SPEECH_RECOGNITION_HOTWORD_LISTENING) ?
      IDS_SEARCH_BOX_HOTWORD_HINT : IDS_SEARCH_BOX_HINT));
}

}  // namespace app_list
