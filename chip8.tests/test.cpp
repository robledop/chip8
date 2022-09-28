#include "pch.h"
extern "C" {
#include "SDL.h"
#include "chip8.h"
}

const char keyboard_map[CHIP_TOTAL_KEYS] = {
	SDLK_0, SDLK_1, SDLK_2, SDLK_3, SDLK_4, SDLK_5, SDLK_6, SDLK_7,
	SDLK_8, SDLK_9, SDLK_a, SDLK_b, SDLK_c, SDLK_d, SDLK_e, SDLK_f
};

const char chip8_default_character_set[] = {
	0xF0,0x90,0x90,0x90,0xF0,	// 0
	0x20,0x60,0x20,0x20,0x70,	// 1
	0xF0,0x10,0xF0,0x80,0xF0,	// 2
	0xF0,0x10,0xF0,0x10,0xF0,	// 3
	0x90,0x90,0xF0,0x10,0x10,	// 4
	0xF0,0x80,0xF0,0x10,0xF0,	// 5
	0xF0,0x80,0xF0,0x90,0xF0,	// 6
	0xF0,0x10,0x20,0x40,0x40,	// 7
	0xF0,0x90,0xF0,0x90,0xF0,	// 8
	0xF0,0x90,0xF0,0x10,0xF0,	// 9
	0xF0,0x90,0xF0,0x90,0x90,	// A
	0xE0,0x90,0xE0,0x90,0xE0,	// B
	0xF0,0x80,0x80,0x80,0xF0,	// C
	0xE0,0x90,0x90,0x90,0xE0,	// D
	0xF0,0x80,0xF0,0x80,0xF0,	// E
	0xF0,0x80,0xF0,0x80,0x80	// F
};

TEST(Init, load_character_set) {

	chip8 chip8{};
	chip8_init(&chip8);
	const int result = memcmp(chip8.memory.memory, chip8_default_character_set, sizeof(chip8_default_character_set));
	EXPECT_EQ(result, 0);
}

TEST(Keyboard, load_keyboard_map) {

	chip8 chip8{};
	chip8_init(&chip8);
	chip8_keyboard_set_map(&chip8.keyboard, keyboard_map);
	if (chip8.keyboard.keyboard_map == NULL)
	{
		FAIL();
	}
	const int result = memcmp(chip8.keyboard.keyboard_map, keyboard_map, sizeof(keyboard_map));
	EXPECT_EQ(result, 0);
}

TEST(Keyboard, keyboard_key_down) {
	chip8 chip8{};
	chip8_init(&chip8);
	chip8_keyboard_set_map(&chip8.keyboard, keyboard_map);

	bool is_down = chip8_keyboard_is_down(&chip8.keyboard, 9);
	EXPECT_FALSE(is_down);

	chip8_keyboard_down(&chip8.keyboard, 9);
	is_down = chip8_keyboard_is_down(&chip8.keyboard, 9);

	EXPECT_TRUE(is_down);
	EXPECT_TRUE(chip8.keyboard.keyboard[9]);
}

TEST(Keyboard, keyboard_key_up) {
	chip8 chip8{};
	chip8_init(&chip8);
	chip8_keyboard_set_map(&chip8.keyboard, keyboard_map);

	chip8_keyboard_down(&chip8.keyboard, 9);
	bool is_down = chip8_keyboard_is_down(&chip8.keyboard, 9);
	EXPECT_TRUE(is_down);

	chip8_keyboard_up(&chip8.keyboard, 9);
	is_down = chip8_keyboard_is_down(&chip8.keyboard, 9);
	EXPECT_FALSE(is_down);
	EXPECT_FALSE(chip8.keyboard.keyboard[9]);
}

TEST(Keyboard, keyboard_map) {

	chip8 chip8{};
	chip8_init(&chip8);
	chip8_keyboard_set_map(&chip8.keyboard, keyboard_map);
	const int result = chip8_keyboard_map(&chip8.keyboard, '5');
	EXPECT_EQ(result, 5);
}

// 00E0 - CLS
// Clear the display.
TEST(Instructions, CLS) {
	chip8 chip8{};
	chip8_init(&chip8);

	chip8.screen.pixels[0][1] = true;
	chip8.screen.pixels[3][60] = true;
	chip8.screen.pixels[5][23] = true;
	chip8.screen.pixels[2][1] = true;
	chip8.screen.pixels[0][33] = true;

	chip8_exec(&chip8, 0x00E0);
	EXPECT_FALSE(chip8.screen.pixels[0][1]);
	EXPECT_FALSE(chip8.screen.pixels[3][60]);
	EXPECT_FALSE(chip8.screen.pixels[5][23]);
	EXPECT_FALSE(chip8.screen.pixels[2][1] );
	EXPECT_FALSE(chip8.screen.pixels[0][33]);
}

