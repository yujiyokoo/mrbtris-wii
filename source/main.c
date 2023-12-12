#include <stdio.h>
#include <stdlib.h>
#include <gccore.h>
#include <wiiuse/wpad.h>
#include <mruby.h>
#include <mruby/irep.h>
#include <mruby/string.h>
#include <mruby/array.h>

static u32 *xfb = NULL;
static GXRModeObj *rmode = NULL;

#define BUFSIZE 100

struct InputBuf {
  uint16_t buffer[BUFSIZE];
  uint32_t index;
} input_buf;

static mrb_value btn_mrb_buffer;

extern const uint8_t program[];

static mrb_value print_msg(mrb_state *mrb, mrb_value self) {
  char *unwrapped_content;
  mrb_value str_content;

  mrb_get_args(mrb, "S", &str_content);
  unwrapped_content = mrb_str_to_cstr(mrb, str_content);
  printf("\x1b[10;60H");
  printf("%s\n", unwrapped_content);

  return mrb_nil_value();
}

static mrb_value get_button_masks(mrb_state *mrb, mrb_value self) {
  mrb_value mask_array;
  mask_array = mrb_ary_new(mrb);

  mrb_ary_push(mrb, mask_array, mrb_fixnum_value(PAD_BUTTON_START));
  mrb_ary_push(mrb, mask_array, mrb_fixnum_value(PAD_BUTTON_LEFT));
  mrb_ary_push(mrb, mask_array, mrb_fixnum_value(PAD_BUTTON_RIGHT));
  mrb_ary_push(mrb, mask_array, mrb_fixnum_value(PAD_BUTTON_UP));
  mrb_ary_push(mrb, mask_array, mrb_fixnum_value(PAD_BUTTON_DOWN));
  mrb_ary_push(mrb, mask_array, mrb_fixnum_value(PAD_BUTTON_A));
  mrb_ary_push(mrb, mask_array, mrb_fixnum_value(PAD_BUTTON_B));

  return mask_array;
}

static u32 CvtRGB (u8 r1, u8 g1, u8 b1, u8 r2, u8 g2, u8 b2)
{
	int y1, cb1, cr1, y2, cb2, cr2, cb, cr;

	y1 = (299 * r1 + 587 * g1 + 114 * b1) / 1000;
	cb1 = (-16874 * r1 - 33126 * g1 + 50000 * b1 + 12800000) / 100000;
	cr1 = (50000 * r1 - 41869 * g1 - 8131 * b1 + 12800000) / 100000;

	y2 = (299 * r2 + 587 * g2 + 114 * b2) / 1000;
	cb2 = (-16874 * r2 - 33126 * g2 + 50000 * b2 + 12800000) / 100000;
	cr2 = (50000 * r2 - 41869 * g2 - 8131 * b2 + 12800000) / 100000;

	cb = (cb1 + cb2) >> 1;
	cr = (cr1 + cr2) >> 1;

	return (y1 << 24) | (cb << 16) | (y2 << 8) | cr;
}

static u32 PACK_PIXEL(int r, int g, int b) {
  return CvtRGB(r, g, b, r, g, b);
}

static mrb_value draw20x20_640(mrb_state *mrb, mrb_value self) {
  mrb_int x, y, r, g, b;
  mrb_get_args(mrb, "iiiii", &x, &y, &r, &g, &b);

  int i = 0, j = 0;

  if(r == 0 && g == 0 && b == 0) {
    for(i = 0; i < 20; i++) {
      for(j = 0; j < 10; j++) {
        xfb[x+j + (y+i) * 320] = PACK_PIXEL(r, g, b);
      }
    }
  } else {
//    for(i = 0; i < 20; i++) {
//      for(j = 0; j < 10; j++) {
//        xfb[x+j + (y+i) * 320] = PACK_PIXEL(r, g, b);
//      }
//    }
    int r_light = (r+128 <= 255) ? r+128 : 255;
    int g_light = (g+128 <= 255) ? g+128 : 255;
    int b_light = (b+128 <= 255) ? b+128 : 255;

    int r_dark = (r-64 >= 0) ? r-64 : 0;
    int g_dark = (g-64 >= 0) ? g-64 : 0;
    int b_dark = (b-64 >= 0) ? b-64 : 0;

    // TODO: implement lines and use them.
    for(j = 0; j < 10; j++) {
      xfb[x+j + (y) * 320] = PACK_PIXEL(30, 30, 30);
      xfb[x+j + (y+19) * 320] = PACK_PIXEL(30, 30, 30);
    }
    for(j = 1; j < 9; j++) {
      xfb[x+j + (y+1) * 320] = PACK_PIXEL(r_light, g_light, b_light);
    }
    for(j = 2; j < 10; j++) {
      xfb[x+j + (y+19) * 320] = PACK_PIXEL(r_dark, g_dark, b_dark);
    }
    for(i = 2; i < 19; i++) {
      xfb[x + (y+i) * 320] = PACK_PIXEL(30, 30, 30);
      xfb[x+1 + (y+i) * 320] = PACK_PIXEL(r_light, g_light, b_light);
      for(j = 2; j < 9; j++) {
        xfb[x+j + (y+i) * 320] = PACK_PIXEL(r, g, b);
      }
      xfb[x+9 + (y+i) * 320] = PACK_PIXEL(r_dark, g_dark, b_dark);
    }
  }

  return mrb_nil_value();
}

