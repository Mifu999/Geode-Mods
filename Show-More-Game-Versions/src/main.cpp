#include <unordered_map>
#include <vector>
#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>

using namespace geode::prelude;

struct labelPosition {
	float posX, posY;
	float anchorX, anchorY;
	CCTextAlignment align;
};

struct VersionDisplay {
	std::string name;
	std::string value;
	bool showVersionPrefix;
};

class $modify(MenuLayer) {
	bool init() {
		if (!MenuLayer::init()) return false;
		
		// Config
		bool confGDVersion = Mod::get()->getSettingValue<bool>("conf-showGDVersion");
		bool confGeodeVersion = Mod::get()->getSettingValue<bool>("conf-showGeodeVersion");
		bool confCocosVersion = Mod::get()->getSettingValue<bool>("conf-showCocos2dVersion");
		bool confOSVersion = Mod::get()->getSettingValue<bool>("conf-showOSVersion");
		bool confArchitecture = Mod::get()->getSettingValue<bool>("conf-showArchitecture");
		bool confModCount = Mod::get()->getSettingValue<bool>("conf-showModCount");
		bool confShowVersionsOnNewLine = Mod::get()->getSettingValue<bool>("conf-versionsOnNewLines");
		std::string confVersionsPosition = Mod::get()->getSettingValue<std::string>("conf-versionsPosition");
		double confVersionAlpha = Mod::get()->getSettingValue<double>("conf-versionsOpacity");
		float confVersionScale = Mod::get()->getSettingValue<double>("conf-versionsScale");
		double confCustomPosX = Mod::get()->getSettingValue<double>("conf-customPosX");
		double confCustomPosY = Mod::get()->getSettingValue<double>("conf-customPosY");
		bool confDebugEnabled = Mod::get()->getSettingValue<bool>("conf-debugLoggingEnabled");
		
		// Vars
		float menuHeight = getContentHeight() - 2;
		float menuWidth = getContentWidth() - 2;
		std::string splitCharacter = (confShowVersionsOnNewLine) ? "\n" : " | ";
		std::vector<VersionDisplay> versions;
		std::string labelString;
		
		// Get versions
		if (confGDVersion) {
			versions.push_back({"GD", Loader::get()->getGameVersion(), true});
		}
		
		if (confGeodeVersion) {
			versions.push_back({"Geode", Loader::get()->getVersion().toNonVString(), true});
		}
		
		if (confCocosVersion) {
			versions.push_back({"Cocos2d", cocos2d::cocos2dVersion(), true});
		}
		
		// OS Version with detailed info
		if (confOSVersion) {
			std::string osInfo = "Unknown";
			
			#ifdef GEODE_IS_WINDOWS
				#if defined(_WIN32_WINNT_WIN10) && _WIN32_WINNT >= _WIN32_WINNT_WIN10
					osInfo = "Windows 10/11";
				#elif defined(_WIN32_WINNT_WINBLUE)
					osInfo = "Windows 8.1";
				#elif defined(_WIN32_WINNT_WIN8)
					osInfo = "Windows 8";
				#elif defined(_WIN32_WINNT_WIN7)
					osInfo = "Windows 7";
				#else
					osInfo = "Windows";
				#endif
			#elif defined(GEODE_IS_MACOS)
				osInfo = "macOS";
			#elif defined(GEODE_IS_ANDROID)
				osInfo = "Android";
			#elif defined(GEODE_IS_IOS)
				osInfo = "iOS";
			#elif defined(GEODE_IS_LINUX)
				osInfo = "Linux";
			#endif
			
			versions.push_back({"OS", osInfo, false});
		}
		
		// Architecture
		if (confArchitecture) {
			std::string arch = "Unknown";
			
			#if defined(__x86_64__) || defined(_M_X64)
				arch = "x64";
			#elif defined(__i386__) || defined(_M_IX86)
				arch = "x86";
			#elif defined(__arm64__) || defined(__aarch64__)
				arch = "ARM64";
			#elif defined(__arm__) || defined(_M_ARM)
				arch = "ARM32";
			#endif
			
			versions.push_back({"Arch", arch, false});
		}
		
		// Mod Count with detailed stats
		if (confModCount) {
			auto allMods = Loader::get()->getAllMods();
			size_t totalMods = allMods.size();
			size_t enabledMods = 0;
			size_t disabledMods = 0;
			
			for (auto* mod : allMods) {
				if (mod->isEnabled()) {
					enabledMods++;
				} else {
					disabledMods++;
				}
			}
			
			std::string modStats = std::to_string(enabledMods) + " enabled, " + 
			                       std::to_string(disabledMods) + " disabled, " + 
			                       std::to_string(totalMods) + " total";
			
			versions.push_back({"Mods", modStats, false});
		}
		
		// Concatenate into single string with separators
		for (size_t i = 0; i < versions.size(); i++) {
			if (i > 0) {
				labelString += splitCharacter;
			}
			
			if (versions[i].showVersionPrefix) {
				labelString += versions[i].name + ": v" + versions[i].value;
			} else {
				labelString += versions[i].name + ": " + versions[i].value;
			}
		}
		
		// Map of label positions (including custom)
		std::unordered_map<std::string, labelPosition> layouts = {
			{"Top Left", {2, menuHeight, 0, 1, kCCTextAlignmentLeft}},
			{"Top Right", {menuWidth, menuHeight, 1, 1, kCCTextAlignmentRight}},
			{"Bottom Left", {2, 2, 0, 0, kCCTextAlignmentLeft}},
			{"Bottom Right", {menuWidth, 2, 1, 0, kCCTextAlignmentRight}},
			{"Custom", {static_cast<float>(confCustomPosX), static_cast<float>(confCustomPosY), 0, 0, kCCTextAlignmentLeft}}
		};
		
		// Make, position, align, style, then add the label
		auto labelItem = CCLabelBMFont::create(labelString.c_str(), "bigFont.fnt");
		auto labelOpts = layouts[confVersionsPosition];
		labelItem->setPosition(labelOpts.posX, labelOpts.posY);
		labelItem->setAnchorPoint({labelOpts.anchorX, labelOpts.anchorY});
		labelItem->setAlignment(labelOpts.align);
		labelItem->setScale(confVersionScale);
		labelItem->setOpacity(confVersionAlpha);
		labelItem->setID("client-versions-label");
		this->addChild(labelItem);
		
		// Print values to console
		if (confDebugEnabled) {
			for (const auto& ver : versions) {
				if (ver.showVersionPrefix) {
					log::info("{}: v{}", ver.name, ver.value);
				} else {
					log::info("{}: {}", ver.name, ver.value);
				}
			}
			log::info("Joined Versions String: {}", labelString);
			log::info("Position: {}", confVersionsPosition);
			if (confVersionsPosition == "Custom") {
				log::info("Custom Position: X={}, Y={}", confCustomPosX, confCustomPosY);
			}
			log::info("Scale: {}", confVersionScale);
		}
		
		return true;
	}
};