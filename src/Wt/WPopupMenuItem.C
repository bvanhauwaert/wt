/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/lexical_cast.hpp>

#include "Wt/WApplication"
#include "Wt/WCheckBox"
#include "Wt/WAnchor"
#include "Wt/WCssDecorationStyle"
#include "Wt/WPopupMenu"
#include "Wt/WPopupMenuItem"
#include "Wt/WText"

namespace {
  int ICON_WIDTH = 24;
  int CHECKBOX_WIDTH = 20;
  int SUBMENU_ARROW_WIDTH = 24;
}

namespace Wt {

WPopupMenuItem::WPopupMenuItem(bool)
  : text_(0),
    checkBox_(0),
    subMenu_(0),
    data_(0),
    separator_(true),
    selectable_(false),
    rendered_(false),
    triggered_(this)
{
  setImplementation(impl_ = new WAnchor());
  impl_->setLoadLaterWhenInvisible(false);
  setStyleClass("Wt-separator");
  setInline(false);
}

WPopupMenuItem::WPopupMenuItem(const WString& text)
  : text_(0),
    checkBox_(0),
    subMenu_(0),
    data_(0),
    separator_(false),
    selectable_(true),
    rendered_(false),
    triggered_(this)
{
  create();

  setText(text);
}

WPopupMenuItem::WPopupMenuItem(const std::string& iconPath, const WString& text)
  : text_(0),
    checkBox_(0),
    subMenu_(0),
    data_(0),
    separator_(false),
    selectable_(true),
    rendered_(false),
    triggered_(this)
{
  create();

  setText(text);

  if (!iconPath.empty())
    setIcon(iconPath);
}

WPopupMenuItem::~WPopupMenuItem()
{
  delete subMenu_;
}

void WPopupMenuItem::create()
{
  setImplementation(impl_ = new WAnchor());
  setInline(false);

  implementStateless(&WPopupMenuItem::renderOver, &WPopupMenuItem::renderOut);
  impl_->mouseWentUp().connect(this, &WPopupMenuItem::onMouseUp);

  setStyleClass("Wt-item");
}

void WPopupMenuItem::load()
{
  WCompositeWidget::load();

  impl_->mouseWentOver().connect(this, &WPopupMenuItem::renderOver);
  impl_->mouseWentOver().setNotExposed();
}

void WPopupMenuItem::render(WFlags<RenderFlag> flags)
{
  if ((flags & RenderFull) && selectable_ && !rendered_) {
    rendered_ = true;
    impl_->mouseWentUp().connect(topLevelMenu(), &WPopupMenu::hide);
  }

  WCompositeWidget::render(flags);
}

void WPopupMenuItem::setDisabled(bool disabled)
{
  if (disabled)
    addStyleClass("Wt-disabled");
  else
    removeStyleClass("Wt-disabled");

  resetLearnedSlot(&WPopupMenuItem::renderOver);

  WCompositeWidget::setDisabled(disabled);
}

void WPopupMenuItem::setText(const WString& text)
{
  if (!text_) {
    text_ = new WText(impl_);
    text_->setInline(false);
    text_->setMargin(ICON_WIDTH, Left);
    text_->setMargin(3, Right);
    text_->setAttributeValue
      ("style", "padding-right: "
       + boost::lexical_cast<std::string>(SUBMENU_ARROW_WIDTH) + "px");
  }

  text_->setText(text);
}

const WString& WPopupMenuItem::text() const
{
  return text_->text();
}

void WPopupMenuItem::setIcon(const std::string& path)
{
  decorationStyle().
    setBackgroundImage(WLink(path), WCssDecorationStyle::NoRepeat, CenterY);
  setAttributeValue("style", "background-position: 3px center");
}

std::string WPopupMenuItem::icon()
{
  return decorationStyle().backgroundImage();
}

void WPopupMenuItem::setCheckable(bool checkable)
{
  if (isCheckable() != checkable) {
    if (checkable) {
      text_->setMargin(ICON_WIDTH - CHECKBOX_WIDTH, Left);
      checkBox_ = new WCheckBox();
      impl_->insertWidget(0, checkBox_);
      text_->setInline(true);
    } else {
      delete checkBox_;
      text_->setMargin(ICON_WIDTH, Left);
      text_->setInline(false);
    }
  }
}

void WPopupMenuItem::setLink(const WLink& link)
{
  impl_->setLink(link);
}

WLink WPopupMenuItem::link() const
{
  return impl_->link();
}

void WPopupMenuItem::setLinkTarget(AnchorTarget target)
{
  impl_->setTarget(target);
}

AnchorTarget WPopupMenuItem::linkTarget() const
{
  return impl_->target();
}

void WPopupMenuItem::setPopupMenu(WPopupMenu *menu)
{
  delete subMenu_;
  subMenu_ = menu;

  selectable_ = !subMenu_;

  std::string resources = WApplication::resourcesUrl();

  if (subMenu_) {
    subMenu_->webWidget()->setLoadLaterWhenInvisible(false);
    subMenu_->parentItem_ = this;
    text_->decorationStyle().
      setBackgroundImage(WLink(resources + "right-arrow.gif"),
			 WCssDecorationStyle::NoRepeat, Right | CenterY);
  }
}

void WPopupMenuItem::setSelectable(bool selectable)
{
  selectable_ = selectable;
}

void WPopupMenuItem::setChecked(bool checked)
{
  if (checkBox_)
    checkBox_->setChecked(checked);
}

bool WPopupMenuItem::isChecked() const
{
  return checkBox_ ? checkBox_->isChecked() : false;
}

void WPopupMenuItem::renderOver()
{
  parentMenu()->renderOutAll();

  if (!isDisabled())
    renderSelected(true);
}

void WPopupMenuItem::renderOut()
{
  if (!isDisabled())
    renderSelected(false);
}

void WPopupMenuItem::renderSelected(bool selected)
{
  if (separator_)
    return;

  if (selected) {
    addStyleClass("Wt-selected", true); removeStyleClass("Wt-item", true);
  } else {
    addStyleClass("Wt-item", true); removeStyleClass("Wt-selected", true);
  }

  if (subMenu_) {
    if (selected)
      subMenu_->popupToo(this);
    else {
      subMenu_->show();
      subMenu_->hide();
    }
  }
}

void WPopupMenuItem::onMouseUp()
{
  if (isDisabled() || !selectable_)
    return;

  if (checkBox_)
    checkBox_->setChecked(!checkBox_->isChecked());

  topLevelMenu()->result_ = this;

  triggered_.emit(this);

  topLevelMenu()->done(this);
}

WPopupMenu *WPopupMenuItem::parentMenu()
{
  return dynamic_cast<WPopupMenu *>(parent()->parent()->parent());
}

WPopupMenu *WPopupMenuItem::topLevelMenu()
{
  return parentMenu()->topLevelMenu();
}

}
