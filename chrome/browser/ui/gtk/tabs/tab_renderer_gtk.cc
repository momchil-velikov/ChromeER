 // Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/gtk/tabs/tab_renderer_gtk.h"

#include <algorithm>
#include <utility>

#include "base/utf_string_conversions.h"
#include "chrome/browser/defaults.h"
#include "chrome/browser/extensions/extension_tab_helper.h"
#include "chrome/browser/favicon/favicon_tab_helper.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/gtk/bookmarks/bookmark_utils_gtk.h"
#include "chrome/browser/ui/gtk/cairo_cached_surface.h"
#include "chrome/browser/ui/gtk/custom_button.h"
#include "chrome/browser/ui/gtk/gtk_theme_service.h"
#include "chrome/browser/ui/gtk/gtk_util.h"
#include "chrome/browser/ui/tab_contents/tab_contents_wrapper.h"
#include "chrome/common/chrome_notification_types.h"
#include "content/browser/tab_contents/tab_contents.h"
#include "content/public/browser/notification_source.h"
#include "grit/generated_resources.h"
#include "grit/theme_resources.h"
#include "grit/theme_resources_standard.h"
#include "grit/ui_resources.h"
#include "ui/base/animation/slide_animation.h"
#include "ui/base/animation/throb_animation.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/gfx/canvas_skia_paint.h"
#include "ui/gfx/favicon_size.h"
#include "ui/gfx/gtk_util.h"
#include "ui/gfx/pango_util.h"
#include "ui/gfx/platform_font_pango.h"
#include "ui/gfx/skbitmap_operations.h"
#include "skia/ext/image_operations.h"

#if !GTK_CHECK_VERSION(2, 22, 0)
#define gtk_button_get_event_window(button) button->event_window
#endif  // Gtk+ >= 2.22

namespace {

const int kFontPixelSize = 12;
const int kLeftPadding = 16;
const int kTopPadding = 6;
const int kRightPadding = 15;
const int kBottomPadding = 5;
const int kDropShadowHeight = 2;
const int kFaviconTitleSpacing = 4;
const int kTitleCloseButtonSpacing = 5;
const int kStandardTitleWidth = 175;
const int kDropShadowOffset = 2;
const int kInactiveTabBackgroundOffsetY = 15;

// When a non-mini-tab becomes a mini-tab the width of the tab animates. If
// the width of a mini-tab is >= kMiniTabRendererAsNormalTabWidth then the tab
// is rendered as a normal tab. This is done to avoid having the title
// immediately disappear when transitioning a tab from normal to mini-tab.
const int kMiniTabRendererAsNormalTabWidth =
    browser_defaults::kMiniTabWidth + 30;

// The tab images are designed to overlap the toolbar by 1 pixel. For now we
// don't actually overlap the toolbar, so this is used to know how many pixels
// at the bottom of the tab images are to be ignored.
const int kToolbarOverlap = 1;

// How long the hover state takes.
const int kHoverDurationMs = 90;

// How opaque to make the hover state (out of 1).
const double kHoverOpacity = 0.33;

// Max opacity for the mini-tab title change animation.
const double kMiniTitleChangeThrobOpacity = 0.75;

// Duration for when the title of an inactive mini-tab changes.
const int kMiniTitleChangeThrobDuration = 1000;

// The vertical and horizontal offset used to position the close button
// in the tab. TODO(jhawkins): Ask pkasting what the Fuzz is about.
const int kCloseButtonVertFuzz = 0;
const int kCloseButtonHorzFuzz = 5;

// Gets the bounds of |widget| relative to |parent|.
gfx::Rect GetWidgetBoundsRelativeToParent(GtkWidget* parent,
                                          GtkWidget* widget) {
  gfx::Point parent_pos = gtk_util::GetWidgetScreenPosition(parent);
  gfx::Point widget_pos = gtk_util::GetWidgetScreenPosition(widget);
  return gfx::Rect(widget_pos.x() - parent_pos.x(),
                   widget_pos.y() - parent_pos.y(),
                   widget->allocation.width, widget->allocation.height);
}

}  // namespace

TabRendererGtk::LoadingAnimation::Data::Data(
    GtkThemeService* theme_service) {
  // The loading animation image is a strip of states. Each state must be
  // square, so the height must divide the width evenly.
  SkBitmap* loading_animation_frames =
      theme_service->GetBitmapNamed(IDR_THROBBER);
  DCHECK(loading_animation_frames);
  DCHECK_EQ(loading_animation_frames->width() %
            loading_animation_frames->height(), 0);
  loading_animation_frame_count =
      loading_animation_frames->width() /
      loading_animation_frames->height();

  SkBitmap* waiting_animation_frames =
      theme_service->GetBitmapNamed(IDR_THROBBER_WAITING);
  DCHECK(waiting_animation_frames);
  DCHECK_EQ(waiting_animation_frames->width() %
            waiting_animation_frames->height(), 0);
  waiting_animation_frame_count =
      waiting_animation_frames->width() /
      waiting_animation_frames->height();

  waiting_to_loading_frame_count_ratio =
      waiting_animation_frame_count /
      loading_animation_frame_count;
  // TODO(beng): eventually remove this when we have a proper themeing system.
  //             themes not supporting IDR_THROBBER_WAITING are causing this
  //             value to be 0 which causes DIV0 crashes. The value of 5
  //             matches the current bitmaps in our source.
  if (waiting_to_loading_frame_count_ratio == 0)
    waiting_to_loading_frame_count_ratio = 5;
}

