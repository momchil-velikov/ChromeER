// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/bind.h"
#include "base/command_line.h"
#include "base/prefs/pref_service.h"
#include "base/strings/utf_string_conversions.h"
#include "chrome/browser/password_manager/password_store_factory.h"
#include "chrome/browser/profiles/profile_info_cache.h"
#include "chrome/browser/profiles/profile_info_cache_observer.h"
#include "chrome/browser/profiles/profile_manager.h"
#include "chrome/browser/profiles/profile_window.h"
#include "chrome/browser/profiles/profiles_state.h"
#include "chrome/browser/ui/browser_finder.h"
#include "chrome/browser/ui/browser_list.h"
#include "chrome/browser/ui/browser_window.h"
#include "chrome/browser/ui/host_desktop.h"
#include "chrome/common/pref_names.h"
#include "chrome/test/base/in_process_browser_test.h"
#include "chrome/test/base/test_switches.h"
#include "chrome/test/base/testing_browser_process.h"
#include "chrome/test/base/ui_test_utils.h"
#include "components/autofill/core/common/password_form.h"
#include "components/password_manager/core/browser/password_store.h"
#include "components/password_manager/core/browser/password_store_consumer.h"

#if defined(OS_CHROMEOS)
#include "base/path_service.h"
#include "chrome/common/chrome_constants.h"
#include "chrome/common/chrome_paths.h"
#include "chromeos/chromeos_switches.h"
#include "testing/gtest/include/gtest/gtest.h"
#endif

namespace {

const ProfileManager::CreateCallback kOnProfileSwitchDoNothing;

// An observer that returns back to test code after a new profile is
// initialized.
void OnUnblockOnProfileCreation(Profile* profile,
                                Profile::CreateStatus status) {
  if (status == Profile::CREATE_STATUS_INITIALIZED)
    base::MessageLoop::current()->Quit();
}

void ProfileCreationComplete(Profile* profile, Profile::CreateStatus status) {
  ASSERT_NE(status, Profile::CREATE_STATUS_LOCAL_FAIL);
  ASSERT_NE(status, Profile::CREATE_STATUS_REMOTE_FAIL);
  // No browser should have been created for this profile yet.
  EXPECT_EQ(chrome::GetTotalBrowserCountForProfile(profile), 0U);
  EXPECT_EQ(chrome::GetTotalBrowserCount(), 1U);
  if (status == Profile::CREATE_STATUS_INITIALIZED)
    base::MessageLoop::current()->Quit();
}

void EphemeralProfileCreationComplete(Profile* profile,
                                      Profile::CreateStatus status) {
  if (status == Profile::CREATE_STATUS_INITIALIZED)
    profile->GetPrefs()->SetBoolean(prefs::kForceEphemeralProfiles, true);
  ProfileCreationComplete(profile, status);
}

class ProfileRemovalObserver : public ProfileInfoCacheObserver {
 public:
  ProfileRemovalObserver() {
    g_browser_process->profile_manager()->GetProfileInfoCache().AddObserver(
        this);
  }

  ~ProfileRemovalObserver() override {
    g_browser_process->profile_manager()->GetProfileInfoCache().RemoveObserver(
        this);
  }

  std::string last_used_profile_name() { return last_used_profile_name_; }

  // ProfileInfoCacheObserver overrides:
  void OnProfileWillBeRemoved(const base::FilePath& profile_path) override {
    last_used_profile_name_ = g_browser_process->local_state()->GetString(
        prefs::kProfileLastUsed);
  }

 private:
  std::string last_used_profile_name_;

  DISALLOW_COPY_AND_ASSIGN(ProfileRemovalObserver);
};

// The class serves to retrieve passwords from PasswordStore asynchronously. It
// used by ProfileManagerBrowserTest.DeletePasswords on some platforms.
class PasswordStoreConsumerVerifier :
    public password_manager::PasswordStoreConsumer {
 public:
  PasswordStoreConsumerVerifier() : called_(false) {}

  void OnGetPasswordStoreResults(
      const std::vector<autofill::PasswordForm*>& results) override {
    EXPECT_FALSE(called_);
    called_ = true;
    password_entries_.clear();
    password_entries_.assign(results.begin(), results.end());
  }

  bool IsCalled() const { return called_; }

  const std::vector<autofill::PasswordForm*>& GetPasswords() const {
    return password_entries_.get();
  }
 private:
  ScopedVector<autofill::PasswordForm> password_entries_;
  bool called_;
};

} // namespace

