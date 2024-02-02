#include "AL/alc.h"
#include "AL/al.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { MaxWidth = 80 };

static void printList(const char *list, char separator)
{
    size_t col = MaxWidth, len;
    const char *indent = "    ";
    const char *next;

    if(!list || *list == '\0')
    {
        fprintf(stdout, "\n%s!!! none !!!\n", indent);
        return;
    }

    do {
        next = strchr(list, separator);
        if(next)
        {
            len = (size_t)(next-list);
            do {
                next++;
            } while(*next == separator);
        }
        else
            len = strlen(list);

        if(len + col + 2 >= MaxWidth)
        {
            fprintf(stdout, "\n%s", indent);
            col = strlen(indent);
        }
        else
        {
            fputc(' ', stdout);
            col++;
        }

        len = fwrite(list, 1, len, stdout);
        col += len;

        if(!next || *next == '\0')
            break;
        fputc(',', stdout);
        col++;

        list = next;
    } while(1);
    fputc('\n', stdout);
}

static ALenum checkALErrors(int linenum)
{
    ALenum err = alGetError();
    if(err != AL_NO_ERROR)
        printf("OpenAL Error: %s (0x%x), @ %d\n", alGetString(err), err, linenum);
    return err;
}
#define checkALErrors() checkALErrors(__LINE__)

static ALCenum checkALCErrors(ALCdevice *device, int linenum)
{
    ALCenum err = alcGetError(device);
    if(err != ALC_NO_ERROR)
        printf("ALC Error: %s (0x%x), @ %d\n", alcGetString(device, err), err, linenum);
    return err;
}
#define checkALCErrors(x) checkALCErrors((x),__LINE__)

static void printALCInfo(ALCdevice *device)
{
    ALCint major, minor;

    if(device)
    {
        const ALCchar *devname = NULL;
        printf("\n");
        if(alcIsExtensionPresent(device, "ALC_ENUMERATE_ALL_EXT") != AL_FALSE)
            devname = alcGetString(device, ALC_ALL_DEVICES_SPECIFIER);
        if(checkALCErrors(device) != ALC_NO_ERROR || !devname)
            devname = alcGetString(device, ALC_DEVICE_SPECIFIER);
        printf("** Info for device \"%s\" **\n", devname);
    }
    alcGetIntegerv(device, ALC_MAJOR_VERSION, 1, &major);
    alcGetIntegerv(device, ALC_MINOR_VERSION, 1, &minor);
    if(checkALCErrors(device) == ALC_NO_ERROR)
        printf("ALC version: %d.%d\n", major, minor);
    if(device)
    {
        printf("ALC extensions:");
        printList(alcGetString(device, ALC_EXTENSIONS), ' ');
        checkALCErrors(device);
    }
}

int main() {
    auto device = alcOpenDevice(nullptr);
    if (!device) {
        printf("!device");
        return 1;
    }
	printALCInfo(device);
    return 0;
}