// 00EE - RET
// Return from a subroutine.
// The interpreter sets the program counter to the address at the top of the stack, then subtracts 1 from the stack pointer.
TEST(Instructions, RET) {
	chip8 chip8{};
	chip8_init(&chip8);
	chip8_stack_push(&chip8, 0x10);
	chip8_stack_push(&chip8, 0x10);
	chip8_stack_push(&chip8, 0x11);

	chip8_exec(&chip8, 0x00EE);
	EXPECT_EQ(chip8.registers.PC, 0x11);
	EXPECT_EQ(chip8.registers.SP, 2);
}

TEST(Instructions, JP_addr) {
	chip8 chip8{};
	chip8_init(&chip8);

	chip8_exec(&chip8, 0x1000);
	EXPECT_EQ(chip8.registers.PC, 0x00);

	chip8_exec(&chip8, 0x1555);
	EXPECT_EQ(chip8.registers.PC, 0x555);
}

TEST(Instructions, CALL_addr) {
	chip8 chip8{};
	chip8_init(&chip8);

	chip8_exec(&chip8, 0x1000);
	chip8_exec(&chip8, 0x2000);
	EXPECT_EQ(chip8.registers.PC, 0x00);
	EXPECT_EQ(chip8_stack_pop(&chip8), 0x00);

	chip8_exec(&chip8, 0x1222);
	chip8_exec(&chip8, 0x2555);
	EXPECT_EQ(chip8.registers.PC, 0x555);
	EXPECT_EQ(chip8_stack_pop(&chip8), 0x222);
}

// 3xkk - SE Vx, byte
// Skip next instruction if Vx = kk.
TEST(Instructions, SE_Vx_byte) {
	chip8 chip8{};
	chip8_init(&chip8);

	chip8.registers.PC = 0x10;
	chip8.registers.V[0x07] = 0x55;

	chip8_exec(&chip8, 0x3755);

	EXPECT_EQ(chip8.registers.PC, 0x12);

	chip8.registers.PC = 0x10;
	chip8.registers.V[0x07] = 0x44;

	chip8_exec(&chip8, 0x3755);

	EXPECT_EQ(chip8.registers.PC, 0x10);
}

// 4xkk - SNE Vx, byte
// Skip next instruction if Vx != kk.
TEST(Instructions, SNE_Vx_byte) {
	chip8 chip8{};
	chip8_init(&chip8);

	chip8.registers.PC = 0x10;
	chip8.registers.V[0x07] = 0x55;

	chip8_exec(&chip8, 0x4755);

	EXPECT_EQ(chip8.registers.PC, 0x10);

	chip8.registers.PC = 0x10;
	chip8.registers.V[0x07] = 0x44;

	chip8_exec(&chip8, 0x4755);

	EXPECT_EQ(chip8.registers.PC, 0x12);
}

// 5xy0 - SE Vx, Vy
// Skip next instruction if Vx = Vy.
TEST(Instructions, SE_Vx_Vy) {
	chip8 chip8{};
	chip8_init(&chip8);

	chip8.registers.PC = 0x10;
	chip8.registers.V[0x07] = 0x55;
	chip8.registers.V[0x05] = 0x55;

	chip8_exec(&chip8, 0x5750);

	EXPECT_EQ(chip8.registers.PC, 0x12);

	chip8.registers.PC = 0x10;
	chip8.registers.V[0x07] = 0x55;
	chip8.registers.V[0x05] = 0x44;
	chip8_exec(&chip8, 0x5750);

	EXPECT_EQ(chip8.registers.PC, 0x10);
}

// 6xkk - LD Vx, byte
// Set Vx = kk.
TEST(Instructions, LD_Vx_byte) {
	chip8 chip8{};
	chip8_init(&chip8);

	chip8.registers.V[0x07] = 0x00;
	chip8_exec(&chip8, 0x6755);
	EXPECT_EQ(chip8.registers.V[0x07], 0x55);
}

// 7xkk - ADD Vx, byte
// Set Vx = Vx + kk.
TEST(Instructions, ADD_Vx_byte) {
	chip8 chip8{};
	chip8_init(&chip8);

	chip8.registers.V[0x07] = 0x11;
	chip8_exec(&chip8, 0x7722);
	EXPECT_EQ(chip8.registers.V[0x07], 0x33);
}

