#include <stdio.h>
#include <stdlib.h>
#include <gccore.h>
#include <wiiuse/wpad.h>
#include <mruby.h>
#include <mruby/irep.h>
#include <mruby/string.h>
#include <mruby/array.h>

static void *xfb = NULL;
static GXRModeObj *rmode = NULL;

extern const uint8_t program[];

static mrb_value print_msg(mrb_state *mrb, mrb_value self) {
  char *unwrapped_content;
  mrb_value str_content;

  mrb_get_args(mrb, "S", &str_content);
  unwrapped_content = mrb_str_to_cstr(mrb, str_content);
  printf("%s\n", unwrapped_content);
  //char* clear_str = "Press START";
  //bfont_draw_str(vram_s + 640 * 100 + 16, 640, 1, clear_str);
  //printf("Hello mruby world!");

  return mrb_nil_value();
}

static mrb_value get_button_masks(mrb_state *mrb, mrb_value self) {
  // unipmelmented
  mrb_value mask_array;
  mask_array = mrb_ary_new(mrb);
  mrb_ary_push(mrb, mask_array, mrb_fixnum_value(0));
  mrb_ary_push(mrb, mask_array, mrb_fixnum_value(0));
  mrb_ary_push(mrb, mask_array, mrb_fixnum_value(0));
  mrb_ary_push(mrb, mask_array, mrb_fixnum_value(0));
  mrb_ary_push(mrb, mask_array, mrb_fixnum_value(0));
  mrb_ary_push(mrb, mask_array, mrb_fixnum_value(0));
  mrb_ary_push(mrb, mask_array, mrb_fixnum_value(0));
  return mask_array;
}

static mrb_value draw20x20_640(mrb_state *mrb, mrb_value self) {
  // unimplemented
  return mrb_nil_value();
}

static mrb_value init_controller_buffer(mrb_state *mrb, mrb_value self) {
  // unimplemented
  return mrb_nil_value();
}

static mrb_value start_controller_reader(mrb_state *mrb, mrb_value self) {
  // unimplemented
  return mrb_fixnum_value(0);
}

static mrb_value get_button_state(mrb_state *mrb, mrb_value self) {
  // unimplemented
  return mrb_fixnum_value(0);
}

static mrb_value get_button_states(mrb_state *mrb, mrb_value self) {
  // unimplemented
  return mrb_fixnum_value(0);
}

static mrb_value start_btn(mrb_state *mrb, mrb_value self) {
  // unimplemented
  return mrb_fixnum_value(0);
}

static mrb_value dpad_left(mrb_state *mrb, mrb_value self) {
  // unimplemented
  return mrb_fixnum_value(0);
}

static mrb_value dpad_right(mrb_state *mrb, mrb_value self) {
  // unimplemented
  return mrb_fixnum_value(0);
}

static mrb_value dpad_up(mrb_state *mrb, mrb_value self) {
  // unimplemented
  return mrb_fixnum_value(0);
}

static mrb_value dpad_down(mrb_state *mrb, mrb_value self) {
  // unimplemented
  return mrb_fixnum_value(0);
}

static mrb_value btn_a(mrb_state *mrb, mrb_value self) {
  // unimplemented
  return mrb_fixnum_value(0);
}

static mrb_value btn_b(mrb_state *mrb, mrb_value self) {
  // unimplemented
  return mrb_fixnum_value(0);
}

static mrb_value clear_score(mrb_state *mrb, mrb_value self) {
  char* clear_str = "Press START";
	printf("\x1b[4;60H");
	printf("%s", clear_str);
  return mrb_nil_value();
}

static mrb_value render_score(mrb_state *mrb, mrb_value self) {
  struct mrb_value score;
  mrb_get_args(mrb, "i", &score);
  char buf[20];
  snprintf(buf, 20, "Score: %8" PRId32, mrb_fixnum(score));
	printf("\x1b[4;60H");
	printf("%s", buf);

  return mrb_nil_value();
}

static mrb_value get_current_button_index(mrb_state *mrb, mrb_value self) {
  // unimplemented
  return mrb_fixnum_value(0);
}

static mrb_value waitvbl(mrb_state *mrb, mrb_value self) {
  // unimplemented
  return mrb_nil_value();
}

static mrb_value get_next_button_state(mrb_state *mrb, mrb_value self) {
  // unimplemented
  return mrb_nil_value();
}


//---------------------------------------------------------------------------------
int main(int argc, char **argv) {
//---------------------------------------------------------------------------------

	// Initialise the video system
	VIDEO_Init();

	// This function initialises the attached controllers
	WPAD_Init();

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

	//printf("Hello Wii World!");

	while(1) {

		// Call WPAD_ScanPads each loop, this reads the latest controller states
		WPAD_ScanPads();

		// WPAD_ButtonsDown tells us which buttons were pressed in this loop
		// this is a "one shot" state which will not fire again until the button has been released
		u32 pressed = WPAD_ButtonsDown(0);

		// We return to the launcher application via exit
		if ( pressed & WPAD_BUTTON_HOME ) exit(0);

		// Wait for the next frame
		VIDEO_WaitVSync();
	}

	return 0;
}
