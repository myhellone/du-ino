/*
 * ####                                                ####
 * ####                                                ####
 * ####                                                ####      ##
 * ####                                                ####    ####
 * ####  ############  ############  ####  ##########  ####  ####
 * ####  ####    ####  ####    ####  ####  ####        ########
 * ####  ####    ####  ####    ####  ####  ####        ########
 * ####  ####    ####  ####    ####  ####  ####        ####  ####
 * ####  ####    ####  ####    ####  ####  ####        ####    ####
 * ####  ############  ############  ####  ##########  ####      ####
 *                             ####                                ####
 * ################################                                  ####
 *            __      __              __              __      __       ####
 *   |  |    |  |    [__)    |_/     (__     |__|    |  |    [__)        ####
 *   |/\|    |__|    |  \    |  \    .__)    |  |    |__|    |             ##
 *
 *
 * DU-INO Arduino Library - Interface Widget Library
 * Aaron Mavrinac <aaron@logick.ca>
 */

#ifndef DUINO_WIDGETS_H_
#define DUINO_WIDGETS_H_

#include "Arduino.h"
#include "du-ino_sh1106.h"

/** Display object (UI element) abstract base class. */
class DUINO_DisplayObject
{
public:
  DUINO_DisplayObject(uint8_t x, uint8_t y);

  virtual void display();

  virtual uint8_t x() const { return x_; }
  virtual uint8_t y() const { return y_; }
  virtual uint8_t width() const = 0;
  virtual uint8_t height() const = 0;

protected:
  const uint8_t x_, y_;
};

/** Widget class. */
class DUINO_Widget
{
public:
  enum Action
  {
    Click,
    DoubleClick,
    Scroll
  };

  enum InvertStyle
  {
    Full,
    Box,
    DottedBox,
    Corners
  };

  DUINO_Widget();

  virtual void invert(bool update_display = true) { }
  virtual bool inverted() const { return false; }

  virtual void on_click();
  virtual void on_double_click();
  virtual void on_scroll(int delta);

  void attach_click_callback(void (*callback)());
  void attach_double_click_callback(void (*callback)());
  void attach_scroll_callback(void (*callback)(int));

protected:
  static void draw_invert(uint8_t x, uint8_t y, uint8_t width, uint8_t height, InvertStyle style);

  void (*click_callback_)();
  void (*double_click_callback_)();
  void (*scroll_callback_)(int);
};

/** Display widget (widget with visual UI element) class. */
class DUINO_DisplayWidget : public DUINO_Widget, public DUINO_DisplayObject
{
public:
  DUINO_DisplayWidget(uint8_t x, uint8_t y, uint8_t width, uint8_t height, InvertStyle style);

  virtual void invert(bool update_display = true);
  virtual bool inverted() const { return inverted_; }

  virtual uint8_t width() const { return width_; }
  virtual uint8_t height() const { return height_; }

protected:
  const uint8_t width_, height_;
  const InvertStyle style_;
  bool inverted_;
};

/** N-element widget array abstract base class template. */
template <uint8_t N>
class DUINO_WidgetArray : public DUINO_Widget
{
public:
  DUINO_WidgetArray(DUINO_Widget::Action t, uint8_t initial_selection = 0)
    : type_(t)
    , selected_(initial_selection)
    , click_callback_array_(NULL)
    , double_click_callback_array_(NULL)
    , scroll_callback_array_(NULL)
  { }

  void select(uint8_t selection)
  {
    const bool show = inverted();
    if(show)
    {
      invert_selected();
    }
    if (selection < N)
    {
      selected_ = selection;
    }
    if(show)
    {
      invert_selected();
    }
  }

  void select_delta(int delta)
  {
    const bool show = inverted();
    if(show)
    {
      invert_selected();
    }
    selected_ += delta;
    selected_ %= N;
    if (selected_ < 0)
    {
      selected_ += N;
    }
    if(show)
    {
      invert_selected();
    }
  }

  void select_prev()
  {
    const bool show = inverted();
    if(show)
    {
      invert_selected();
    }
    selected_--;
    if (selected_ < 0)
    {
      selected_ = N;
    }
    if(show)
    {
      invert_selected();
    }
  }

  void select_next()
  {
    const bool show = inverted();
    if(show)
    {
      invert_selected();
    }
    selected_++;
    selected_ %= N;
    if(show)
    {
      invert_selected();
    }
  }

  int selected() const { return selected_; }

  virtual void on_click()
  {
    switch (type_)
    {
      case DUINO_Widget::Click:
        select_next();
        break;
      default:
        click_default();
        if (click_callback_array_)
        {
          click_callback_array_(selected_);
        }
        break;
    }

    if(click_callback_)
    {
      click_callback_();
    }
  }