// This file contains tests for the ProfileManager that require a heavyweight
// InProcessBrowserTest.  These include tests involving profile deletion.

// TODO(jeremy): crbug.com/103355 - These tests should be enabled on all
// platforms.
class ProfileManagerBrowserTest : public InProcessBrowserTest {
 protected:
  void SetUpCommandLine(CommandLine* command_line) override {
#if defined(OS_CHROMEOS)
    command_line->AppendSwitch(
        chromeos::switches::kIgnoreUserProfileMappingForTests);
#endif
  }
};

#if defined(OS_MACOSX)

// Delete single profile and make sure a new one is created.
IN_PROC_BROWSER_TEST_F(ProfileManagerBrowserTest, DeleteSingletonProfile) {
  ProfileManager* profile_manager = g_browser_process->profile_manager();
  ProfileInfoCache& cache = profile_manager->GetProfileInfoCache();
  ProfileRemovalObserver observer;

  // We should start out with 1 profile.
  ASSERT_EQ(cache.GetNumberOfProfiles(), 1U);

  // Delete singleton profile.
  base::FilePath singleton_profile_path = cache.GetPathOfProfileAtIndex(0);
  EXPECT_FALSE(singleton_profile_path.empty());
  profile_manager->ScheduleProfileForDeletion(singleton_profile_path,
                                              ProfileManager::CreateCallback());

  // Spin things till profile is actually deleted.
  content::RunAllPendingInMessageLoop();

  // Make sure a new profile was created automatically.
  EXPECT_EQ(cache.GetNumberOfProfiles(), 1U);
  base::FilePath new_profile_path = cache.GetPathOfProfileAtIndex(0);
  EXPECT_NE(new_profile_path, singleton_profile_path);

  // Make sure that last used profile preference is set correctly.
  Profile* last_used = ProfileManager::GetLastUsedProfile();
  EXPECT_EQ(new_profile_path, last_used->GetPath());

  // Make sure the last used profile was set correctly before the notification
  // was sent.
  std::string last_used_profile_name =
      last_used->GetPath().BaseName().MaybeAsASCII();
  EXPECT_EQ(last_used_profile_name, observer.last_used_profile_name());
}

// Delete all profiles in a multi profile setup and make sure a new one is
// created.
// Crashes/CHECKs. See crbug.com/104851
IN_PROC_BROWSER_TEST_F(ProfileManagerBrowserTest, DISABLED_DeleteAllProfiles) {
  ProfileManager* profile_manager = g_browser_process->profile_manager();
  ProfileInfoCache& cache = profile_manager->GetProfileInfoCache();

  // Create an additional profile.
  base::FilePath new_path = profile_manager->GenerateNextProfileDirectoryPath();
  profile_manager->CreateProfileAsync(new_path,
                                      base::Bind(&OnUnblockOnProfileCreation),
                                      base::string16(), base::string16(),
                                      std::string());

  // Spin to allow profile creation to take place, loop is terminated
  // by OnUnblockOnProfileCreation when the profile is created.
  content::RunMessageLoop();

  ASSERT_EQ(cache.GetNumberOfProfiles(), 2U);

  // Delete all profiles.
  base::FilePath profile_path1 = cache.GetPathOfProfileAtIndex(0);
  base::FilePath profile_path2 = cache.GetPathOfProfileAtIndex(1);
  EXPECT_FALSE(profile_path1.empty());
  EXPECT_FALSE(profile_path2.empty());
  profile_manager->ScheduleProfileForDeletion(profile_path1,
                                              ProfileManager::CreateCallback());
  profile_manager->ScheduleProfileForDeletion(profile_path2,
                                              ProfileManager::CreateCallback());

  // Spin things so deletion can take place.
  content::RunAllPendingInMessageLoop();

  // Make sure a new profile was created automatically.
  EXPECT_EQ(cache.GetNumberOfProfiles(), 1U);
  base::FilePath new_profile_path = cache.GetPathOfProfileAtIndex(0);
  EXPECT_NE(new_profile_path, profile_path1);
  EXPECT_NE(new_profile_path, profile_path2);

  // Make sure that last used profile preference is set correctly.
  Profile* last_used = ProfileManager::GetLastUsedProfile();
  EXPECT_EQ(new_profile_path, last_used->GetPath());
}
#endif  // OS_MACOSX