TabRendererGtk::LoadingAnimation::Data::Data(
    int loading, int waiting, int waiting_to_loading)
    : loading_animation_frame_count(loading),
      waiting_animation_frame_count(waiting),
      waiting_to_loading_frame_count_ratio(waiting_to_loading) {
}

bool TabRendererGtk::initialized_ = false;
int TabRendererGtk::tab_active_l_width_ = 0;
int TabRendererGtk::tab_active_l_height_ = 0;
int TabRendererGtk::tab_inactive_l_height_ = 0;
gfx::Font* TabRendererGtk::title_font_ = NULL;
int TabRendererGtk::title_font_height_ = 0;
int TabRendererGtk::close_button_width_ = 0;
int TabRendererGtk::close_button_height_ = 0;
SkColor TabRendererGtk::selected_title_color_ = SK_ColorBLACK;
SkColor TabRendererGtk::unselected_title_color_ = SkColorSetRGB(64, 64, 64);

////////////////////////////////////////////////////////////////////////////////
// TabRendererGtk::LoadingAnimation, public:
//
TabRendererGtk::LoadingAnimation::LoadingAnimation(
    GtkThemeService* theme_service)
    : data_(new Data(theme_service)),
      theme_service_(theme_service),
      animation_state_(ANIMATION_NONE),
      animation_frame_(0) {
  registrar_.Add(this,
                 chrome::NOTIFICATION_BROWSER_THEME_CHANGED,
                 content::Source<ThemeService>(theme_service_));
}

TabRendererGtk::LoadingAnimation::LoadingAnimation(
    const LoadingAnimation::Data& data)
    : data_(new Data(data)),
      theme_service_(NULL),
      animation_state_(ANIMATION_NONE),
      animation_frame_(0) {
}

TabRendererGtk::LoadingAnimation::~LoadingAnimation() {}

bool TabRendererGtk::LoadingAnimation::ValidateLoadingAnimation(
    AnimationState animation_state) {
  bool has_changed = false;
  if (animation_state_ != animation_state) {
    // The waiting animation is the reverse of the loading animation, but at a
    // different rate - the following reverses and scales the animation_frame_
    // so that the frame is at an equivalent position when going from one
    // animation to the other.
    if (animation_state_ == ANIMATION_WAITING &&
        animation_state == ANIMATION_LOADING) {
      animation_frame_ = data_->loading_animation_frame_count -
          (animation_frame_ / data_->waiting_to_loading_frame_count_ratio);
    }
    animation_state_ = animation_state;
    has_changed = true;
  }

  if (animation_state_ != ANIMATION_NONE) {
    animation_frame_ = (animation_frame_ + 1) %
                       ((animation_state_ == ANIMATION_WAITING) ?
                         data_->waiting_animation_frame_count :
                         data_->loading_animation_frame_count);
    has_changed = true;
  } else {
    animation_frame_ = 0;
  }
  return has_changed;
}

void TabRendererGtk::LoadingAnimation::Observe(
    int type,
    const content::NotificationSource& source,
    const content::NotificationDetails& details) {
  DCHECK(type == chrome::NOTIFICATION_BROWSER_THEME_CHANGED);
  data_.reset(new Data(theme_service_));
}

TabRendererGtk::TabData::TabData()
    : is_default_favicon(false),
      loading(false),
      crashed(false),
      incognito(false),
      show_icon(true),
      mini(false),
      blocked(false),
      animating_mini_change(false),
      app(false) {
}

TabRendererGtk::TabData::~TabData() {}

////////////////////////////////////////////////////////////////////////////////
// FaviconCrashAnimation
//
//  A custom animation subclass to manage the favicon crash animation.
class TabRendererGtk::FaviconCrashAnimation : public ui::LinearAnimation,
                                              public ui::AnimationDelegate {
 public:
  explicit FaviconCrashAnimation(TabRendererGtk* target)
      : ALLOW_THIS_IN_INITIALIZER_LIST(ui::LinearAnimation(1000, 25, this)),
        target_(target) {
  }
  virtual ~FaviconCrashAnimation() {}

  // ui::Animation overrides:
  virtual void AnimateToState(double state) {
    const double kHidingOffset = 27;

    if (state < .5) {
      target_->SetFaviconHidingOffset(
          static_cast<int>(floor(kHidingOffset * 2.0 * state)));
    } else {
      target_->DisplayCrashedFavicon();
      target_->SetFaviconHidingOffset(
          static_cast<int>(
              floor(kHidingOffset - ((state - .5) * 2.0 * kHidingOffset))));
    }
  }

  // ui::AnimationDelegate overrides:
  virtual void AnimationCanceled(const ui::Animation* animation) {
    target_->SetFaviconHidingOffset(0);
  }

 private:
  TabRendererGtk* target_;

  DISALLOW_COPY_AND_ASSIGN(FaviconCrashAnimation);
};

////////////////////////////////////////////////////////////////////////////////
// TabRendererGtk, public:

