#include <stdio.h>
#include <Windows.h>
#include "chip8.h"

// //SDL screen constants
//
// const int SCREEN_HEIGHT = 32;
// const int SCREEN_WIDTH = 64;
// const int SCREEN_MULTIPLIER = 1;
//
// uint8_t screenBuffer[SCREEN_HEIGHT * SCREEN_WIDTH * 4] = {0};
// bool bufferDebug = true;

/**********************
/Trying to use Javidx9's console rendering techniques...
/**********************/

int nScreenWidth = 64;
int nScreenHeight = 32;

int pitch = 0;
void* pixels = NULL;

emu chip8;

// bool init()
// {
// 	if(SDL_Init(SDL_INIT_VIDEO) < 0)
// 	{
// 		printf("Could not initialize SDL2! Error: %s\n", SDL_GetError());
// 		return false;
// 	}
// }

// void close(SDL_Window* window, SDL_Texture* texture, SDL_Renderer* renderer)
// {
// 	SDL_DestroyTexture(texture);
// 	SDL_DestroyRenderer(renderer);
// 	SDL_DestroyWindow(window);
// 	SDL_Quit();
// }
//
// void copyToBuffer()
// {
// 	for(int i = 0; i < 2048; i++)
// 	{
// 		if(chip8.graphics[i] == 1)
// 		{
// 			screenBuffer[(i*4)] = 255;
// 			screenBuffer[(i*4)+1] = 255;
// 			screenBuffer[(i*4)+2] = 255;
// 			screenBuffer[(i*4)+3] = 255;
// 		} else {
// 			screenBuffer[(i*4)] = 0;
// 			screenBuffer[(i*4)+1] = 0;
// 			screenBuffer[(i*4)+2] = 0;
// 			screenBuffer[(i*4)+3] = 255;
// 		}
// 	}
// }

// bool updateTexture(SDL_Texture* texture)
// {
// 	if(SDL_LockTexture(texture, NULL, &pixels, &pitch) != 0)
// 	{
// 		printf("Couldn't lock the texture! Error: %s\n", SDL_GetError());
// 		return false;
// 	} else
// 	{
// 		memcpy(pixels, screenBuffer, pitch * SCREEN_HEIGHT);
// 	}
//
// 	SDL_UnlockTexture(texture);
// 	return true;
// }
//
// void display(SDL_Renderer* renderer, SDL_Texture* texture)
// {
// 	SDL_RenderClear(renderer);
// 	SDL_RenderCopy(renderer, texture, NULL, NULL);
// 	SDL_RenderPresent(renderer);
// }

int wmain(int argc, wchar_t *argv[], wchar_t **envp)
{
	if(argc < 2)
	{
		printf("Usage: emu ROMPATH\n");
		return 1;
	}

	if(!chip8.loadRom(argv[1]))
	{
		printf("Ya failed.\n");
		return 1;
	}

	//Create console screen buffer

	wchar_t *screen = new wchar_t[nScreenWidth*nScreenHeight];
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

	// init();
	// SDL_Window* window = SDL_CreateWindow("Chip8 by Aerosol", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE);
	// 	if (window == NULL)
	// 	{
	// 		printf("Window got messed up. Error: %s\n", SDL_GetError());
	// 		return 1;
	// 	}
	// SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	// 	if(renderer == NULL)
	// 	{
	// 		printf("Couldn't create the renderer! Error: %s\n", SDL_GetError());
	// 		return 1;
	// 	}
	// SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
	// 	if(texture == NULL)
	// 	{
	// 		printf("Couldn't create texture! Error: %s\n", SDL_GetError());
	// 		return 1;
	// 	}


	// SDL_Event e;

	while(1)
	{
		short pixel = ' ';
		screen[nScreenWidth * nScreenHeight - 1] = '\0';
		WriteConsoleOutputCharacterW(hConsole, screen, nScreenWidth * nScreenHeight, {0, 0}, &dwBytesWritten);
		// while(SDL_PollEvent(&e) != 0)
		// {
		// 	if(e.type == SDL_QUIT)
		// 	{
		// 		quit = 1;
		// 	}
			/*chip8.graphics[0] = 1;
			copyToBuffer();
			if(!bufferDebug)
			{
				for(int i = 0; i < 32; i++)
				{
					printf("Location %d: %x\n", i, screenBuffer[i]);
				}
				bufferDebug = true;
			}
			updateTexture(texture);
			display(renderer, texture);*/


			chip8.emuCycle();
			chip8.debugRender();
			for(int x = 0; x < nScreenWidth; x++)
			{
				for(int y = 0; y < nScreenHeight; y++)
				{
					if(chip8.graphics[(y*64) + x] == 0)
						pixel = 0x2588 ;
					else
						pixel = ' ';
				}
			}
			// if(chip8.drawFlag)
			// {
			// 	copyToBuffer();
			// 	if(bufferDebug)
			// {
			// 	for(int i = 0; i < 32; i++)
			// 	{
			// 		printf("Location %d: %x\n", i, screenBuffer[i]);
			// 	}
			// 	bufferDebug = false;
			// }
			// 	updateTexture(texture);
			// 	display(renderer, texture);
			// 	chip8.drawFlag = false;
			// }
		// }
	}
	// close(window, texture, renderer);
	return 0;
}
