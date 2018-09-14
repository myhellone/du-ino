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
 * DU-INO Prophet VS Envelope Generator
 * Aaron Mavrinac <aaron@logick.ca>
 *
 * JACK    FUNCTION
 * ----    --------
 * GT1 O -
 * GT2 O -
 * GT3 I - gate in
 * GT4 I -
 * CI1   -
 * CI2   -
 * CI3   - VCA audio in
 * CI4   -
 * OFFST -
 * CO1   - 10V envelope out
 * CO2   -
 * CO3   - 5V envelope out
 * CO4   -
 * FNCTN - VCA audio out
 *
 * SWITCH CONFIGURATION
 * --------------------
 * SG2    [_][_]    SG1
 * SG4    [^][^]    SG3
 * SC2    [^][^]    SC1
 * SC4    [^][_]    SC3
 */

#include <du-ino_function.h>
#include <du-ino_widgets.h>
#include <du-ino_save.h>
#include <avr/pgmspace.h>

static const unsigned char icons[] PROGMEM =
{
  0x3e, 0x63, 0x5d, 0x5d, 0x63, 0x3e, 0x00,  // 0
  0x3e, 0x7f, 0x7b, 0x41, 0x7f, 0x3e, 0x00,  // 1
  0x3e, 0x5b, 0x4d, 0x55, 0x5b, 0x3e, 0x00,  // 2
  0x3e, 0x6b, 0x5d, 0x55, 0x6b, 0x3e, 0x00,  // 3
  0x3e, 0x73, 0x75, 0x77, 0x41, 0x3e, 0x00   // 4
};

static const uint16_t rate_lut[] PROGMEM =
{
  11, 13, 15, 18, 21, 25, 28, 32, 36, 40, 45, 50, 55, 61, 68, 74, 82, 90, 98, 107, 117, 128, 140, 152, 165, 180, 195,
  212, 230, 249, 270, 293, 317, 343, 371, 402, 434, 470, 508, 549, 593, 640, 691, 746, 806, 870, 939, 1013, 1093, 1180,
  1273, 1373, 1481, 1597, 1722, 1857, 2003, 2160, 2328, 2510, 2707, 2918, 3145, 3391, 3655, 3940, 4246, 4577, 4933,
  5317, 5730, 6176, 6656, 7173, 7731, 8331, 8978, 9675, 10426, 11236, 12108, 13047, 14060, 15150, 16326, 17592, 18956,
  20427, 22011, 23717, 25556, 27538, 29673, 31973, 34452, 37122, 40000
};

void gate_isr();

void loop_scroll_callback(int delta);
void repeat_scroll_callback(int delta);
// TODO: declare points callbacks

class DU_VSEG_Function : public DUINO_Function
{
public:
  DU_VSEG_Function() : DUINO_Function(0b10111100) { }

  virtual void setup()
  {
    // initialize values
    gate_ = retrigger_ = false;

    // build widget hierarchy
    widget_save_ = new DUINO_SaveWidget<ParameterValues>(121, 0);
    widget_loop_ = new DUINO_DisplayWidget(0, 12, 96, 7, DUINO_Widget::Corners);
    widget_loop_->attach_scroll_callback(loop_scroll_callback);
    widget_repeat_ = new DUINO_DisplayWidget(115, 11, 13, 9, DUINO_Widget::Full);
    widget_repeat_->attach_scroll_callback(repeat_scroll_callback);
    // TODO: widgets points
    // TODO: attach points callbacks
    container_loop_repeat_ = new DUINO_WidgetContainer<2>(DUINO_Widget::Click);
    container_loop_repeat_->attach_child(widget_loop_, 0);
    container_loop_repeat_->attach_child(widget_repeat_, 1);
    container_points_ = new DUINO_WidgetContainer<8>(DUINO_Widget::Click);
    // TODO: attach widgets points
    container_outer_ = new DUINO_WidgetContainer<3>(DUINO_Widget::DoubleClick, 1);
    container_outer_->attach_child(widget_save_, 0);
    container_outer_->attach_child(container_loop_repeat_, 1);
    container_outer_->attach_child(container_points_, 2);

    // load parameters
    widget_save_->load_params();
    sanitize_level();
    sanitize_rate();
    sanitize_loop();
    sanitize_repeat();

    // draw title
    Display.draw_du_logo_sm(0, 0, DUINO_SH1106::White);
    Display.draw_text(16, 0, "VSEG", DUINO_SH1106::White);

    // draw save box
    Display.fill_rect(widget_save_->x() + 1, widget_save_->y() + 1, 5, 5, DUINO_SH1106::White);

    // draw fixed elements
    Display.draw_char(54, 0, 'L', DUINO_SH1106::White);
    Display.draw_pixel(61, 1, DUINO_SH1106::White);
    Display.draw_pixel(61, 5, DUINO_SH1106::White);
    Display.draw_char(79, 0, 'R', DUINO_SH1106::White);
    Display.draw_pixel(86, 1, DUINO_SH1106::White);
    Display.draw_pixel(86, 5, DUINO_SH1106::White);
    Display.draw_char(widget_repeat_->x() + 1, widget_repeat_->y() + 1, 'x', DUINO_SH1106::White);

    // draw parameters
    display_loop(false);
    display_repeat(false);
    display_envelope(false);

    // initialize widgets
    widget_setup(container_outer_);

    // output full display
    Display.display();

    // attach gate interrupt
    gt_attach_interrupt(GT3, gate_isr, CHANGE);
  }