TabRendererGtk::TabRendererGtk(GtkThemeService* theme_service)
    : showing_icon_(false),
      showing_close_button_(false),
      favicon_hiding_offset_(0),
      should_display_crashed_favicon_(false),
      loading_animation_(theme_service),
      background_offset_x_(0),
      background_offset_y_(kInactiveTabBackgroundOffsetY),
      theme_service_(theme_service),
      close_button_color_(0),
      is_active_(false) {
  InitResources();

  tab_.Own(gtk_fixed_new());
  gtk_widget_set_app_paintable(tab_.get(), TRUE);
  g_signal_connect(tab_.get(), "expose-event",
                   G_CALLBACK(OnExposeEventThunk), this);
  g_signal_connect(tab_.get(), "size-allocate",
                   G_CALLBACK(OnSizeAllocateThunk), this);
  close_button_.reset(MakeCloseButton());
  gtk_widget_show(tab_.get());

  hover_animation_.reset(new ui::SlideAnimation(this));
  hover_animation_->SetSlideDuration(kHoverDurationMs);
}

TabRendererGtk::~TabRendererGtk() {
  tab_.Destroy();
}

void TabRendererGtk::UpdateData(TabContents* contents,
                                bool app,
                                bool loading_only) {
  DCHECK(contents);
  TabContentsWrapper* wrapper =
      TabContentsWrapper::GetCurrentWrapperForContents(contents);

  if (!loading_only) {
    data_.title = contents->GetTitle();
    data_.incognito = contents->browser_context()->IsOffTheRecord();
    data_.crashed = contents->is_crashed();

    SkBitmap* app_icon =
        TabContentsWrapper::GetCurrentWrapperForContents(contents)->
            extension_tab_helper()->GetExtensionAppIcon();
    if (app_icon) {
      data_.favicon = *app_icon;
    } else {
      data_.favicon = wrapper->favicon_tab_helper()->GetFavicon();
    }

    data_.app = app;

    // Make a cairo cached version of the favicon.
    if (!data_.favicon.isNull()) {
      // Instead of resizing the icon during each frame, create our resized
      // icon resource now, send it to the xserver and use that each frame
      // instead.

      // For source images smaller than the favicon square, scale them as if
      // they were padded to fit the favicon square, so we don't blow up tiny
      // falcons into larger or nonproportional results.
      int src_w = data_.favicon.width();
      int src_h = data_.favicon.height();
      float float_src_w = static_cast<float>(src_w);
      float float_src_h = static_cast<float>(src_h);
      float scalable_w, scalable_h;
      if (src_w <= gfx::kFaviconSize && src_h <= gfx::kFaviconSize) {
        scalable_w = scalable_h = gfx::kFaviconSize;
      } else {
        scalable_w = float_src_w;
        scalable_h = float_src_h;
      }

      // Scale proportionately.
      float float_size = gfx::kFaviconSize;
      float scale = std::min(float_size / scalable_w,
                             float_size / scalable_h);
      int dest_w = static_cast<int>(float_src_w * scale);
      int dest_h = static_cast<int>(float_src_h * scale);

      GdkPixbuf* pixbuf;
      if (dest_w == src_w && dest_h == src_h) {
        pixbuf = gfx::GdkPixbufFromSkBitmap(&data_.favicon);
      } else {
        SkBitmap resized_icon = skia::ImageOperations::Resize(
            data_.favicon,
            skia::ImageOperations::RESIZE_BETTER,
            dest_w, dest_h);
        pixbuf = gfx::GdkPixbufFromSkBitmap(&resized_icon);
      }

      data_.cairo_favicon.UsePixbuf(pixbuf);
      g_object_unref(pixbuf);
    } else {
      data_.cairo_favicon.Reset();
    }

    // This is kind of a hacky way to determine whether our icon is the default
    // favicon. But the plumbing that would be necessary to do it right would
    // be a good bit of work and would sully code for other platforms which
    // don't care to custom-theme the favicon. Hopefully the default favicon
    // will eventually be chromium-themable and this code will go away.
    data_.is_default_favicon =
        (data_.favicon.pixelRef() ==
        ResourceBundle::GetSharedInstance().GetBitmapNamed(
            IDR_DEFAULT_FAVICON)->pixelRef());
  }

  // Loading state also involves whether we show the favicon, since that's where
  // we display the throbber.
  data_.loading = contents->IsLoading();
  data_.show_icon = wrapper->favicon_tab_helper()->ShouldDisplayFavicon();
}

void TabRendererGtk::UpdateFromModel() {
  // Force a layout, since the tab may have grown a favicon.
  Layout();
  SchedulePaint();

  if (data_.crashed) {
    if (!should_display_crashed_favicon_ && !IsPerformingCrashAnimation())
      StartCrashAnimation();
  } else {
    if (IsPerformingCrashAnimation())
      StopCrashAnimation();
    ResetCrashedFavicon();
  }
}

void TabRendererGtk::SetBlocked(bool blocked) {
  if (data_.blocked == blocked)
    return;
  data_.blocked = blocked;
  // TODO(zelidrag) bug 32399: Make tabs pulse on Linux as well.
}

bool TabRendererGtk::is_blocked() const {
  return data_.blocked;
}

bool TabRendererGtk::IsActive() const {
  return is_active_;
}

bool TabRendererGtk::IsSelected() const {
  return true;
}

bool TabRendererGtk::IsVisible() const {
  return gtk_widget_get_visible(tab_.get());
}

void TabRendererGtk::SetVisible(bool visible) const {
  if (visible) {
    gtk_widget_show(tab_.get());
    if (data_.mini)
      gtk_widget_show(close_button_->widget());
  } else {
    gtk_widget_hide_all(tab_.get());
  }
}

