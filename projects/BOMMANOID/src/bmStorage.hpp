
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
    static constexpr int gIndentSize = 2;
    static constexpr int gMaxSize = 48;
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
            printSpaces(gMaxSize - indent * gIndentSize - strlen(entry.name()));
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

    static String GetUsbMassStorageFilename(size_t slot)
    {
        return String("/ClarinoidSettingsSlot") + slot + ".json";
    }

    Result SaveUsbMassStorage(size_t slot, ClarinoidJsonDocument &doc)
    {
        Serial.println("SaveUsbMassStorage");
        auto fileName = GetUsbMassStorageFilename(slot);
        Serial.println(String("SaveUsbMassStorage fileName: ") + fileName);
        File f = gUsbFileSystem1.open(fileName.c_str(), FILE_WRITE_BEGIN);
        Serial.println(String("file opened; size=") + (int)f.size());
        size_t ret = serializeJsonPretty(doc, f);
        Serial.println(String("SaveUsbMassStorage::serializeJsonPretty complete"));
        f.close();
        if (ret == 0)
        {
            Serial.println(String("SaveUsbMassStorage error"));
            return Result::Failure(String("Error saving to slot ") + slot);
        }
            Serial.println(String("SaveUsbMassStorage success"));
        return Result::Success(String(ret) + " bytes");
    }

    // returns 0 if error.
    virtual Result SaveDocument(StorageChannel ch, ClarinoidJsonDocument &doc) override
    {
        switch (ch)
        {
        case StorageChannel::UsbMassStorage0:
            return SaveUsbMassStorage(0, doc);
        case StorageChannel::UsbMassStorage1:
            return SaveUsbMassStorage(1, doc);
        case StorageChannel::UsbMassStorage2:
            return SaveUsbMassStorage(2, doc);
        case StorageChannel::UsbMassStorage3:
            return SaveUsbMassStorage(3, doc);
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

    DeserializationError LoadUsbMassStorage(size_t slot, ClarinoidJsonDocument &doc)
    {
        auto fileName = GetUsbMassStorageFilename(slot);
        Serial.println(String("LoadUsbMassStorage filename: ") + fileName);
        File f = gUsbFileSystem1.open(fileName.c_str(), FILE_READ);
        Serial.println(String("file size: ") + (int)f.size());
        auto ret = deserializeJson(doc, f);
        Serial.println(String("deserializeJson: ") + ret.c_str() + ", code " + (int)ret.code());
        f.close();
        return ret;
    }

    virtual DeserializationError LoadDocument(StorageChannel ch, ClarinoidJsonDocument &doc) override
    {
        switch (ch)
        {
        case StorageChannel::UsbMassStorage0:
            return LoadUsbMassStorage(0, doc);
        case StorageChannel::UsbMassStorage1:
            return LoadUsbMassStorage(1, doc);
        case StorageChannel::UsbMassStorage2:
            return LoadUsbMassStorage(2, doc);
        case StorageChannel::UsbMassStorage3:
            return LoadUsbMassStorage(3, doc);
        default:
        case StorageChannel::SerialStream:
            return deserializeJson(doc, Serial);
        case StorageChannel::USBMidiSysex:
            Serial.println("USBMidiSysex not supported yet");
            return DeserializationError{DeserializationError::Code::InvalidInput};
        case StorageChannel::TRSMidiSysex:
            Serial.println("TRSMidiSysex not supported yet");
            return DeserializationError{DeserializationError::Code::InvalidInput};
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