  virtual void loop()
  {
    // TODO: envelope function

    widget_loop();
  }

  void gate_callback()
  {
    gate_ = gt_read_debounce(GT3);
    if (gate_)
    {
      retrigger_ = true;
    }
  }

  void widget_loop_scroll_callback(int delta)
  {
    const int8_t loop_last = widget_save_->params.vals.loop;

    widget_save_->params.vals.loop += delta;
    sanitize_loop();
    if (widget_save_->params.vals.loop != loop_last)
    {
      widget_save_->mark_changed();
      widget_save_->display();
    }

    display_loop();
  }

  void widget_repeat_scroll_callback(int delta)
  {
    const int8_t repeat_last = widget_save_->params.vals.repeat;

    widget_save_->params.vals.repeat += delta;
    sanitize_repeat();
    if (widget_save_->params.vals.repeat != repeat_last)
    {
      widget_save_->mark_changed();
      widget_save_->display();
    }

    display_repeat();
  }

  // TODO: points callbacks

private:
  void sanitize_level()
  {
    for (uint8_t p = 0; p < 4; ++p)
    {
      if (widget_save_->params.vals.level[p] < 0)
      {
        widget_save_->params.vals.level[p] = 0;
      }
      else if (widget_save_->params.vals.level[p] > 99)
      {
        widget_save_->params.vals.level[p] = 99;
      }
    }
  }

  void sanitize_rate()
  {
    for (uint8_t p = 0; p < 4; ++p)
    {
      if (widget_save_->params.vals.rate[p] < 0)
      {
        widget_save_->params.vals.rate[p] = 0;
      }
      else if (widget_save_->params.vals.rate[p] > 96)
      {
        widget_save_->params.vals.rate[p] = 96;
      }
    }
  }

  void sanitize_loop()
  {
    if (widget_save_->params.vals.loop < -1)
    {
      widget_save_->params.vals.loop = -1;
    }
    else if (widget_save_->params.vals.loop > 5)
    {
      widget_save_->params.vals.loop = 5;
    }
  }

  void sanitize_repeat()
  {
    if (widget_save_->params.vals.repeat < 0)
    {
      widget_save_->params.vals.repeat = 0;
    }
    else if (widget_save_->params.vals.repeat > 7)
    {
      widget_save_->params.vals.repeat = 7;
    }
  }

  float level_to_cv(uint8_t p)
  {
    if (p == 4)
    {
      return 0.0;
    }

    return (float)widget_save_->params.vals.level[p] / 9.9;
  }

  uint16_t rate_to_ms(uint8_t p)
  {
    if (p == 0)
    {
      return 0;
    }

    return (uint16_t)pgm_read_word(&rate_lut[widget_save_->params.vals.rate[p - 1]]);
  }

  uint8_t level_to_y(uint8_t p)
  {
    if (p == 4)
    {
      return 57;
    }

    return 24 + (99 - widget_save_->params.vals.level[p]) / 3;
  }

  uint8_t rate_to_x(uint8_t p)
  {
    if (p == 0)
    {
      return 0;
    }

    return rate_to_x(p - 1) + 6 + (widget_save_->params.vals.rate[p - 1] / 4) + (p == 4 ? 2 : 0);
  }