bool TabRendererGtk::ValidateLoadingAnimation(AnimationState animation_state) {
  return loading_animation_.ValidateLoadingAnimation(animation_state);
}

void TabRendererGtk::PaintFaviconArea(GtkWidget* widget, cairo_t* cr) {
  DCHECK(ShouldShowIcon());

  cairo_rectangle(cr,
                  x() + favicon_bounds_.x(),
                  y() + favicon_bounds_.y(),
                  favicon_bounds_.width(),
                  favicon_bounds_.height());
  cairo_clip(cr);

  // The tab is rendered into a windowless widget whose offset is at the
  // coordinate event->area.  Translate by these offsets so we can render at
  // (0,0) to match Windows' rendering metrics.
  cairo_matrix_t cairo_matrix;
  cairo_matrix_init_translate(&cairo_matrix, x(), y());
  cairo_set_matrix(cr, &cairo_matrix);

  // Which background should we be painting?
  int theme_id;
  int offset_y = 0;
  if (IsActive()) {
    theme_id = IDR_THEME_TOOLBAR;
  } else {
    theme_id = data_.incognito ? IDR_THEME_TAB_BACKGROUND_INCOGNITO :
               IDR_THEME_TAB_BACKGROUND;

    if (!theme_service_->HasCustomImage(theme_id))
      offset_y = background_offset_y_;
  }

  // Paint the background behind the favicon.
  CairoCachedSurface* tab_bg =
      theme_service_->GetSurfaceNamed(theme_id, widget);
  tab_bg->SetSource(cr, -x(), -offset_y);
  cairo_pattern_set_extend(cairo_get_source(cr), CAIRO_EXTEND_REPEAT);
  cairo_rectangle(cr,
                  favicon_bounds_.x(), favicon_bounds_.y(),
                  favicon_bounds_.width(), favicon_bounds_.height());
  cairo_fill(cr);

  if (!IsActive()) {
    double throb_value = GetThrobValue();
    if (throb_value > 0) {
      cairo_push_group(cr);
      CairoCachedSurface* active_bg = theme_service_->GetSurfaceNamed(
          IDR_THEME_TOOLBAR, widget);
      active_bg->SetSource(cr, -x(), 0);
      cairo_pattern_set_extend(cairo_get_source(cr), CAIRO_EXTEND_REPEAT);

      cairo_rectangle(cr,
                      favicon_bounds_.x(), favicon_bounds_.y(),
                      favicon_bounds_.width(), favicon_bounds_.height());
      cairo_fill(cr);

      cairo_pop_group_to_source(cr);
      cairo_paint_with_alpha(cr, throb_value);
    }
  }

  PaintIcon(widget, cr);
}

bool TabRendererGtk::ShouldShowIcon() const {
  if (mini() && height() >= GetMinimumUnselectedSize().height()) {
    return true;
  } else if (!data_.show_icon) {
    return false;
  } else if (IsActive()) {
    // The active tab clips favicon before close button.
    return IconCapacity() >= 2;
  }
  // Non-selected tabs clip close button before favicon.
  return IconCapacity() >= 1;
}

// static
gfx::Size TabRendererGtk::GetMinimumUnselectedSize() {
  InitResources();

  gfx::Size minimum_size;
  minimum_size.set_width(kLeftPadding + kRightPadding);
  // Since we use bitmap images, the real minimum height of the image is
  // defined most accurately by the height of the end cap images.
  minimum_size.set_height(tab_active_l_height_ - kToolbarOverlap);
  return minimum_size;
}

// static
gfx::Size TabRendererGtk::GetMinimumSelectedSize() {
  gfx::Size minimum_size = GetMinimumUnselectedSize();
  minimum_size.set_width(kLeftPadding + gfx::kFaviconSize + kRightPadding);
  return minimum_size;
}

// static
gfx::Size TabRendererGtk::GetStandardSize() {
  gfx::Size standard_size = GetMinimumUnselectedSize();
  standard_size.Enlarge(kFaviconTitleSpacing + kStandardTitleWidth, 0);
  return standard_size;
}

// static
int TabRendererGtk::GetMiniWidth() {
  return browser_defaults::kMiniTabWidth;
}

// static
int TabRendererGtk::GetContentHeight() {
  // The height of the content of the Tab is the largest of the favicon,
  // the title text and the close button graphic.
  int content_height = std::max(gfx::kFaviconSize, title_font_height_);
  return std::max(content_height, close_button_height_);
}

// static
void TabRendererGtk::SetSelectedTitleColor(SkColor color) {
  selected_title_color_ = color;
}

// static
void TabRendererGtk::SetUnselectedTitleColor(SkColor color) {
  unselected_title_color_ = color;
}

gfx::Rect TabRendererGtk::GetNonMirroredBounds(GtkWidget* parent) const {
  // The tabstrip widget is a windowless widget so the tab widget's allocation
  // is relative to the browser titlebar.  We need the bounds relative to the
  // tabstrip.
  gfx::Rect bounds = GetWidgetBoundsRelativeToParent(parent, widget());
  bounds.set_x(gtk_util::MirroredLeftPointForRect(parent, bounds));
  return bounds;
}