// 8xy0 - LD Vx, Vy
// Set Vx = Vy.
TEST(Instructions, LD_Vx_Vy) {
	chip8 chip8{};
	chip8_init(&chip8);

	chip8.registers.V[0x01] = 0x11;
	chip8.registers.V[0x02] = 0x22;

	chip8_exec(&chip8, 0x8120);

	EXPECT_EQ(chip8.registers.V[0x01], 0x22);
}

// 8xy1 - OR Vx, Vy
// Set Vx = Vx OR Vy.
TEST(Instructions, OR_Vx_Vy) {
	chip8 chip8{};
	chip8_init(&chip8);

	chip8.registers.V[0x01] = 0x11;
	chip8.registers.V[0x02] = 0x22;

	chip8_exec(&chip8, 0x8121);

	EXPECT_EQ(chip8.registers.V[0x01], 0x11 | 0x22);
}

// 8xy2 - AND Vx, Vy
// Set Vx = Vx AND Vy.
TEST(Instructions, AND_Vx_Vy) {
	chip8 chip8{};
	chip8_init(&chip8);

	chip8.registers.V[0x01] = 0x11;
	chip8.registers.V[0x02] = 0x22;

	chip8_exec(&chip8, 0x8122);

	EXPECT_EQ(chip8.registers.V[0x01], 0x11 & 0x22);
}

// 8xy3 - XOR Vx, Vy
// Set Vx = Vx XOR Vy.
TEST(Instructions, XOR_Vx_Vy) {
	chip8 chip8{};
	chip8_init(&chip8);

	chip8.registers.V[0x01] = 0x11;
	chip8.registers.V[0x02] = 0x22;

	chip8_exec(&chip8, 0x8123);

	EXPECT_EQ(chip8.registers.V[0x01], 0x11 ^ 0x22);
}


// 8xy4 - ADD Vx, Vy
// Set Vx = Vx + Vy, set VF = carry
TEST(Instructions, ADD_Vx_Vy) {
	chip8 chip8{};
	chip8_init(&chip8);

	chip8.registers.V[0x01] = 0x11;
	chip8.registers.V[0x02] = 0x22;

	chip8_exec(&chip8, 0x8124);

	EXPECT_EQ(chip8.registers.V[0x01], 0x11 + 0x22);
	EXPECT_EQ(chip8.registers.V[0x0F], 0);


	chip8.registers.V[0x01] = 0xFF;
	chip8.registers.V[0x02] = 0x22;

	chip8_exec(&chip8, 0x8124);

	printf("0xFF + 0x22 = %d\n", 0xFF + 0x22);
	printf("chip8.registers.V[0x01] = %d\n", chip8.registers.V[0x01]);

	EXPECT_EQ(chip8.registers.V[0x01], (char)(0xFF + 0x22));
	EXPECT_EQ(chip8.registers.V[0x0F], 0x01);
}

// 8xy5 - SUB Vx, Vy
// Set Vx = Vx - Vy, set VF = NOT borrow.
TEST(Instructions, SUB_Vx_Vy) {
	chip8 chip8{};
	chip8_init(&chip8);

	chip8.registers.V[0x01] = 0x22;
	chip8.registers.V[0x02] = 0x11;

	chip8_exec(&chip8, 0x8125);

	EXPECT_EQ(chip8.registers.V[0x01], 0x22 - 0x11);
	EXPECT_EQ(chip8.registers.V[0x0F], 0x01);

	chip8.registers.V[0x01] = 0x11;
	chip8.registers.V[0x02] = 0x22;

	chip8_exec(&chip8, 0x8125);

	EXPECT_EQ(chip8.registers.V[0x01], (unsigned char)(0x11 - 0x22));
	EXPECT_EQ(chip8.registers.V[0x0F], 0x00);
}

// 8xy6 - SHR Vx {, Vy}
// Set Vx = Vx SHR 1.
// If the least-significant bit of Vx is 1, then VF is set to 1, otherwise 0. Then Vx is divided by 2.
TEST(Instructions, SHR_Vx__Vy) {
	chip8 chip8{};
	chip8_init(&chip8);

	chip8.registers.V[0x01] = 0b00001001;

	chip8_exec(&chip8, 0x8126);

	EXPECT_EQ(chip8.registers.V[0x0F], 0x01);
	EXPECT_EQ(chip8.registers.V[0x01], (unsigned char)(0b00001001 / 2));

	chip8.registers.V[0x01] = 0b00001000;

	chip8_exec(&chip8, 0x8126);

	EXPECT_EQ(chip8.registers.V[0x0F], 0x00);
	EXPECT_EQ(chip8.registers.V[0x01], (unsigned char)(0b00001000 / 2));
}