  virtual void on_double_click()
  {
    switch (type_)
    {
      case DUINO_Widget::DoubleClick:
        select_next();
        break;
      default:
        double_click_default();
        if(double_click_callback_array_)
        {
          double_click_callback_array_(selected_);
        }
        break;
    }

    if(double_click_callback_)
    {
      double_click_callback_();
    }
  }

  virtual void on_scroll(int delta)
  {
    if(delta == 0)
    {
      return;
    }

    switch (type_)
    {
      case DUINO_Widget::Scroll:
        select_delta(delta);
        break;
      default:
        scroll_default(delta);
        if(scroll_callback_array_)
        {
          scroll_callback_array_(selected_, delta);
        }
        break;
    }

    if(scroll_callback_)
    {
      scroll_callback_(delta);
    }
  }

  void attach_click_callback_array(void (*callback)(uint8_t)) { click_callback_array_ = callback; }
  void attach_double_click_callback_array(void (*callback)(uint8_t)) { double_click_callback_array_ = callback; }
  void attach_scroll_callback_array(void (*callback)(uint8_t, int)) { scroll_callback_array_ = callback; }

protected:
  virtual void invert_selected() = 0;

  virtual void click_default() { }
  virtual void double_click_default() { }
  virtual void scroll_default(int delta) { }

  const Action type_;
  int selected_;

  void (*click_callback_array_)(uint8_t);
  void (*double_click_callback_array_)(uint8_t);
  void (*scroll_callback_array_)(uint8_t, int);
};

/** Multi-display-widget (a horizontal or vertical array of similar widgets) class template. */
template <uint8_t N>
class DUINO_MultiDisplayWidget : public DUINO_WidgetArray<N>, public DUINO_DisplayObject
{
public:
  DUINO_MultiDisplayWidget(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t step, bool vertical,
    DUINO_Widget::InvertStyle style, DUINO_Widget::Action t, uint8_t initial_selection = 0)
    : width_(width)
    , height_(height)
    , step_(step)
    , vertical_(vertical)
    , style_(style)
    , inverted_(false)
    , DUINO_WidgetArray<N>(t, initial_selection)
    , DUINO_DisplayObject(x, y)
  { }

  virtual void display()
  {
    Display.display(x(this->selected_), x(this->selected_) + width() - 1, y(this->selected_) >> 3,
        (y(this->selected_) + height() - 1) >> 3);
  }

  virtual void invert(bool update_display = true)
  {
    this->draw_invert(x(this->selected_), y(this->selected_), width(), height(), style_);

    if (update_display)
    {
      this->display();
    }

    inverted_ = !inverted_;
  }

  virtual bool inverted() const { return inverted_; }

  using DUINO_DisplayObject::x;
  using DUINO_DisplayObject::y;
  uint8_t x(uint8_t i) const { return this->x() + (vertical_ ? 0 : i * step_); }
  uint8_t y(uint8_t i) const { return this->y() + (vertical_ ? i * step_ : 0); }
  virtual uint8_t width() const { return width_; }
  virtual uint8_t height() const { return height_; }

protected:
  virtual void invert_selected()
  {
    invert();
  }

  const uint8_t width_, height_, step_;
  const bool vertical_;
  const DUINO_Widget::InvertStyle style_;
  bool inverted_;
};

/** Widget container (organizer of functional widget hierarchy) class template. */
template <uint8_t N>
class DUINO_WidgetContainer : public DUINO_WidgetArray<N>
{
public:
  DUINO_WidgetContainer(DUINO_Widget::Action t, uint8_t initial_selection = 0)
    : DUINO_WidgetArray<N>(t, initial_selection)
  {
    for (uint8_t i = 0; i < N; ++i)
    {
      children_[i] = NULL;
    } 
  }

  virtual void invert(bool update_display = true)
  {
    if (children_[this->selected_])
    {
      children_[this->selected_]->invert(update_display);
    }
  }

  virtual bool inverted() const
  {
    if (children_[this->selected_])
    {
      return children_[this->selected_]->inverted();
    }
  }

  void attach_child(DUINO_Widget * child, uint8_t position)
  {
    children_[position] = child;
  }

  DUINO_Widget * get_child(uint8_t i) { return children_[i]; }

protected:
  virtual void invert_selected()
  {
    if (children_[this->selected_])
    {
      children_[this->selected_]->invert();
    }
  }

  virtual void click_default()
  {
    if (children_[this->selected_])
    {
      children_[this->selected_]->on_click();
    }
  }

  virtual void double_click_default()
  {
    if (children_[this->selected_])
    {
      children_[this->selected_]->on_double_click();
    }
  }

  virtual void scroll_default(int delta)
  {
    if (children_[this->selected_])
    {
      children_[this->selected_]->on_scroll(delta);
    }
  }

  DUINO_Widget * children_[N];
};

#endif // DUINO_WIDGETS_H_