gfx::Rect TabRendererGtk::GetRequisition() const {
  return gfx::Rect(requisition_.x(), requisition_.y(),
                   requisition_.width(), requisition_.height());
}

void TabRendererGtk::StartMiniTabTitleAnimation() {
  if (!mini_title_animation_.get()) {
    mini_title_animation_.reset(new ui::ThrobAnimation(this));
    mini_title_animation_->SetThrobDuration(kMiniTitleChangeThrobDuration);
  }

  if (!mini_title_animation_->is_animating())
    mini_title_animation_->StartThrobbing(-1);
}

void TabRendererGtk::StopMiniTabTitleAnimation() {
  if (mini_title_animation_.get())
    mini_title_animation_->Stop();
}

void TabRendererGtk::SetBounds(const gfx::Rect& bounds) {
  requisition_ = bounds;
  gtk_widget_set_size_request(tab_.get(), bounds.width(), bounds.height());
}

////////////////////////////////////////////////////////////////////////////////
// TabRendererGtk, protected:

void TabRendererGtk::Raise() const {
  if (gtk_button_get_event_window(GTK_BUTTON(close_button_->widget())))
    gdk_window_raise(gtk_button_get_event_window(
        GTK_BUTTON(close_button_->widget())));
}

string16 TabRendererGtk::GetTitle() const {
  return data_.title;
}

///////////////////////////////////////////////////////////////////////////////
// TabRendererGtk, ui::AnimationDelegate implementation:

void TabRendererGtk::AnimationProgressed(const ui::Animation* animation) {
  gtk_widget_queue_draw(tab_.get());
}

void TabRendererGtk::AnimationCanceled(const ui::Animation* animation) {
  AnimationEnded(animation);
}

void TabRendererGtk::AnimationEnded(const ui::Animation* animation) {
  gtk_widget_queue_draw(tab_.get());
}

////////////////////////////////////////////////////////////////////////////////
// TabRendererGtk, private:

void TabRendererGtk::StartCrashAnimation() {
  if (!crash_animation_.get())
    crash_animation_.reset(new FaviconCrashAnimation(this));
  crash_animation_->Stop();
  crash_animation_->Start();
}

void TabRendererGtk::StopCrashAnimation() {
  if (!crash_animation_.get())
    return;
  crash_animation_->Stop();
}

bool TabRendererGtk::IsPerformingCrashAnimation() const {
  return crash_animation_.get() && crash_animation_->is_animating();
}

void TabRendererGtk::SetFaviconHidingOffset(int offset) {
  favicon_hiding_offset_ = offset;
  SchedulePaint();
}

void TabRendererGtk::DisplayCrashedFavicon() {
  should_display_crashed_favicon_ = true;
}

void TabRendererGtk::ResetCrashedFavicon() {
  should_display_crashed_favicon_ = false;
}

void TabRendererGtk::Paint(GtkWidget* widget, cairo_t* cr) {
  // Don't paint if we're narrower than we can render correctly. (This should
  // only happen during animations).
  if (width() < GetMinimumUnselectedSize().width() && !mini())
    return;

  // See if the model changes whether the icons should be painted.
  const bool show_icon = ShouldShowIcon();
  const bool show_close_button = ShouldShowCloseBox();
  if (show_icon != showing_icon_ ||
      show_close_button != showing_close_button_)
    Layout();

  PaintTabBackground(widget, cr);

  if (!mini() || width() > kMiniTabRendererAsNormalTabWidth)
    PaintTitle(widget, cr);

  if (show_icon)
    PaintIcon(widget, cr);
}

cairo_surface_t* TabRendererGtk::PaintToSurface(GtkWidget* widget,
                                                cairo_t* cr) {
  cairo_surface_t* target = cairo_get_target(cr);
  cairo_surface_t* out_surface = cairo_surface_create_similar(
      target,
      CAIRO_CONTENT_COLOR_ALPHA,
      width(), height());

  cairo_t* out_cr = cairo_create(out_surface);
  Paint(widget, out_cr);
  cairo_destroy(out_cr);

  return out_surface;
}

void TabRendererGtk::SchedulePaint() {
  gtk_widget_queue_draw(tab_.get());
}

gfx::Rect TabRendererGtk::GetLocalBounds() {
  return gfx::Rect(0, 0, bounds_.width(), bounds_.height());
}

