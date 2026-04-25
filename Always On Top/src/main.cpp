#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/ui/BasedButtonSprite.hpp>
#include <Geode/ui/Notification.hpp>
#include <Geode/ui/Popup.hpp>

using namespace geode::prelude;

#ifdef GEODE_IS_WINDOWS
    #define WIN32_LEAN_AND_MEAN
    #include <Windows.h>
#endif

static bool g_enabled = false;
static bool g_didInit = false;

#ifdef GEODE_IS_WINDOWS
static HWND g_hwnd = nullptr;

static HWND findGD() {
    if (g_hwnd && IsWindow(g_hwnd)) return g_hwnd;
    g_hwnd = FindWindowA(nullptr, "Geometry Dash");
    return g_hwnd;
}
#endif

static void applyTopmost(bool on) {
#ifdef GEODE_IS_WINDOWS
    auto hwnd = findGD();
    if (!hwnd) {
        log::warn("couldn't find GD window");
        return;
    }
    SetWindowPos(hwnd, on ? HWND_TOPMOST : HWND_NOTOPMOST,
        0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
#else
    (void)on;
#endif
}

static CCNode* makeIcon(bool on) {
    auto color = on ? CircleBaseColor::Green : CircleBaseColor::Pink;

    auto spr = CircleButtonSprite::createWithSprite(
        "pin-icon.png"_spr, 0.9f, color, CircleBaseSize::Medium
    );
    if (!spr) {
        spr = CircleButtonSprite::createWithSpriteFrameName(
            "GJ_completesIcon_001.png", 1.0f, color, CircleBaseSize::Medium
        );
    }
    return spr;
}

static void updateButton() {
    auto scene = CCDirector::sharedDirector()->getRunningScene();
    if (!scene) return;

    auto btn = typeinfo_cast<CCMenuItemSpriteExtra*>(
        scene->getChildByIDRecursive("mifu.always-on-top/always-on-top-button")
    );
    if (!btn) return;

    auto icon = makeIcon(g_enabled);
    if (!icon) return;

    btn->setNormalImage(icon);
    btn->setContentSize(icon->getContentSize());
}

static void setTopmost(bool on) {
    g_enabled = on;
    applyTopmost(on);
    Mod::get()->setSavedValue("last-enabled", on);
    updateButton();

    if (Mod::get()->getSettingValue<bool>("show-notifications")) {
        Notification::create(
            on ? "Always On Top: ON" : "Always On Top: OFF",
            on ? NotificationIcon::Success : NotificationIcon::Info,
            1.2f
        )->show();
    }
}

static void showInfo() {
    std::string body =
        "Keep the <cy>Geometry Dash</c> window above every other "
        "window on your desktop.\n\n"
        "<cj>Useful for:</c>\n"
        "- Staying focused during a run\n"
        "- Not being interrupted by popups, alerts or system notifications\n"
        "- Watching a tutorial in a small window while you practice\n"
        "- Keeping GD visible on a second screen\n\n";
    body += g_enabled ? "Status: <cg>ENABLED</c>" : "Status: <cr>DISABLED</c>";

    geode::createQuickPopup(
        "Always On Top",
        body,
        "Cancel",
        g_enabled ? "Disable" : "Enable",
        [](auto, bool ok) {
            if (ok) setTopmost(!g_enabled);
        }
    );
}

class $modify(AOTMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;

        if (!g_didInit) {
            g_didInit = true;
            auto mod = Mod::get();
            bool startup = mod->getSettingValue<bool>("enabled-at-startup");
            bool remember = mod->getSettingValue<bool>("remember-state");
            bool saved = mod->getSavedValue<bool>("last-enabled", false);

            if (startup || (remember && saved)) {
                g_enabled = true;
                applyTopmost(true);
            }
        }

        auto menu = static_cast<CCMenu*>(this->getChildByID("bottom-menu"));
        if (!menu) return true;

        auto icon = makeIcon(g_enabled);
        if (!icon) return true;

        auto btn = CCMenuItemSpriteExtra::create(
            icon, this, menu_selector(AOTMenuLayer::onButton)
        );
        btn->setID("always-on-top-button"_spr);
        menu->addChild(btn);
        menu->updateLayout();

        return true;
    }

    void onButton(CCObject*) {
        auto mod = Mod::get();
        if (!mod->getSavedValue<bool>("seen-info", false)) {
            mod->setSavedValue("seen-info", true);
            showInfo();
            return;
        }

        // TODO: add a keybind to toggle this without going back to the main menu
        // would need geode.custom-keybinds as a dependency
        setTopmost(!g_enabled);
    }
};