#if defined(OS_CHROMEOS)

class ProfileManagerCrOSBrowserTest : public ProfileManagerBrowserTest {
 protected:
  virtual void SetUpCommandLine(CommandLine* command_line) override {
    // Use a user hash other than the default chrome::kTestUserProfileDir
    // so that the prefix case is tested.
    command_line->AppendSwitchASCII(chromeos::switches::kLoginProfile,
                                    "test-user-hash");
  }
};

IN_PROC_BROWSER_TEST_F(ProfileManagerCrOSBrowserTest, GetLastUsedProfile) {
  // Make sure that last used profile is correct.
  Profile* last_used_profile = ProfileManager::GetLastUsedProfile();
  EXPECT_TRUE(last_used_profile != NULL);

  base::FilePath profile_path;
  PathService::Get(chrome::DIR_USER_DATA, &profile_path);

  profile_path = profile_path.AppendASCII(
      std::string(chrome::kProfileDirPrefix) + "test-user-hash");
  EXPECT_EQ(profile_path.value(), last_used_profile->GetPath().value());
}

#endif  // OS_CHROMEOS

// Times out (http://crbug.com/159002)
IN_PROC_BROWSER_TEST_F(ProfileManagerBrowserTest,
                       DISABLED_CreateProfileWithCallback) {
  ProfileManager* profile_manager = g_browser_process->profile_manager();

  ASSERT_EQ(profile_manager->GetNumberOfProfiles(), 1U);
  EXPECT_EQ(chrome::GetTotalBrowserCount(), 1U);

  // Create a profile, make sure callback is invoked before any callbacks are
  // invoked (so they can do things like sign in the profile, etc).
  ProfileManager::CreateMultiProfileAsync(
      base::string16(), // name
      base::string16(), // icon url
      base::Bind(ProfileCreationComplete),
      std::string());
  // Wait for profile to finish loading.
  content::RunMessageLoop();
  EXPECT_EQ(profile_manager->GetNumberOfProfiles(), 2U);
  EXPECT_EQ(chrome::GetTotalBrowserCount(), 2U);

  // Now close all browser windows.
  std::vector<Profile*> profiles = profile_manager->GetLoadedProfiles();
  for (std::vector<Profile*>::const_iterator it = profiles.begin();
       it != profiles.end(); ++it) {
    BrowserList::CloseAllBrowsersWithProfile(*it);
  }
}