void TabRendererGtk::Layout() {
  gfx::Rect local_bounds = GetLocalBounds();
  if (local_bounds.IsEmpty())
    return;
  local_bounds.Inset(kLeftPadding, kTopPadding, kRightPadding, kBottomPadding);

  // Figure out who is tallest.
  int content_height = GetContentHeight();

  // Size the Favicon.
  showing_icon_ = ShouldShowIcon();
  if (showing_icon_) {
    int favicon_top = kTopPadding + (content_height - gfx::kFaviconSize) / 2;
    favicon_bounds_.SetRect(local_bounds.x(), favicon_top,
                            gfx::kFaviconSize, gfx::kFaviconSize);
    if ((mini() || data_.animating_mini_change) &&
        bounds_.width() < kMiniTabRendererAsNormalTabWidth) {
      int mini_delta = kMiniTabRendererAsNormalTabWidth - GetMiniWidth();
      int ideal_delta = bounds_.width() - GetMiniWidth();
      if (ideal_delta < mini_delta) {
        int ideal_x = (GetMiniWidth() - gfx::kFaviconSize) / 2;
        int x = favicon_bounds_.x() + static_cast<int>(
            (1 - static_cast<float>(ideal_delta) /
             static_cast<float>(mini_delta)) *
            (ideal_x - favicon_bounds_.x()));
        favicon_bounds_.set_x(x);
      }
    }
  } else {
    favicon_bounds_.SetRect(local_bounds.x(), local_bounds.y(), 0, 0);
  }

  // Size the Close button.
  showing_close_button_ = ShouldShowCloseBox();
  if (showing_close_button_) {
    int close_button_top =
        kTopPadding + kCloseButtonVertFuzz +
        (content_height - close_button_height_) / 2;
    close_button_bounds_.SetRect(local_bounds.width() + kCloseButtonHorzFuzz,
                                 close_button_top, close_button_width_,
                                 close_button_height_);

    // If the close button color has changed, generate a new one.
    if (theme_service_) {
      SkColor tab_text_color =
        theme_service_->GetColor(ThemeService::COLOR_TAB_TEXT);
      if (!close_button_color_ || tab_text_color != close_button_color_) {
        close_button_color_ = tab_text_color;
        ResourceBundle& rb = ResourceBundle::GetSharedInstance();
        close_button_->SetBackground(close_button_color_,
            rb.GetBitmapNamed(IDR_TAB_CLOSE),
            rb.GetBitmapNamed(IDR_TAB_CLOSE_MASK));
      }
    }
  } else {
    close_button_bounds_.SetRect(0, 0, 0, 0);
  }

  if (!mini() || width() >= kMiniTabRendererAsNormalTabWidth) {
    // Size the Title text to fill the remaining space.
    int title_left = favicon_bounds_.right() + kFaviconTitleSpacing;
    int title_top = kTopPadding;

    // If the user has big fonts, the title will appear rendered too far down
    // on the y-axis if we use the regular top padding, so we need to adjust it
    // so that the text appears centered.
    gfx::Size minimum_size = GetMinimumUnselectedSize();
    int text_height = title_top + title_font_height_ + kBottomPadding;
    if (text_height > minimum_size.height())
      title_top -= (text_height - minimum_size.height()) / 2;

    int title_width;
    if (close_button_bounds_.width() && close_button_bounds_.height()) {
      title_width = std::max(close_button_bounds_.x() -
                             kTitleCloseButtonSpacing - title_left, 0);
    } else {
      title_width = std::max(local_bounds.width() - title_left, 0);
    }
    title_bounds_.SetRect(title_left, title_top, title_width, content_height);
  }

  favicon_bounds_.set_x(
      gtk_util::MirroredLeftPointForRect(tab_.get(), favicon_bounds_));
  close_button_bounds_.set_x(
      gtk_util::MirroredLeftPointForRect(tab_.get(), close_button_bounds_));
  title_bounds_.set_x(
      gtk_util::MirroredLeftPointForRect(tab_.get(), title_bounds_));

  MoveCloseButtonWidget();
}

void TabRendererGtk::MoveCloseButtonWidget() {
  if (!close_button_bounds_.IsEmpty()) {
    gtk_fixed_move(GTK_FIXED(tab_.get()), close_button_->widget(),
                   close_button_bounds_.x(), close_button_bounds_.y());
    gtk_widget_show(close_button_->widget());
  } else {
    gtk_widget_hide(close_button_->widget());
  }
}

void TabRendererGtk::PaintTab(GtkWidget* widget, GdkEventExpose* event) {
  cairo_t* cr = gdk_cairo_create(GDK_DRAWABLE(widget->window));
  gdk_cairo_rectangle(cr, &event->area);
  cairo_clip(cr);

  // The tab is rendered into a windowless widget whose offset is at the
  // coordinate event->area.  Translate by these offsets so we can render at
  // (0,0) to match Windows' rendering metrics.
  cairo_matrix_t cairo_matrix;
  cairo_matrix_init_translate(&cairo_matrix, event->area.x, event->area.y);
  cairo_set_matrix(cr, &cairo_matrix);

  // Save the original x offset so we can position background images properly.
  background_offset_x_ = event->area.x;

  Paint(widget, cr);
  cairo_destroy(cr);
}

void TabRendererGtk::PaintTitle(GtkWidget* widget, cairo_t* cr) {
  if (title_bounds_.IsEmpty())
    return;

  // Paint the Title.
  string16 title = data_.title;
  if (title.empty()) {
    title = data_.loading ?
        l10n_util::GetStringUTF16(IDS_TAB_LOADING_TITLE) :
        TabContentsWrapper::GetDefaultTitle();
  } else {
    Browser::FormatTitleForDisplay(&title);
  }

  SkColor title_color = IsSelected() ? selected_title_color_
                                     : unselected_title_color_;

  DrawTextOntoCairoSurface(cr,
                           title,
                           *title_font_,
                           title_bounds_,
                           title_bounds_,
                           title_color,
                           base::i18n::IsRTL() ? gfx::Canvas::TEXT_ALIGN_RIGHT :
                           gfx::Canvas::TEXT_ALIGN_LEFT);
}

