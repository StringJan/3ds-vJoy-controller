#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>
#include <stdarg.h>
#include <unistd.h>

#include <fcntl.h>

#include <sys/types.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <3ds.h>

#define SOC_ALIGN       0x1000
#define SOC_BUFFERSIZE  0x100000

static u32 *SOC_buffer = NULL;
s32 sock = -1, csock = -1;
static char address[60];

__attribute__((format(printf,1,2)))
void failExit(const char *fmt, ...);

char keysNames[32][32] = {
		"KEY_A", "KEY_B", "KEY_SELECT", "KEY_START",
		"KEY_DRIGHT", "KEY_DLEFT", "KEY_DUP", "KEY_DDOWN",
		"KEY_R", "KEY_L", "KEY_X", "KEY_Y",
		"", "", "KEY_ZL", "KEY_ZR",
		"", "", "", "",
		"KEY_TOUCH", "", "", "",
		"KEY_CSTICK_RIGHT", "KEY_CSTICK_LEFT", "KEY_CSTICK_UP", "KEY_CSTICK_DOWN",
		"KEY_CPAD_RIGHT", "KEY_CPAD_LEFT", "KEY_CPAD_UP", "KEY_CPAD_DOWN"
	};


//---------------------------------------------------------------------------------
void socShutdown() {
//---------------------------------------------------------------------------------
	printf("waiting for socExit...\n");
	socExit();

}

void getAddressText(SwkbdState swkbd) {
	swkbdInit(&swkbd, SWKBD_TYPE_WESTERN, 2, -1);
			swkbdSetValidation(&swkbd, SWKBD_NOTEMPTY_NOTBLANK, 0, 0);
			swkbdSetFeatures(&swkbd, SWKBD_DARKEN_TOP_SCREEN | SWKBD_ALLOW_HOME | SWKBD_ALLOW_RESET | SWKBD_ALLOW_POWER);
			swkbdSetHintText(&swkbd, "Enter the server IP");
			// swkbdSetFilterCallback(&swkbd, MyCallback, NULL);
			SwkbdButton button = SWKBD_BUTTON_NONE;
			bool shouldQuit = false;
			do
			{
				swkbdSetInitialText(&swkbd, "");
				button = swkbdInputText(&swkbd, address, sizeof(address));
				if (button != SWKBD_BUTTON_NONE)
					break;

				SwkbdResult res = swkbdGetResult(&swkbd);
				if (res == SWKBD_RESETPRESSED)
				{
					shouldQuit = true;
					aptSetChainloaderToSelf();
					break;
				}
				else if (res != SWKBD_HOMEPRESSED && res != SWKBD_POWERPRESSED) {
					failExit("Error on input\n");
				}

				shouldQuit = !aptMainLoop();
			} while (!shouldQuit);
}

int map_range(int value, int from_min, int from_max, int to_min, int to_max) {
	return (value - from_min) * (to_max - to_min) / (from_max - from_min) + to_min;
}

void buttonsToString(u32 keys, circlePosition pos, char* buttons) {
	int keysFormatted = 0;
	if (keys & KEY_A) keysFormatted = (1<<0) | keysFormatted;
	if (keys & KEY_B) keysFormatted = (1<<1) | keysFormatted;
	if (keys & KEY_X) keysFormatted = (1<<2) | keysFormatted;
	if (keys & KEY_Y) keysFormatted = (1<<3) | keysFormatted;
	int scaledX = map_range(pos.dx, -160, 160, 0, 32768);
	int scaledY = map_range(pos.dy, -160, 160, 0, 32768);
	sprintf(buttons, "<%d; %d; %d>", keysFormatted, scaledX, scaledY);
}