IN_PROC_BROWSER_TEST_F(ProfileManagerBrowserTest,
                       SwitchToProfile) {
#if defined(OS_WIN) && defined(USE_ASH)
  // Disable this test in Metro+Ash for now (http://crbug.com/262796).
  if (CommandLine::ForCurrentProcess()->HasSwitch(switches::kAshBrowserTests))
    return;
#endif

  // If multiprofile mode is not enabled, you can't switch between profiles.
  if (!profiles::IsMultipleProfilesEnabled())
    return;

  ProfileManager* profile_manager = g_browser_process->profile_manager();
  ProfileInfoCache& cache = profile_manager->GetProfileInfoCache();
  base::FilePath path_profile1 = cache.GetPathOfProfileAtIndex(0);

  ASSERT_EQ(profile_manager->GetNumberOfProfiles(), 1U);
  EXPECT_EQ(chrome::GetTotalBrowserCount(), 1U);

  // Create an additional profile.
  base::FilePath path_profile2 =
      profile_manager->GenerateNextProfileDirectoryPath();
  profile_manager->CreateProfileAsync(path_profile2,
                                      base::Bind(&OnUnblockOnProfileCreation),
                                      base::string16(), base::string16(),
                                      std::string());

  // Spin to allow profile creation to take place, loop is terminated
  // by OnUnblockOnProfileCreation when the profile is created.
  content::RunMessageLoop();

  chrome::HostDesktopType desktop_type = chrome::GetActiveDesktop();
  BrowserList* browser_list = BrowserList::GetInstance(desktop_type);
  ASSERT_EQ(cache.GetNumberOfProfiles(), 2U);
  EXPECT_EQ(1U, browser_list->size());

  // Open a browser window for the first profile.
  profiles::SwitchToProfile(path_profile1, desktop_type, false,
                            kOnProfileSwitchDoNothing,
                            ProfileMetrics::SWITCH_PROFILE_ICON);
  EXPECT_EQ(chrome::GetTotalBrowserCount(), 1U);
  EXPECT_EQ(1U, browser_list->size());
  EXPECT_EQ(path_profile1, browser_list->get(0)->profile()->GetPath());

  // Open a browser window for the second profile.
  profiles::SwitchToProfile(path_profile2, desktop_type, false,
                            kOnProfileSwitchDoNothing,
                            ProfileMetrics::SWITCH_PROFILE_ICON);
  EXPECT_EQ(chrome::GetTotalBrowserCount(), 2U);
  EXPECT_EQ(2U, browser_list->size());
  EXPECT_EQ(path_profile2, browser_list->get(1)->profile()->GetPath());

  // Switch to the first profile without opening a new window.
  profiles::SwitchToProfile(path_profile1, desktop_type, false,
                            kOnProfileSwitchDoNothing,
                            ProfileMetrics::SWITCH_PROFILE_ICON);
  EXPECT_EQ(chrome::GetTotalBrowserCount(), 2U);
  EXPECT_EQ(2U, browser_list->size());

  EXPECT_EQ(path_profile1, browser_list->get(0)->profile()->GetPath());
  EXPECT_EQ(path_profile2, browser_list->get(1)->profile()->GetPath());
}

// Flakes on Windows: http://crbug.com/314905
#if defined(OS_WIN)
#define MAYBE_EphemeralProfile DISABLED_EphemeralProfile
#else
#define MAYBE_EphemeralProfile EphemeralProfile
#endif
IN_PROC_BROWSER_TEST_F(ProfileManagerBrowserTest, MAYBE_EphemeralProfile) {
#if defined(OS_WIN) && defined(USE_ASH)
  // Disable this test in Metro+Ash for now (http://crbug.com/262796).
  if (CommandLine::ForCurrentProcess()->HasSwitch(switches::kAshBrowserTests))
    return;
#endif

  // If multiprofile mode is not enabled, you can't switch between profiles.
  if (!profiles::IsMultipleProfilesEnabled())
    return;

  ProfileManager* profile_manager = g_browser_process->profile_manager();
  ProfileInfoCache& cache = profile_manager->GetProfileInfoCache();
  base::FilePath path_profile1 = cache.GetPathOfProfileAtIndex(0);

  ASSERT_EQ(1U, profile_manager->GetNumberOfProfiles());
  EXPECT_EQ(1U, chrome::GetTotalBrowserCount());

  // Create an ephemeral profile.
  base::FilePath path_profile2 =
      profile_manager->GenerateNextProfileDirectoryPath();
  profile_manager->CreateProfileAsync(
      path_profile2,
      base::Bind(&EphemeralProfileCreationComplete),
      base::string16(), base::string16(), std::string());

  // Spin to allow profile creation to take place.
  content::RunMessageLoop();

  chrome::HostDesktopType desktop_type = chrome::GetActiveDesktop();
  BrowserList* browser_list = BrowserList::GetInstance(desktop_type);
  ASSERT_EQ(2U, cache.GetNumberOfProfiles());
  EXPECT_EQ(1U, browser_list->size());

  // Open a browser window for the second profile.
  profiles::SwitchToProfile(path_profile2, desktop_type, false,
                            kOnProfileSwitchDoNothing,
                            ProfileMetrics::SWITCH_PROFILE_ICON);
  EXPECT_EQ(2U, chrome::GetTotalBrowserCount());
  EXPECT_EQ(2U, browser_list->size());
  EXPECT_EQ(path_profile2, browser_list->get(1)->profile()->GetPath());

  // Create a second window for the ephemeral profile.
  profiles::SwitchToProfile(path_profile2, desktop_type, true,
                            kOnProfileSwitchDoNothing,
                            ProfileMetrics::SWITCH_PROFILE_ICON);
  EXPECT_EQ(3U, chrome::GetTotalBrowserCount());
  EXPECT_EQ(3U, browser_list->size());

  EXPECT_EQ(path_profile1, browser_list->get(0)->profile()->GetPath());
  EXPECT_EQ(path_profile2, browser_list->get(1)->profile()->GetPath());
  EXPECT_EQ(path_profile2, browser_list->get(2)->profile()->GetPath());

  // Closing the first window of the ephemeral profile should not delete it.
  browser_list->get(2)->window()->Close();
  content::RunAllPendingInMessageLoop();
  EXPECT_EQ(2U, browser_list->size());
  ASSERT_EQ(2U, cache.GetNumberOfProfiles());

  // The second should though.
  browser_list->get(1)->window()->Close();
  content::RunAllPendingInMessageLoop();
  EXPECT_EQ(1U, browser_list->size());
  ASSERT_EQ(1U, cache.GetNumberOfProfiles());
}