void TabRendererGtk::PaintIcon(GtkWidget* widget, cairo_t* cr) {
  if (loading_animation_.animation_state() != ANIMATION_NONE) {
    PaintLoadingAnimation(widget, cr);
    return;
  }

  CairoCachedSurface* to_display = NULL;
  if (should_display_crashed_favicon_) {
    to_display = theme_service_->GetSurfaceNamed(IDR_SAD_FAVICON, widget);
  } else if (!data_.favicon.isNull()) {
    if (data_.is_default_favicon && theme_service_->UsingNativeTheme()) {
      to_display = theme_service_->GetCairoIcon(
          GtkThemeService::NATIVE_FAVICON, widget);
    } else if (data_.cairo_favicon.valid()) {
      to_display = &data_.cairo_favicon;
    }
  }

  if (to_display) {
    to_display->SetSource(cr,
                          favicon_bounds_.x(),
                          favicon_bounds_.y() + favicon_hiding_offset_);
    cairo_paint(cr);
  }
}

void TabRendererGtk::PaintTabBackground(GtkWidget* widget, cairo_t* cr) {
  if (IsActive()) {
    PaintActiveTabBackground(widget, cr);
  } else {
    PaintInactiveTabBackground(widget, cr);

    double throb_value = GetThrobValue();
    if (throb_value > 0) {
      cairo_push_group(cr);
      PaintActiveTabBackground(widget, cr);
      cairo_pop_group_to_source(cr);
      cairo_paint_with_alpha(cr, throb_value);
    }
  }
}

void TabRendererGtk::DrawTabBackground(
    cairo_t* cr,
    GtkWidget* widget,
    CairoCachedSurface* tab_bg,
    int offset_x,
    int offset_y) {
  tab_bg->SetSource(cr, -offset_x, -offset_y);
  cairo_pattern_set_extend(cairo_get_source(cr), CAIRO_EXTEND_REPEAT);

  // Draw left edge
  CairoCachedSurface* tab_l_mask =
      theme_service_->GetSurfaceNamed(IDR_TAB_ALPHA_LEFT, widget);
  tab_l_mask->MaskSource(cr, 0, 0);

  // Draw center
  cairo_rectangle(cr,
                  tab_active_l_width_, kDropShadowOffset,
                  width() - (2 * tab_active_l_width_),
                  tab_inactive_l_height_);
  cairo_fill(cr);

  // Draw right edge
  CairoCachedSurface* tab_r_mask =
      theme_service_->GetSurfaceNamed(IDR_TAB_ALPHA_RIGHT, widget);
  tab_r_mask->MaskSource(cr, width() - tab_active_l_width_, 0);
}

void TabRendererGtk::DrawTabShadow(
    cairo_t* cr,
    GtkWidget* widget,
    int left_idr,
    int center_idr,
    int right_idr) {
  // Draw left drop shadow
  CairoCachedSurface* active_image_l =
      theme_service_->GetSurfaceNamed(left_idr, widget);
  active_image_l->SetSource(cr, 0, 0);
  cairo_paint(cr);

  // Draw the center shadow
  CairoCachedSurface* active_image_c =
      theme_service_->GetSurfaceNamed(center_idr, widget);
  active_image_c->SetSource(cr, 0, 0);
  cairo_pattern_set_extend(cairo_get_source(cr), CAIRO_EXTEND_REPEAT);
  cairo_rectangle(cr, tab_active_l_width_, 0,
                  width() - (2 * tab_active_l_width_),
                  height());
  cairo_fill(cr);

  // Draw right drop shadow
  CairoCachedSurface* active_image_r =
      theme_service_->GetSurfaceNamed(right_idr, widget);
  active_image_r->SetSource(cr, width() - active_image_r->Width(), 0);
  cairo_paint(cr);
}

void TabRendererGtk::PaintInactiveTabBackground(GtkWidget* widget,
                                                     cairo_t* cr) {
  int theme_id = data_.incognito ?
      IDR_THEME_TAB_BACKGROUND_INCOGNITO : IDR_THEME_TAB_BACKGROUND;
  if (IsSelected())
    theme_id = IDR_THEME_TAB_BACKGROUND_V;

  CairoCachedSurface* tab_bg =
      theme_service_->GetSurfaceNamed(theme_id, widget);

  // If the theme is providing a custom background image, then its top edge
  // should be at the top of the tab. Otherwise, we assume that the background
  // image is a composited foreground + frame image.
  int offset_y = theme_service_->HasCustomImage(theme_id) ?
      0 : background_offset_y_;

  DrawTabBackground(cr, widget, tab_bg, background_offset_x_, offset_y);

  DrawTabShadow(cr, widget, IDR_TAB_INACTIVE_LEFT, IDR_TAB_INACTIVE_CENTER,
                IDR_TAB_INACTIVE_RIGHT);
}

void TabRendererGtk::PaintActiveTabBackground(GtkWidget* widget,
                                                   cairo_t* cr) {
  CairoCachedSurface* tab_bg =
      theme_service_->GetSurfaceNamed(IDR_THEME_TOOLBAR, widget);

  DrawTabBackground(cr, widget, tab_bg, background_offset_x_, 0);
  DrawTabShadow(cr, widget, IDR_TAB_ACTIVE_LEFT, IDR_TAB_ACTIVE_CENTER,
                IDR_TAB_ACTIVE_RIGHT);
}