  void display_plr(uint8_t p, bool update = true)
  {
    // blank areas
    Display.fill_rect(45, 0, 5, 7, DUINO_SH1106::Black);
    Display.fill_rect(64, 0, 11, 7, DUINO_SH1106::Black);
    Display.fill_rect(89, 0, 29, 7, DUINO_SH1106::Black);

    // display point number
    Display.draw_char(45, 0, '0' + p, DUINO_SH1106::White);

    uint8_t i;
    
    // display level
    int8_t level = widget_save_->params.vals.level[p];
    i = 0;
    while (level)
    {
      Display.draw_char(70 - 6 * i, 0, '0' + level % 10, DUINO_SH1106::White);
      level /= 10;
      i++;
    }

    // display rate
    uint16_t rate = rate_to_ms(p);
    i = 0;
    while (rate)
    {
      Display.draw_char(113 - 6 * i, 0, '0' + rate % 10, DUINO_SH1106::White);
      rate /= 10;
      i++;
    }

    if (update)
    {
      Display.display(45, 117, 0, 0);
    }
  }

  void display_loop(bool update = true)
  {
    // blank area
    Display.fill_rect(widget_loop_->x() + 2, widget_loop_->y() + 1, widget_loop_->width() - 4,
        widget_loop_->height() - 2, DUINO_SH1106::Black);

    if (widget_save_->params.vals.loop > -1)
    {
      const uint8_t x1 = rate_to_x(widget_save_->params.vals.loop % 3) + 2;
      const uint8_t x2 = rate_to_x(3) + 3;
      const uint8_t w = x2 - x1 + 1;

      // draw shaft
      Display.draw_hline(x1, widget_loop_->y() + 3, w, DUINO_SH1106::White);

      // draw forward arrow head
      Display.draw_vline(x2 - 1, widget_loop_->y() + 2, 3, DUINO_SH1106::White);
      Display.draw_vline(x2 - 2, widget_loop_->y() + 1, 5, DUINO_SH1106::White);
      
      // draw reverse arrow head
      if (widget_save_->params.vals.loop > 2)
      {
        Display.draw_vline(x1 + 1, widget_loop_->y() + 2, 3, DUINO_SH1106::White);
        Display.draw_vline(x1 + 2, widget_loop_->y() + 1, 5, DUINO_SH1106::White);
      }
    }

    if (update)
    {
      widget_loop_->display();
    }
  }

  void display_repeat(bool update = true)
  {
    Display.fill_rect(widget_repeat_->x() + 7, widget_repeat_->y() + 1, 5, 7,
        widget_repeat_->inverted() ? DUINO_SH1106::White : DUINO_SH1106::Black);

    const unsigned char c = widget_save_->params.vals.repeat ? '0' + widget_save_->params.vals.repeat : 'C';
    Display.draw_char(widget_repeat_->x() + 7, widget_repeat_->y() + 1, c,
        widget_repeat_->inverted() ? DUINO_SH1106::Black : DUINO_SH1106::White);

    if (update)
    {
      widget_repeat_->display();
    }
  }

  void display_envelope(bool update = true)
  {
    Display.fill_rect(0, 24, 128, 40, DUINO_SH1106::Black);

    for (uint8_t p = 0; p < 5; ++p)
    {
      const uint8_t x = rate_to_x(p);
      const uint8_t y = level_to_y(p);

      Display.draw_bitmap_7(x, y, icons, p, DUINO_SH1106::White);

      if (p < 3)
      {
        Display.draw_line(x + 5, y + 3, rate_to_x(p + 1), level_to_y(p + 1) + 3, DUINO_SH1106::White);
      }
      else if (p == 3)
      {
        Display.draw_hline(x + 6, y + 3, 2, DUINO_SH1106::White);
        // TODO: draw exponential decay to 4
      }
    }

    if (update)
    {
      Display.display(0, 127, 3, 7);
    }
  }

  struct ParameterValues
  {
    int8_t level[4];  // levels of points 0 - 3 (0 - 99)
    int8_t rate[4];   // rates of points 1 - 4 (0 - 96)
    int8_t loop;      // -1 = off, loop % 3 = loop start, loop > 2 = loop back and forth
    int8_t repeat;    // 0 = continuous, 1 - 7 = finite repeats
  };

  DUINO_WidgetContainer<3> * container_outer_;
  DUINO_WidgetContainer<2> * container_loop_repeat_;
  DUINO_WidgetContainer<8> * container_points_;
  DUINO_SaveWidget<ParameterValues> * widget_save_;
  DUINO_DisplayWidget * widget_loop_;
  DUINO_DisplayWidget * widget_repeat_;
  DUINO_Widget * widgets_points_[8];

  volatile bool gate_, retrigger_;
};

DU_VSEG_Function * function;

void gate_isr() { function->gate_callback(); }

void loop_scroll_callback(int delta) { function->widget_loop_scroll_callback(delta); }
void repeat_scroll_callback(int delta) { function->widget_repeat_scroll_callback(delta); }
// TODO: define points callbacks

void setup()
{
  function = new DU_VSEG_Function();

  function->begin();
}

void loop()
{
  function->loop();
}