// 8xy7 - SUBN Vx, Vy
// Set Vx = Vy - Vx, set VF = NOT borrow.
// If Vy > Vx, then VF is set to 1, otherwise 0. Then Vx is subtracted from Vy, and the results stored in Vx.
TEST(Instructions, SUBN_Vx_Vy) {
	chip8 chip8{};
	chip8_init(&chip8);

	chip8.registers.V[0x01] = 0x22;
	chip8.registers.V[0x02] = 0x11;

	chip8_exec(&chip8, 0x8127);

	EXPECT_EQ(chip8.registers.V[0x01], (unsigned char)(0x11 - 0x22));
	EXPECT_EQ(chip8.registers.V[0x0F], 0x00);

	chip8.registers.V[0x01] = 0x11;
	chip8.registers.V[0x02] = 0x22;

	chip8_exec(&chip8, 0x8127);

	EXPECT_EQ(chip8.registers.V[0x01], (unsigned char)(0x22 - 0x11));
	EXPECT_EQ(chip8.registers.V[0x0F], 0x01);
}

// 8xyE - SHL Vx {, Vy}
// Set Vx = Vx SHL 1.
// If the most-significant bit of Vx is 1, then VF is set to 1, otherwise to 0. Then Vx is multiplied by 2.
TEST(Instructions, SHL_Vx__Vy) {
	chip8 chip8{};
	chip8_init(&chip8);

	chip8.registers.V[0x01] = 0b00010000;

	chip8_exec(&chip8, 0x812E);

	EXPECT_EQ(chip8.registers.V[0x01], (unsigned char)(0b00010000 * 2));
	EXPECT_EQ(chip8.registers.V[0x0F], 0x00);


	chip8.registers.V[0x01] = 0b10010000;

	chip8_exec(&chip8, 0x812E);

	EXPECT_EQ(chip8.registers.V[0x01], (unsigned char)(0b10010000 * 2));
	EXPECT_EQ(chip8.registers.V[0x0F], 0x01);
}

// 9xy0 - SNE Vx, Vy
// Skip next instruction if Vx != Vy.
TEST(Instructions, SNE_Vx_Vy) {
	chip8 chip8{};
	chip8_init(&chip8);

	chip8.registers.PC = 0x10;
	chip8.registers.V[0x01] = 0x11;
	chip8.registers.V[0x02] = 0x22;

	chip8_exec(&chip8, 0x9120);

	EXPECT_EQ(chip8.registers.PC, 0x12);

	chip8.registers.PC = 0x10;
	chip8.registers.V[0x01] = 0x11;
	chip8.registers.V[0x02] = 0x11;

	chip8_exec(&chip8, 0x9120);

	EXPECT_EQ(chip8.registers.PC, 0x10);
}

// Annn - LD I, addr
// Set I = nnn.
TEST(Instructions, LD_I_addr) {
	chip8 chip8{};
	chip8_init(&chip8);

	chip8.registers.I = 0x10;

	chip8_exec(&chip8, 0xA333);

	EXPECT_EQ(chip8.registers.I, 0x333);
}

// Bnnn - JP V0, addr
// Jump to location nnn + V0.
TEST(Instructions, JP_V0_addr) {
	chip8 chip8{};
	chip8_init(&chip8);

	chip8.registers.PC = 0x10;
	chip8.registers.V[0x00] = 0x11;

	chip8_exec(&chip8, 0xB333);

	EXPECT_EQ(chip8.registers.PC, (unsigned short)(0x333 + 0x11));
}

// Cxkk - RND Vx, byte
// Set Vx = random byte AND kk.
TEST(Instructions, RND_Vx_byte) {
	chip8 chip8{};
	chip8_init(&chip8);

	chip8.registers.V[0x00] = 0x11;

	chip8_exec(&chip8, 0xC033);

	EXPECT_NE(chip8.registers.V[0x00], 0x11);
}