//---------------------------------------------------------------------------------
int main(int argc, char **argv) {
//---------------------------------------------------------------------------------
	int ret;
;
	struct sockaddr_in client;
	char temp[1026];

	char buttons[32];

	static SwkbdState swkbd;

	gfxInitDefault();

	// register gfxExit to be run when app quits
	// this can help simplify error handling
	atexit(gfxExit);

	consoleInit(GFX_TOP, NULL);

	printf ("\nlibctru sockets demo\n");

	// allocate buffer for SOC service
	SOC_buffer = (u32*)memalign(SOC_ALIGN, SOC_BUFFERSIZE);

	if(SOC_buffer == NULL) {
		failExit("memalign: failed to allocate\n");
	}

	// Now intialise soc:u service
	if ((ret = socInit(SOC_buffer, SOC_BUFFERSIZE)) != 0) {
    	failExit("socInit: 0x%08X\n", (unsigned int)ret);
	}
	printf("Initialized socket service\n");

	// register socShutdown to run at exit
	// atexit functions execute in reverse order so this runs before gfxExit
	atexit(socShutdown);

	// libctru provides BSD sockets so most code from here is standard

	sock = socket (AF_INET, SOCK_STREAM, 0);

	if (sock < 0) {
		failExit("socket: %d %s\n", errno, strerror(errno));
	}
	printf("Created socket\n");

	memset (&client, 0, sizeof (client));

	client.sin_family = AF_INET;
	client.sin_port = htons (9999);

	getAddressText(swkbd);
	printf("Address: %s\n", address);
    if (inet_pton(AF_INET, address, &client.sin_addr)
        <= 0) {
        failExit("\nInvalid address/ Address not supported\n");
    }

	// Set socket non blocking so we can still read input to exit
	fcntl(sock, F_SETFL, fcntl(sock, F_GETFL, 0) | O_NONBLOCK);
	int loopCounter = 0;
	bool welcomed = false;
	// set client socket to blocking to simplify sending data back
	fcntl(sock, F_SETFL, fcntl(sock, F_GETFL, 0) & ~O_NONBLOCK);
	printf("Connecting to server\n");
	int connRet = connect(sock, (struct sockaddr*)&client, sizeof(client));
	// Connect to server
	if (connRet < 0 ) {
		perror("Connect:");
		printf("Loop Counter: %d\n", loopCounter);
		printf("Failed to connect\n");
		printf(osStrError(connRet));
		failExit("Connect %d", connRet);
	}
	printf("Connected!\n");
	while (aptMainLoop()) {
		// set client socket to blocking to simplify sending data back
		fcntl(sock, F_SETFL, fcntl(sock, F_GETFL, 0) & ~O_NONBLOCK);
		gspWaitForVBlank();
		hidScanInput();
		circlePosition pos;
		hidCircleRead(&pos);
		u32 keys = hidKeysDown() | hidKeysHeld();
		if (keys & KEY_START) break;
		printf("%ld\n", keys);
		buttonsToString(keys, pos, buttons);
		// set client socket to blocking to simplify sending data back
		fcntl(sock, F_SETFL, fcntl(sock, F_GETFL, 0) & ~O_NONBLOCK);
		//memset (temp, 0, 1026);

		if (!welcomed) {
			printf("Waiting for welcome\n");
			ret = recv (sock, temp, 20, 0);

			char *result = strstr(temp, "Welcome");
			if (result != NULL) {
				printf("Welcomed by Server\n");
				welcomed = true;
			}
			if (result == NULL) {
				failExit("Wrong welcome message on loop %d: %s", loopCounter, temp);
			}
		} else {
			svcSleepThread(1000000000);
			send(sock, buttons, strlen(buttons), 0);
			printf("Sent data\n");
		}

		loopCounter++;
	}

	close(sock);

	return 0;
}

//---------------------------------------------------------------------------------
void failExit(const char *fmt, ...) {
//---------------------------------------------------------------------------------

	if(sock>0) close(sock);
	if(csock>0) close(csock);

	va_list ap;

	printf(CONSOLE_RED);
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
	printf(CONSOLE_RESET);
	printf("\nPress B to exit\n");

	while (aptMainLoop()) {
		gspWaitForVBlank();
		hidScanInput();

		u32 kDown = hidKeysDown();
		if (kDown & KEY_B) exit(0);
	}
}