void TabRendererGtk::PaintLoadingAnimation(GtkWidget* widget,
                                                cairo_t* cr) {
  int id = loading_animation_.animation_state() == ANIMATION_WAITING ?
           IDR_THROBBER_WAITING : IDR_THROBBER;
  CairoCachedSurface* throbber = theme_service_->GetSurfaceNamed(id, widget);

  const int image_size = throbber->Height();
  const int image_offset = loading_animation_.animation_frame() * image_size;
  DCHECK(image_size == favicon_bounds_.height());
  DCHECK(image_size == favicon_bounds_.width());

  throbber->SetSource(cr, favicon_bounds_.x() - image_offset,
                      favicon_bounds_.y());
  cairo_rectangle(cr, favicon_bounds_.x(), favicon_bounds_.y(),
                  image_size, image_size);
  cairo_fill(cr);
}

int TabRendererGtk::IconCapacity() const {
  if (height() < GetMinimumUnselectedSize().height())
    return 0;
  return (width() - kLeftPadding - kRightPadding) / gfx::kFaviconSize;
}

bool TabRendererGtk::ShouldShowCloseBox() const {
  // The selected tab never clips close button.
  return !mini() && (IsActive() || IconCapacity() >= 3);
}

CustomDrawButton* TabRendererGtk::MakeCloseButton() {
  CustomDrawButton* button = new CustomDrawButton(IDR_TAB_CLOSE,
      IDR_TAB_CLOSE_P, IDR_TAB_CLOSE_H, IDR_TAB_CLOSE);

  gtk_widget_set_tooltip_text(button->widget(),
      l10n_util::GetStringUTF8(IDS_TOOLTIP_CLOSE_TAB).c_str());

  g_signal_connect(button->widget(), "clicked",
                   G_CALLBACK(OnCloseButtonClickedThunk), this);
  g_signal_connect(button->widget(), "button-release-event",
                   G_CALLBACK(OnCloseButtonMouseReleaseThunk), this);
  g_signal_connect(button->widget(), "enter-notify-event",
                   G_CALLBACK(OnEnterNotifyEventThunk), this);
  g_signal_connect(button->widget(), "leave-notify-event",
                   G_CALLBACK(OnLeaveNotifyEventThunk), this);
  gtk_widget_set_can_focus(button->widget(), FALSE);
  gtk_fixed_put(GTK_FIXED(tab_.get()), button->widget(), 0, 0);

  return button;
}

double TabRendererGtk::GetThrobValue() {
  if (mini_title_animation_.get() && mini_title_animation_->is_animating()) {
    return mini_title_animation_->GetCurrentValue() *
        kMiniTitleChangeThrobOpacity;
  }
  return hover_animation_.get() ?
      kHoverOpacity * hover_animation_->GetCurrentValue() : 0;
}

void TabRendererGtk::CloseButtonClicked() {
  // Nothing to do.
}

void TabRendererGtk::OnCloseButtonClicked(GtkWidget* widget) {
  CloseButtonClicked();
}

gboolean TabRendererGtk::OnCloseButtonMouseRelease(GtkWidget* widget,
                                                   GdkEventButton* event) {
  if (event->button == 2) {
    CloseButtonClicked();
    return TRUE;
  }

  return FALSE;
}

gboolean TabRendererGtk::OnExposeEvent(GtkWidget* widget,
                                       GdkEventExpose* event) {
  PaintTab(widget, event);
  gtk_container_propagate_expose(GTK_CONTAINER(tab_.get()),
                                 close_button_->widget(), event);
  return TRUE;
}

void TabRendererGtk::OnSizeAllocate(GtkWidget* widget,
                                    GtkAllocation* allocation) {
  gfx::Rect bounds = gfx::Rect(allocation->x, allocation->y,
                               allocation->width, allocation->height);

  // Nothing to do if the bounds are the same.  If we don't catch this, we'll
  // get an infinite loop of size-allocate signals.
  if (bounds_ == bounds)
    return;

  bounds_ = bounds;
  Layout();
}

gboolean TabRendererGtk::OnEnterNotifyEvent(GtkWidget* widget,
                                            GdkEventCrossing* event) {
  hover_animation_->SetTweenType(ui::Tween::EASE_OUT);
  hover_animation_->Show();
  return FALSE;
}

gboolean TabRendererGtk::OnLeaveNotifyEvent(GtkWidget* widget,
                                            GdkEventCrossing* event) {
  hover_animation_->SetTweenType(ui::Tween::EASE_IN);
  hover_animation_->Hide();
  return FALSE;
}

// static
void TabRendererGtk::InitResources() {
  if (initialized_)
    return;

  // Grab the pixel sizes of our masking images.
  ResourceBundle& rb = ResourceBundle::GetSharedInstance();
  SkBitmap* tab_active_l = rb.GetBitmapNamed(IDR_TAB_ACTIVE_LEFT);
  tab_active_l_width_ = tab_active_l->width();
  tab_active_l_height_ = tab_active_l->height();

  SkBitmap* tab_inactive_l = rb.GetBitmapNamed(IDR_TAB_INACTIVE_LEFT);
  tab_inactive_l_height_ = tab_inactive_l->height();

  close_button_width_ = rb.GetBitmapNamed(IDR_TAB_CLOSE)->width();
  close_button_height_ = rb.GetBitmapNamed(IDR_TAB_CLOSE)->height();

  const gfx::Font& base_font = rb.GetFont(ResourceBundle::BaseFont);
  title_font_ = new gfx::Font(base_font.GetFontName(), kFontPixelSize);
  title_font_height_ = title_font_->GetHeight();

  initialized_ = true;
}