static mrb_value init_controller_buffer(mrb_state *mrb, mrb_value self) {
  btn_mrb_buffer = mrb_ary_new(mrb);;
  input_buf.index = 0;

  int i = 0;
  while(i < BUFSIZE) {
    mrb_ary_set(mrb, btn_mrb_buffer, i, mrb_nil_value());
    input_buf.buffer[i] = 0; i ++ ;
  }

  return mrb_nil_value();
}

void *read_buttons() {
  while(1) {
    input_buf.index = (input_buf.index + 1) % BUFSIZE;
		PAD_ScanPads();
		u16 btns = PAD_ButtonsHeld(0);
		input_buf.buffer[input_buf.index] = btns;

		LWP_YieldThread();
  }
}

static mrb_value start_controller_reader(mrb_state *mrb, mrb_value self) {
  // unimplemented
  lwp_t thread;
  LWP_CreateThread(&thread, read_buttons, NULL, NULL, 0, 0);
  return mrb_fixnum_value(0);
}

static mrb_value get_button_state(mrb_state *mrb, mrb_value self) {
  PAD_ScanPads();
	u16 btns = PAD_ButtonsHeld(0);
  return mrb_fixnum_value(btns);
}

static mrb_value get_button_states(mrb_state *mrb, mrb_value self) {
  // unimplemented
  return mrb_fixnum_value(0);
}

static mrb_value start_btn(mrb_state *mrb, mrb_value self) {
  mrb_int state;
  mrb_get_args(mrb, "i", &state);

  return mrb_bool_value(state & PAD_BUTTON_START);
}

static mrb_value dpad_left(mrb_state *mrb, mrb_value self) {
  mrb_int state;
  mrb_get_args(mrb, "i", &state);

  return mrb_bool_value(state & PAD_BUTTON_LEFT);
}

static mrb_value dpad_right(mrb_state *mrb, mrb_value self) {
  mrb_int state;
  mrb_get_args(mrb, "i", &state);

  return mrb_bool_value(state & PAD_BUTTON_RIGHT);
}

static mrb_value dpad_up(mrb_state *mrb, mrb_value self) {
  // unimplemented
  return mrb_fixnum_value(0);
}

static mrb_value dpad_down(mrb_state *mrb, mrb_value self) {
  mrb_int state;
  mrb_get_args(mrb, "i", &state);

  return mrb_bool_value(state & PAD_BUTTON_DOWN);
}

static mrb_value btn_a(mrb_state *mrb, mrb_value self) {
  mrb_int state;
  mrb_get_args(mrb, "i", &state);

  return mrb_bool_value(state & PAD_BUTTON_A);
}

static mrb_value btn_b(mrb_state *mrb, mrb_value self) {
  mrb_int state;
  mrb_get_args(mrb, "i", &state);

  return mrb_bool_value(state & PAD_BUTTON_B);
}

static mrb_value clear_score(mrb_state *mrb, mrb_value self) {
  char* clear_str = "Press START";
  printf("\x1b[4;60H");
  printf("%s", clear_str);
  return mrb_nil_value();
}

static mrb_value render_score(mrb_state *mrb, mrb_value self) {
  mrb_int score;
  mrb_get_args(mrb, "i", &score);
  char buf[20];
  snprintf(buf, 20, "Score: %8" PRId32, score);
  printf("\x1b[4;60H");
  printf("%s", buf);

  return mrb_nil_value();
}

static mrb_value get_current_button_index(mrb_state *mrb, mrb_value self) {
  // unimplemented
  return mrb_fixnum_value(0);
}

static mrb_value waitvbl(mrb_state *mrb, mrb_value self) {
  VIDEO_WaitVSync();
  return mrb_nil_value();
}

