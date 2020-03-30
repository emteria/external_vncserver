#include <binder/IServiceManager.h>
#include <binder/MemoryBase.h>
#include <binder/Parcel.h>
#include <utils/String16.h>

#include "common.h"
#include "log.h"

#include "clipboard.h"

using namespace android;

enum {
    SET_PRIMARY_CLIP = IBinder::FIRST_CALL_TRANSACTION
};

void writeToken(Parcel& data)
{
    data.writeInterfaceToken(String16("android.content.IClipboard")); // interface name
    data.writeInt32(1); // clip count
}

void writeDescription(Parcel& data)
{
    data.writeInt32(1); // label kind
    data.writeString16(String16("vnc")); // label text
    data.writeInt32(0); // array of mime types
    data.writeInt32(-1); // extras bundle
}

void writeIcon(Parcel& data)
{
    data.writeInt32(0); // no icon
}

void writeContent(Parcel& data, int len, char* str)
{
    data.writeInt32(1); // item count
    data.writeInt32(1); // content kind
    data.writeString16(String16(str, len)); // content text
}

void setClipboard(int len, char* str)
{
    sp<IServiceManager> sm = defaultServiceManager();
    sp<IBinder> binder = sm->checkService(String16("clipboard"));
    if (binder == NULL)
    {
        L("ERR: no clipboard service found\n");
        return;
    }

    Parcel data, reply;
    writeToken(data);
    writeDescription(data);
    writeIcon(data);
    writeContent(data, len, str);

    status_t result = binder->transact(SET_PRIMARY_CLIP, data, &reply, IBinder::FLAG_ONEWAY);
    if (result != NO_ERROR)
    {
        L("ERR: clipboard transaction failed\n");
        return;
    }
}
