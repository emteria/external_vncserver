/*
droid vnc server - Android VNC server
Copyright (C) 2009 Jose Pereira <onaips@gmail.com>
Copyright (C) 2021 The emteria.OS project

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 3 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/

#include "common.h"
#include "flinger.h"
#include "clipboard.h"
#include "input.h"

extern "C" {
    #include "libvncserver/scale.h"
    #include "rfb/rfb.h"
    #include "rfb/rfbregion.h"
    #include "rfb/keysym.h"
}

//  port 5900 is bound natively in some Android devices
int port = 5901;
char* passwd = NULL;

rfbScreenInfoPtr vncscr;
unsigned int* vncbuf;

uint32_t idle = 0;
uint32_t standby = 1;
uint16_t scaling = 100;

const char* defaultPassFile = "/data/vnc/password.bin";

// reverse connection
char* rhost = NULL;
int rport = 0;

screenFormat screenformat;
void (*update_screen)(void) = NULL;

void setIdle(int i)
{
    idle = i;
}

void closeVncServer(int signo)
{
    L("Cleaning up vncd (signo %d)...\n", signo);

    closeDisplay();
    closeFlinger();
    cleanupInput();

    free(vncbuf);
    rfbScreenCleanup(vncscr);

    exit(0);
}

void clientGone(rfbClientPtr cl)
{
    L("Client disconnected from %s\n", cl->host);
}

void rotateScreen(android::ui::Rotation rotation)
{
    L("Performing screen rotation from %s to %s\n", toCString(screenformat.rotation), toCString(rotation));
    bool oldLandscape = screenformat.rotation == android::ui::ROTATION_0 || screenformat.rotation == android::ui::ROTATION_180;
    bool newLandscape = rotation == android::ui::ROTATION_0 || rotation == android::ui::ROTATION_180;
    if (oldLandscape != newLandscape)
    {
        // we need to restart the whole vncd to re-initialize display size
        L("Cannot handle dimention flip when rotating screen, restarting vncd service...\n");
        property_set("ctl.restart", "vncd");
    }
    else
    {
        // only change the rotation value in the settings
        L("Applying screen rotation without dimention flip\n");
        screenformat.rotation = rotation;
    }
}

enum rfbNewClientAction clientHook(rfbClientPtr cl)
{
    L("Client connected from %s\n", cl->host);
    cl->clientGoneHook = (ClientGoneHookPtr) clientGone;

    if (scaling != 100)
    {
        int w = screenformat.width * scaling / 100;
        int h = screenformat.height * scaling / 100;

        L("Scaling to w=%d, h=%d\n", w, h);
        rfbScalingSetup(cl, w, h);
    }

    return RFB_CLIENT_ACCEPT;
}

void setClipboardText(char* str, int len, struct _rfbClientRec* cl)
{
    L("Updating local clipboard with remote text\n");
    setClipboard(len, str);
}

void setTextChat(struct _rfbClientRec* cl, int len, char* str)
{
    L("Text chat: %s\n", str);
}

void initVncServer()
{
	vncscr = rfbGetScreen(NULL, NULL, screenformat.width, screenformat.height, 0, 3, screenformat.bitsPerPixel/CHAR_BIT);
	vncbuf = (unsigned int*) calloc(screenformat.width * screenformat.height, screenformat.bitsPerPixel/CHAR_BIT);

	assert(vncscr != NULL);
	assert(vncbuf != NULL);

	vncscr->desktopName = (char*) "emteria.OS";
	vncscr->frameBuffer = (char*) vncbuf;
	vncscr->port = port;
	vncscr->ipv6port = port;
	vncscr->authPasswdData = passwd;
	vncscr->newClientHook = (rfbNewClientHookPtr) clientHook;
	vncscr->kbdAddEvent = keyEvent;
	vncscr->ptrAddEvent = ptrEvent;
	vncscr->setXCutText = setClipboardText;
	vncscr->setTextChat = setTextChat;
	vncscr->permitFileTransfer = true;

	vncscr->serverFormat.redShift = screenformat.redShift;
	vncscr->serverFormat.greenShift = screenformat.greenShift;
	vncscr->serverFormat.blueShift = screenformat.blueShift;

	vncscr->serverFormat.redMax = (( 1 << screenformat.redMax) -1);
	vncscr->serverFormat.greenMax = (( 1 << screenformat.greenMax) -1);
	vncscr->serverFormat.blueMax = (( 1 << screenformat.blueMax) -1);

	vncscr->serverFormat.trueColour = TRUE;
	vncscr->serverFormat.bitsPerPixel = screenformat.bitsPerPixel;

	vncscr->alwaysShared = TRUE;
	vncscr->handleEventsEagerly = TRUE;
	vncscr->deferUpdateTime = 10;

	rfbInitServer(vncscr);
	rfbMarkRectAsModified(vncscr, 0, 0, screenformat.width, screenformat.height);
}

void extractReverseHostPort(char *str)
{
	int len = strlen(str);
	char *p;

	// copy in to host
	rhost = (char *) malloc(len+1);
	if (!rhost) {
		L("reverse_connect: could not malloc string %d\n", len);
		exit(-1);
	}

	strncpy(rhost, str, len);
	rhost[len] = '\0';

	// extract port, if any
	if ((p = strrchr(rhost, ':')) != NULL) {
		rport = atoi(p+1);
		*p = '\0';
	}
}

// inspired by rfbReverseConnection function
// however, we do not set the "reverse connection" flag
// this forces authorization flow if the password was set
bool createReverseConnection()
{
    int sock = rfbConnect(vncscr, rhost, rport);
    if (sock < 0)
        return false;

    rfbClientPtr cl = rfbNewClient(vncscr, sock);
    if (!cl)
        return false;

    return true;
}

void printUsage()
{
    L("\nvncd [parameters]\n"
        "-s <scale>\t- Scale percentage (20,30,50,100,150)\n"
        "-p <file>\t- Path to custom password file\n"
        "-P <port>\t- Custom port for the VNC server\n"
        "-R <host:port>\t- Host and port for reverse connection\n"
        "-h\t\t- Print this help\n"
        "-v\t\t- Output vncd version\n"
        "\n");
}

void parseArguments(int argc, char **argv)
{
    if (argc > 1)
    {
        int i=1;
        int r;
        while (i < argc)
        {
            if (*argv[i] == '-')
            {
		switch(*(argv[i] + 1))
		{
		case 'h':
			printUsage();
			exit(0);
			break;
		case 'p':
			i++;
			passwd = argv[i];
			break;
		case 'P':
			i++;
			port = atoi(argv[i]);
			break;
		case 's':
			i++;
			r = atoi(argv[i]);
			if (r >= 1 && r <= 150) { scaling = r; }
					else { scaling = 100; }
			L("Scaling to %d\n", scaling);
			break;
		case 'R':
			i++;
			extractReverseHostPort(argv[i]);
			break;
		case 'v':
			i++;
			L("emteria.OS VNC server v3.0\n");
			exit(0);
		}
            }
            i++;
        }
    }
}

int main(int argc, char **argv)
{
    signal(SIGINT, closeVncServer);
    signal(SIGKILL, closeVncServer);
    signal(SIGILL, closeVncServer);

    char args[PROPERTY_VALUE_MAX];
    memset(args, 0, PROPERTY_VALUE_MAX);
    property_get("persist.sys.vncd.args", args, "");

    // parse terminal arguments
    if (argc > 1)
    {
        L("Parsing %d commandline arguments\n", argc);
        parseArguments(argc, argv);
    }
    // parse property arguments
    else if (strlen(args) > 1)
    {
        int argsLim = 64;
        int argsCount = 1;

        char* argsVals[argsLim];
        char* p2 = strtok(args, " ");
        while (p2 && argsCount < argsLim - 1)
        {
            argsVals[argsCount++] = p2;
            p2 = strtok(0, " ");
        }
        argsVals[argsCount] = 0;

        L("Parsing %d property arguments\n", argsCount);
        parseArguments(argsCount, argsVals);
    }

    bool defaultPassExists = (access(defaultPassFile, F_OK) == 0);
    bool userPassSpecified = (passwd != NULL);
    bool userPassExists = userPassSpecified && (access(passwd, F_OK) == 0);
    if (defaultPassExists)
    {
        passwd = (char*) defaultPassFile;
        if (userPassSpecified) { L("Default password file takes precedence over user-specified password file\n"); }
    }
    else if (!userPassExists)
    {
        passwd = NULL;
        if (userPassSpecified) { L("User-specified password file is not readable\n"); }
    }

    initFlinger();
    int error = initDisplay();
    if (error != 0)
    {
        L("Failed initializing VNC display\n");
        closeVncServer(-1);
    }

    L("Initializing VNC server:\n");
    L(" - rotation: %s\n", toCString(screenformat.rotation));
    L(" - width: %d\n", screenformat.width);
    L(" - height: %d\n", screenformat.height);
    L(" - bpp: %d\n", screenformat.bitsPerPixel);
    L(" - rgba: %d:%d:%d:%d\n", screenformat.redShift, screenformat.greenShift, screenformat.blueShift, screenformat.alphaShift);
    L(" - length: %d:%d:%d:%d\n", screenformat.redMax, screenformat.greenMax, screenformat.blueMax, screenformat.alphaMax);
    L(" - password: %s\n", (passwd != NULL) ? "yes" : "no");
    L(" - scaling: %d\n", scaling);
    L(" - port: %d\n", port);

    initInput();
    initVncServer();

    bool startRemote = (rhost != NULL);
    if (startRemote) { createReverseConnection(); }

    while (true)
    {
        long usec = (vncscr->deferUpdateTime + standby) * 1000;
        rfbProcessEvents(vncscr, usec);

        if (idle) { standby += 5; }
             else { standby = 1; }

        if (vncscr->clientHead == NULL)
        {
            idle = 1;
            standby = 10;
            continue;
        }

        android::ui::Rotation rotation = getScreenRotation();
        if (screenformat.rotation != rotation) { rotateScreen(rotation); }

        for (rfbClientPtr client_ptr = vncscr->clientHead; client_ptr; client_ptr = client_ptr->next)
        {
            // update screen once if at least one client has requested
            if (!sraRgnEmpty(client_ptr->requestedRegion))
            {
                readBuffer(vncbuf);
                rfbMarkRectAsModified(vncscr, 0, 0, screenformat.width, screenformat.height);
                break;
            }
        }
    }

    L("Terminating...\n");
    closeVncServer(0);
}
