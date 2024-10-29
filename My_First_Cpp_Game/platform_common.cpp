struct Button_State {
	bool is_down;
	bool changed;
};

enum {
	BUTTON_UP,
	BUTTON_DOWN,
	BUTTON_LEFT,
	BUTTON_RIGHT,
	BUTTON_W,
	BUTTON_S,
	BUTTON_ENTER,

	BUTTON_COUNT // should be the last item
};

struct Input {
	Button_State buttons[BUTTON_COUNT];
};