// The test makes sense on those platforms where the keychain exists.
#if !defined(OS_WIN) && !defined(OS_ANDROID) && !defined(OS_CHROMEOS)
IN_PROC_BROWSER_TEST_F(ProfileManagerBrowserTest, DeletePasswords) {
  Profile* profile = ProfileManager::GetActiveUserProfile();
  ASSERT_TRUE(profile);

  autofill::PasswordForm form;
  form.scheme = autofill::PasswordForm::SCHEME_HTML;
  form.origin = GURL("http://accounts.google.com/LoginAuth");
  form.signon_realm = "http://accounts.google.com/";
  form.username_value = base::ASCIIToUTF16("my_username");
  form.password_value = base::ASCIIToUTF16("my_password");
  form.ssl_valid = false;
  form.preferred = true;
  form.blacklisted_by_user = false;

  scoped_refptr<password_manager::PasswordStore> password_store =
      PasswordStoreFactory::GetForProfile(profile, Profile::EXPLICIT_ACCESS)
          .get();
  ASSERT_TRUE(password_store.get());

  password_store->AddLogin(form);
  PasswordStoreConsumerVerifier verify_add;
  password_store->GetAutofillableLogins(&verify_add);

  ProfileManager* profile_manager = g_browser_process->profile_manager();
  profile_manager->ScheduleProfileForDeletion(profile->GetPath(),
                                              ProfileManager::CreateCallback());
  content::RunAllPendingInMessageLoop();
  PasswordStoreConsumerVerifier verify_delete;
  password_store->GetAutofillableLogins(&verify_delete);

  // Run the password background thread.
  base::RunLoop run_loop;
  base::Closure task = base::Bind(
      base::IgnoreResult(&content::BrowserThread::PostTask),
      content::BrowserThread::UI,
      FROM_HERE,
      run_loop.QuitClosure());
  EXPECT_TRUE(password_store->ScheduleTask(task));
  run_loop.Run();

  EXPECT_TRUE(verify_add.IsCalled());
  EXPECT_EQ(1u, verify_add.GetPasswords().size());
  EXPECT_TRUE(verify_delete.IsCalled());
  EXPECT_EQ(0u, verify_delete.GetPasswords().size());
}
#endif  // !defined(OS_WIN) && !defined(OS_ANDROID) && !defined(OS_CHROMEOS)