// Dxyn - DRW Vx, Vy, nibble
// Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
// The interpreter reads n bytes from memory, starting at the address stored in I.
// These bytes are then displayed as sprites on screen at coordinates (Vx, Vy).
// Sprites are XORed onto the existing screen. If this causes any pixels to be erased,
// VF is set to 1, otherwise it is set to 0. If the sprite is positioned so part of it is
// outside the coordinates of the display, it wraps around to the opposite side of the screen.
TEST(Instructions, DRW_Vx_Vy_nibble) {
	chip8 chip8{};
	chip8_init(&chip8);

	chip8.registers.I = 0x00;
	
	chip8_exec(&chip8, 0xD005);


	EXPECT_TRUE(chip8.screen.pixels[0][0]);
	EXPECT_TRUE(chip8.screen.pixels[0][1]);
	EXPECT_TRUE(chip8.screen.pixels[0][2]);
	EXPECT_TRUE(chip8.screen.pixels[0][3]);

	EXPECT_TRUE(chip8.screen.pixels[1][0]);
	EXPECT_FALSE(chip8.screen.pixels[1][1]);
	EXPECT_FALSE(chip8.screen.pixels[1][2]);
	EXPECT_TRUE(chip8.screen.pixels[1][3]);

	EXPECT_TRUE(chip8.screen.pixels[2][0]);
	EXPECT_FALSE(chip8.screen.pixels[2][1]);
	EXPECT_FALSE(chip8.screen.pixels[2][2]);
	EXPECT_TRUE(chip8.screen.pixels[2][3]);

	EXPECT_TRUE(chip8.screen.pixels[3][0]);
	EXPECT_FALSE(chip8.screen.pixels[3][1]);
	EXPECT_FALSE(chip8.screen.pixels[3][2]);
	EXPECT_TRUE(chip8.screen.pixels[3][3]);

	EXPECT_TRUE(chip8.screen.pixels[4][0]);
	EXPECT_TRUE(chip8.screen.pixels[4][1]);
	EXPECT_TRUE(chip8.screen.pixels[4][2]);
	EXPECT_TRUE(chip8.screen.pixels[4][3]);
}

// Ex9E - SKP Vx
// Skip next instruction if key with the value of Vx is pressed.
TEST(Instructions, SKP_Vx) {
	chip8 chip8{};
	chip8_init(&chip8);
	chip8_keyboard_set_map(&chip8.keyboard, keyboard_map);

	chip8.registers.PC = 0x10;
	chip8.registers.V[0x00] = 0x01;

	chip8_keyboard_down(&chip8.keyboard, 0x01);
	chip8_exec(&chip8, 0xE09E);
	EXPECT_EQ(chip8.registers.PC, (unsigned short)0x12);

	chip8_keyboard_up(&chip8.keyboard, 0x01);
	chip8_exec(&chip8, 0xE09E);
	EXPECT_EQ(chip8.registers.PC, (unsigned short)0x12);
}

// ExA1 - SKNP Vx
// Skip next instruction if key with the value of Vx is not pressed.
TEST(Instructions, SKNP_Vx) {
	chip8 chip8{};
	chip8_init(&chip8);
	chip8_keyboard_set_map(&chip8.keyboard, keyboard_map);

	chip8.registers.PC = 0x10;
	chip8.registers.V[0x00] = 0x01;

	chip8_keyboard_down(&chip8.keyboard, 0x01);
	chip8_exec(&chip8, 0xE0A1);
	EXPECT_EQ(chip8.registers.PC, (unsigned short)0x10);

	chip8_keyboard_up(&chip8.keyboard, 0x01);
	chip8_exec(&chip8, 0xE0A1);
	EXPECT_EQ(chip8.registers.PC, (unsigned short)0x12);
}


// Fx07 - LD Vx, DT
// Set Vx = delay timer value.
TEST(Instructions, LD_Vx_DT) {
	chip8 chip8{};
	chip8_init(&chip8);

	chip8.registers.delay_timer = 0x04;

	chip8_exec(&chip8, 0xF007);
	EXPECT_EQ(chip8.registers.V[0], 0x04);
}

// Fx0A - LD Vx, K
// Wait for a key press, store the value of the key in Vx.
//! I don't this there's a way to unit test this one



// Fx15 - LD DT, Vx
// Set delay timer = Vx.
TEST(Instructions, LD_DT_Vx) {
	chip8 chip8{};
	chip8_init(&chip8);

	chip8.registers.V[0x00] = 0x04;

	chip8_exec(&chip8, 0xF015);
	EXPECT_EQ(chip8.registers.delay_timer, 0x04);
}

