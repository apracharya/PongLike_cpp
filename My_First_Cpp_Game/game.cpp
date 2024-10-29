
#define is_down(b) input -> buttons[b].is_down
#define pressed(b) (input -> buttons[b].is_down && input -> buttons[b].changed)
#define released(b) (!input -> buttons[b].is_down && input -> buttons[b].changed)

//float player_pos_x = 0.f;
float player_1_p, player_1_dp, player_2_p, player_2_dp; // derivative of position
float arena_half_size_y = 45, arena_half_size_x = 85;
float player_half_size_y = 12, player_half_size_x = 2.5;
float ball_p_x, ball_p_y, ball_dp_x = 100, ball_dp_y, ball_half_size = 1;
int player_1_score, player_2_score;

internal void
simulate_player(float *p, float *dp, float ddp, float dt) {
	ddp -= *dp * 10.f; // we do this for friction
	*p = *p + *dp * dt + ddp * dt * dt * .5f; // for position
	*dp = *dp + ddp * dt; // for velocity

	// collision
	if (*p + player_half_size_y > arena_half_size_y) {
		*p = arena_half_size_y - player_half_size_y;
		*dp = 0; // for bounce player_1_dp *= -1;
	}
	else if (*p - player_half_size_y < -arena_half_size_y) {
		*p = -arena_half_size_y + player_half_size_y;
		*dp = 0; // for bounce player_1_dp *= -1;
	}
}

// 1 = ball, 2 = player
internal bool
aabb_vs_aabb(float p1x, float p1y, float hs1x, float hs1y,
	float p2x, float p2y, float hs2x, float hs2y) {
	return (
		p1x + hs1x > p2x - hs2x && // right of ball > left of player
		p1x - hs1x < p2x + hs2x && // left of ball < right of player
		p1y + hs1y > p2y - hs2y && // top of ball > bottom of player 
		p1y - hs1y < p2y + hs2y // bottom of ball < top of player
		);
}

enum Gamemode {
	GM_MENU,
	GM_GAMEPLAY
};

Gamemode current_gamemode;
bool hot_button;
bool enemy_is_ai;
// to move the rectangle
internal void
simulate_game(Input* input, float dt) {
	clear_screen(0xff5500);
	draw_rect(0, 0, arena_half_size_x, arena_half_size_y, 0xffaa33);

	if(current_gamemode == GM_GAMEPLAY) {

		// for player 1
		float player_1_ddp = 0.f; // units per second

		// for single player ai
		if (!enemy_is_ai) { // when bool is 0, ai
			if (is_down(BUTTON_UP)) player_1_ddp += 2000;
			if (is_down(BUTTON_DOWN)) player_1_ddp -= 2000;
			//if (pressed(BUTTON_LEFT)) player_pos_x -= player_ddp;
			//if (pressed(BUTTON_RIGHT)) player_pos_x += player_ddp;
		}
		else {
			//if (ball_p_y > player_1_p) player_1_ddp += 1000;
			//if (ball_p_y < player_1_p) player_1_ddp -= 1000;

			player_1_ddp = (ball_p_y - player_1_p) * 100;
			if (player_1_ddp > 1000) player_1_ddp = 1300;
			if (player_1_ddp < -1000) player_1_ddp = -1300;
		}

		// for player 2
		float player_2_ddp = 0.f; // units per second
		if (is_down(BUTTON_W)) player_2_ddp += 2000;
		if (is_down(BUTTON_S)) player_2_ddp -= 2000;

		simulate_player(&player_1_p, &player_1_dp, player_1_ddp, dt);
		simulate_player(&player_2_p, &player_2_dp, player_2_ddp, dt);
	
		// simulate ball
		{

			ball_p_x += ball_dp_x * dt;
			ball_p_y += ball_dp_y * dt;

			// collision with players
			if (aabb_vs_aabb(ball_p_x, ball_p_y, ball_half_size, ball_half_size, 80, player_1_p, player_half_size_x, player_half_size_y)
				) {
				ball_p_x = 80 - player_half_size_x - ball_half_size;
				ball_dp_x *= -1;

				// changes direction when ball hits on edges
				// + changes velocity y according to player movement while collision
				// intensity off velocity rising can be changed by multiplying
				ball_dp_y = (ball_p_y - player_1_p) * 2 + player_1_dp * 0.75f;
			}
			else if (aabb_vs_aabb(ball_p_x, ball_p_y, ball_half_size, ball_half_size, -80, player_2_p, player_half_size_x, player_half_size_y)
				) {
				ball_p_x = -80 + player_half_size_x + ball_half_size;

				//ball_dp_x *= -1; // bounces back
				ball_dp_x = ball_dp_x * -1; // ... - player_2_dp increases x speed if player if moving
				ball_dp_y = (ball_p_y - player_2_p) * 2 + player_2_dp * 0.75f;

			}


			// collision with arena top bottom
			if (ball_p_y + ball_half_size > arena_half_size_y) { // collision for arena top
				ball_p_y = arena_half_size_y - ball_half_size;
				ball_dp_y *= -1;
			}
			else if (ball_p_y - ball_half_size < -arena_half_size_y) { // collision for arena bottom
				ball_p_y = -arena_half_size_y + ball_half_size;
				ball_dp_y *= -1;
			}

			// collision with arena left and right --> out
			if (ball_p_x + ball_half_size > arena_half_size_x) {
				ball_p_x = 0;
				ball_p_y = 0;
				ball_dp_x *= -1;
				ball_dp_y = 0;
				player_2_score++;
			}
			else if (ball_p_x - ball_half_size < -arena_half_size_x) {
				ball_p_x = 0;
				ball_p_y = 0;
				ball_dp_x *= -1;
				ball_dp_y = 0;
				player_1_score++;
			}
		}

		draw_number(player_1_score, 10, 40, 1.f, 0xbbffbb);
		draw_number(player_2_score, -10, 40, 1.f, 0xbbffbb);

		// draw rects for each score
		float at_x = 80;
		for (int i = 0; i < player_1_score; i++) {
			draw_rect(at_x, 47.f, 1.f, 1.f, 0xaaaaaa);
			at_x -= 3.f;
		}

		at_x = -80;
		for (int i = 0; i < player_2_score; i++) {
			draw_rect(at_x, 47.f, 1.f, 1.f, 0xaaaaaa);
			at_x += 3.f;
		}
	
		// render ball
		draw_rect(ball_p_x, ball_p_y, ball_half_size, ball_half_size, 0xffffff);

		// players
		draw_rect(80, player_1_p, player_half_size_x, player_half_size_y, 0xdddddd);
		draw_rect(-80, player_2_p, player_half_size_x, player_half_size_y, 0xff0000);
	} else {

		if (pressed(BUTTON_LEFT) || pressed(BUTTON_RIGHT)) {
			hot_button = !hot_button;
		}

		if (pressed(BUTTON_ENTER)) {
			current_gamemode = GM_GAMEPLAY;
			enemy_is_ai = hot_button ? 0 : 1;
		}

		if (hot_button == 0) {
			draw_rect(20, 0, 10, 10, 0xff0000);
			draw_rect(-20, 0, 10, 10, 0xcccccc);
		}
		else {
			draw_rect(20, 0, 10, 10, 0xcccccc);
			draw_rect(-20, 0, 10, 10, 0xff0000);
		}
		
	}
}
