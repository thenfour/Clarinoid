
#pragma once

namespace clarinoid
{

USBDrive gUsbDrive(gUsbHost);
USBFilesystem gUsbFileSystem1(gUsbHost);

static constexpr size_t aos8n8et9uhpch = sizeof(gUsbDrive);
static constexpr size_t aos8ne8t9uhpch = sizeof(gUsbFileSystem1);
static constexpr size_t aos8989ne8t9uhpch = sizeof(SynthModulationSpec);


void printDirectory(File dir, int indent = 0)
{
    static int gIndentSize = 2;
    static int gMaxSize = 48;
    auto printSpaces = [](int n) {
        for (int i = 0; i < n; ++i)
        {
            Serial.print(" ");
        }
    };

    while (true)
    {
        File entry = dir.openNextFile();
        if (!entry)
        {
            // Serial.println("** no more files **");
            break;
        }

        printSpaces(indent * gIndentSize);
        Serial.print(entry.name());
        if (entry.isDirectory())
        {
            Serial.println("/");
            printDirectory(entry, indent + 1);
        }
        else
        {
            // files have sizes, directories do not
            printSpaces(48 - indent * gIndentSize - strlen(entry.name()));
            Serial.print("  ");
            Serial.println(entry.size(), DEC);
        }
        entry.close();
    }
}


struct BommanoidStorage : IStorage
{
    BommanoidStorage()
    {
        gUsbHost.begin();
        REQUIRE(gUsbFileSystem1.begin(&gUsbDrive));
    }

    // returns 0 if error.
    virtual size_t SaveDocument(StorageChannel ch, ClarinoidJsonDocument &doc) override
    {
        switch (ch)
        {
        case StorageChannel::UsbMassStorage0:
            Serial.println("usb mass storage 0 not supported yet");
            return 0;
        case StorageChannel::UsbMassStorage1:
            Serial.println("usb mass storage 1 not supported yet");
            return 0;
        case StorageChannel::UsbMassStorage2:
            Serial.println("usb mass storage 2 not supported yet");
            return 0;
        case StorageChannel::UsbMassStorage3:
            Serial.println("usb mass storage 3 not supported yet");
            return 0;
        default:
        case StorageChannel::SerialStream:
            return serializeJsonPretty(doc, Serial);
        case StorageChannel::USBMidiSysex:
            Serial.println("USBMidiSysex not supported yet");
            return 0;
        case StorageChannel::TRSMidiSysex:
            Serial.println("TRSMidiSysex not supported yet");
            return 0;
        }
    }

    virtual void Dir(StorageChannel ch) override
    {
        File root = gUsbFileSystem1.open("/");
        printDirectory(root);
    }
};

// #define FILE_READ  0
// #define FILE_WRITE 1
// #define FILE_WRITE_BEGIN 2

// https://forum.pjrc.com/threads/64136-File-abstraction-and-SdFat-integration/page3
// {
//     Serial.println("Writing contents...");
//     File f = gUsbFileSystem1.open("/clarinoidSettings.mp", FILE_WRITE_BEGIN); // FILE_READ /
//     f.write("hello world");
//     f.close();
// }
// {
//     File f2 = gUsbFileSystem1.open("/clarinoidSettings.mp", FILE_READ); // FILE_READ /
//     String co = f2.readString();
//     f2.close();
//     Serial.println("read contents:");
//     Serial.println(co);
// }

} // namespace clarinoid