static mrb_value get_next_button_state(mrb_state *mrb, mrb_value self) {
  mrb_int wanted_index;
  mrb_get_args(mrb, "i", &wanted_index);
  int curr_index = input_buf.index;

  if(wanted_index >= BUFSIZE || wanted_index < 0) { wanted_index = wanted_index % BUFSIZE; }

  if(wanted_index == (curr_index + 1) % BUFSIZE) {
    return mrb_nil_value();
  } else {
    return mrb_fixnum_value(input_buf.buffer[wanted_index]);
  }
}


//---------------------------------------------------------------------------------
int main(int argc, char **argv) {
  //---------------------------------------------------------------------------------
  // Initialise the video system
  VIDEO_Init();

  // This function initialises the attached controllers
  //WPAD_Init();
  PAD_Init();

  // Obtain the preferred video mode from the system
  // This will correspond to the settings in the Wii menu
  rmode = VIDEO_GetPreferredMode(NULL);

  // Allocate memory for the display in the uncached region
  xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));

  // Initialise the console, required for printf
  console_init(xfb,20,20,rmode->fbWidth,rmode->xfbHeight,rmode->fbWidth*VI_DISPLAY_PIX_SZ);

  // Set up the video registers with the chosen mode
  VIDEO_Configure(rmode);

  // Tell the video hardware where our display memory is
  VIDEO_SetNextFramebuffer(xfb);

  // Make the display visible
  VIDEO_SetBlack(FALSE);

  // Flush the video register changes to the hardware
  VIDEO_Flush();

  // Wait for Video setup to complete
  VIDEO_WaitVSync();
  if(rmode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();


  // The console understands VT terminal escape codes
  // This positions the cursor on row 2, column 0
  // we can use variables for this with format codes too
  // e.g. printf ("\x1b[%d;%dH", row, column );
  printf("\x1b[2;0H");

  mrb_state *mrb = mrb_open();
  if (!mrb) { return 1; }
  struct RClass *mwii_module = mrb_define_module(mrb, "MrbtrisWii");
  //define_module_functions(mrb, mwii_module);
  mrb_define_module_function(mrb, mwii_module, "print_msg", print_msg, MRB_ARGS_REQ(1));
  mrb_define_module_function(mrb, mwii_module, "get_button_masks", get_button_masks, MRB_ARGS_NONE());
  mrb_define_module_function(mrb, mwii_module, "clear_score", clear_score, MRB_ARGS_NONE());
  mrb_define_module_function(mrb, mwii_module, "draw20x20_640", draw20x20_640, MRB_ARGS_REQ(5));
  mrb_define_module_function(mrb, mwii_module, "init_controller_buffer", init_controller_buffer, MRB_ARGS_NONE());
  mrb_define_module_function(mrb, mwii_module, "start_controller_reader", start_controller_reader, MRB_ARGS_NONE());

  mrb_define_module_function(mrb, mwii_module, "get_button_state", get_button_state, MRB_ARGS_NONE());
  mrb_define_module_function(mrb, mwii_module, "get_button_states", get_button_states, MRB_ARGS_REQ(1));
  mrb_define_module_function(mrb, mwii_module, "start_btn?", start_btn, MRB_ARGS_REQ(1));
  mrb_define_module_function(mrb, mwii_module, "dpad_left?", dpad_left, MRB_ARGS_REQ(1));
  mrb_define_module_function(mrb, mwii_module, "dpad_right?", dpad_right, MRB_ARGS_REQ(1));
  mrb_define_module_function(mrb, mwii_module, "dpad_up?", dpad_up, MRB_ARGS_REQ(1));
  mrb_define_module_function(mrb, mwii_module, "dpad_down?", dpad_down, MRB_ARGS_REQ(1));
  mrb_define_module_function(mrb, mwii_module, "btn_a?", btn_a, MRB_ARGS_REQ(1));
  mrb_define_module_function(mrb, mwii_module, "btn_b?", btn_b, MRB_ARGS_REQ(1));
  mrb_define_module_function(mrb, mwii_module, "render_score", render_score, MRB_ARGS_REQ(1));
  mrb_define_module_function(mrb, mwii_module, "get_current_button_index", get_current_button_index, MRB_ARGS_NONE());
  mrb_define_module_function(mrb, mwii_module, "waitvbl", waitvbl, MRB_ARGS_NONE());
  mrb_define_module_function(mrb, mwii_module, "get_next_button_state", get_next_button_state, MRB_ARGS_REQ(1));

  mrb_load_irep(mrb, program);

	return 0;
}