// Fx18 - LD ST, Vx
// Set sound timer = Vx.
TEST(Instructions, LD_ST_Vx) {
	chip8 chip8{};
	chip8_init(&chip8);

	chip8.registers.V[0x00] = 0x04;

	chip8_exec(&chip8, 0xF018);
	EXPECT_EQ(chip8.registers.sound_timer, 0x04);
}

// Fx1E - ADD I, Vx
// Set I = I + Vx.
TEST(Instructions, ADD_I_Vx) {
	chip8 chip8{};
	chip8_init(&chip8);

	chip8.registers.I = 0x10;
	chip8.registers.V[0x00] = 0x04;

	chip8_exec(&chip8, 0xF01E);
	EXPECT_EQ(chip8.registers.I, 0x14);
}

// Fx29 - LD F, Vx
// Set I = location of sprite for digit Vx.
// The value of I is set to the location for the hexadecimal sprite corresponding to the value of Vx.
TEST(Instructions, LD_F_Vx) {
	chip8 chip8{};
	chip8_init(&chip8);

	chip8.registers.I = 0x10;
	chip8.registers.V[0x00] = 0x04;

	chip8_exec(&chip8, 0xF029);
	EXPECT_EQ(chip8.registers.I, 0x04 * CHIP8_DEFAULT_SPRITE_HEIGHT);
}

// Fx33 - LD B, Vx
// Store BCD representation of Vx in memory locations I, I+1, and I+2.
// The interpreter takes the decimal value of Vx, and places the hundreds digit in memory at
// location in I, the tens digit at location I+1, and the ones digit at location I+2.
TEST(Instructions, LD_B_Vx) {
	chip8 chip8{};
	chip8_init(&chip8);

	chip8.registers.I = 0x200;
	chip8.registers.V[0x00] = 134;

	chip8_exec(&chip8, 0xF033);
	EXPECT_EQ(chip8.memory.memory[chip8.registers.I] , 1);
	EXPECT_EQ(chip8.memory.memory[chip8.registers.I + 1], 3);
	EXPECT_EQ(chip8.memory.memory[chip8.registers.I + 2], 4);
}

// Fx55 - LD [I], Vx
// Store registers V0 through Vx in memory starting at location I.
// The interpreter copies the values of registers V0 through Vx into memory, starting at the address in I.
TEST(Instructions, LD__I__Vx) {
	chip8 chip8{};
	chip8_init(&chip8);

	chip8.registers.I = 0x200;
	chip8.registers.V[0x00] = 0x00;
	chip8.registers.V[0x01] = 0x01;
	chip8.registers.V[0x02] = 0x02;
	chip8.registers.V[0x03] = 0x03;
	chip8.registers.V[0x04] = 0x04;
	chip8.registers.V[0x05] = 0x05;

	chip8_exec(&chip8, 0xF555);

	EXPECT_EQ(chip8.memory.memory[chip8.registers.I], 0x00);
	EXPECT_EQ(chip8.memory.memory[chip8.registers.I + 1], 0x01);
	EXPECT_EQ(chip8.memory.memory[chip8.registers.I + 2], 0x02);
	EXPECT_EQ(chip8.memory.memory[chip8.registers.I + 3], 0x03);
	EXPECT_EQ(chip8.memory.memory[chip8.registers.I + 4], 0x04);
	EXPECT_EQ(chip8.memory.memory[chip8.registers.I + 5], 0x05);
}

// Fx65 - LD Vx, [I]
// Read registers V0 through Vx from memory starting at location I.
// The interpreter reads values from memory starting at location I into registers V0 through Vx.
TEST(Instructions, LD_Vx__I) {
	chip8 chip8{};
	chip8_init(&chip8);

	chip8.registers.I = 0x200;

	chip8.memory.memory[chip8.registers.I] = 0x00;
	chip8.memory.memory[chip8.registers.I + 1] = 0x01;
	chip8.memory.memory[chip8.registers.I + 2] = 0x02;
	chip8.memory.memory[chip8.registers.I + 3] = 0x03;
	chip8.memory.memory[chip8.registers.I + 4] = 0x04;
	chip8.memory.memory[chip8.registers.I + 5] = 0x05;

	chip8_exec(&chip8, 0xF565);
	EXPECT_EQ(chip8.registers.V[0], 0x00);
	EXPECT_EQ(chip8.registers.V[1], 0x01);
	EXPECT_EQ(chip8.registers.V[2], 0x02);
	EXPECT_EQ(chip8.registers.V[3], 0x03);
	EXPECT_EQ(chip8.registers.V[4], 0x04);
	EXPECT_EQ(chip8.registers.V[5], 0x05);
}