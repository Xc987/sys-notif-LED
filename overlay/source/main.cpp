#define TESLA_INIT_IMPL
#include <tesla.hpp>
#include <string>
#include <fstream>

class GuiTest : public tsl::Gui {
public:
    GuiTest() { }

    virtual tsl::elm::Element* createUI() override {
        auto frame = new tsl::elm::OverlayFrame("sys-notif-LED", "v1.0.0");
        auto list = new tsl::elm::List();

        auto solidItem = new tsl::elm::ListItem("Set LED: Solid");
        solidItem->setClickListener([](u64 keys) {
            if (keys & HidNpadButton_A) {
                const std::string resetPath = "sdmc:/config/sys-notif-LED/reset";
                const std::string typePath  = "sdmc:/config/sys-notif-LED/type";
                fsdevMountSdmc();
                std::ofstream resetFile(resetPath);
                std::ofstream typeFile(typePath);
                typeFile << "solid";
                return true;
            }
            return false;
        });
        list->addItem(solidItem);

        auto dimItem = new tsl::elm::ListItem("Set LED to Dim");
        dimItem->setClickListener([](u64 keys) {
            if (keys & HidNpadButton_A) {
                const std::string resetPath = "sdmc:/config/sys-notif-LED/reset";
                const std::string typePath  = "sdmc:/config/sys-notif-LED/type";
                fsdevMountSdmc();
                std::ofstream resetFile(resetPath);
                std::ofstream typeFile(typePath);
                typeFile << "dim";
                return true;
            }
            return false;
        });
        list->addItem(dimItem);
        
        auto fadeItem = new tsl::elm::ListItem("Set LED to Fade");
        fadeItem->setClickListener([](u64 keys) {
            if (keys & HidNpadButton_A) {
                const std::string resetPath = "sdmc:/config/sys-notif-LED/reset";
                const std::string typePath  = "sdmc:/config/sys-notif-LED/type";
                fsdevMountSdmc();
                std::ofstream resetFile(resetPath);
                std::ofstream typeFile(typePath);
                typeFile << "fade";
                return true;
            }
            return false;
        });
        list->addItem(fadeItem);

        auto offItem = new tsl::elm::ListItem("Set LED to Off");
        offItem->setClickListener([](u64 keys) {
            if (keys & HidNpadButton_A) {
                const std::string resetPath = "sdmc:/config/sys-notif-LED/reset";
                const std::string typePath  = "sdmc:/config/sys-notif-LED/type";
                fsdevMountSdmc();
                std::ofstream resetFile(resetPath);
                std::ofstream typeFile(typePath);
                typeFile << "off";
                return true;
            }
            return false;
        });
        list->addItem(offItem);

        auto chargeitem = new tsl::elm::ListItem("*Dim when charging");
        chargeitem->setClickListener([](u64 keys) {
            if (keys & HidNpadButton_A) {
                const std::string resetPath = "sdmc:/config/sys-notif-LED/reset";
                const std::string typePath  = "sdmc:/config/sys-notif-LED/type";
                fsdevMountSdmc();
                std::ofstream resetFile(resetPath);
                std::ofstream typeFile(typePath);
                typeFile << "charge";
                return true;
            }
            return false;
        });
        list->addItem(chargeitem);

        frame->setContent(list);
        return frame;
    }

    virtual void update() override { }

    virtual bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos,
                             HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) override {
        return false;
    }
};

class OverlayTest : public tsl::Overlay {
public:
    virtual void initServices() override {
        fsdevMountSdmc();
    }

    virtual void exitServices() override {
        fsdevUnmountDevice("sdmc");
    }

    virtual void onShow() override {}
    virtual void onHide() override {}

    virtual std::unique_ptr<tsl::Gui> loadInitialGui() override {
        return initially<GuiTest>();
    }
};

int main(int argc, char **argv) {
    return tsl::loop<OverlayTest>(argc, argv);
}
