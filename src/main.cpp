#include <Geode/Geode.hpp>
#include <Geode/ui/GeodeUI.hpp>

using namespace geode::prelude;

#include <SimpleIni.h>

#define fileExists fileExistsInSearchPaths
#define SETTING(type, key_name) Mod::get()->getSettingValue<type>(key_name)

std::string escDEC(std::string str) {
    auto result = matjson::parse(str);
	if (auto err = result.err()) return str + "\n" + err.value().message;
    return result.unwrapOrDefault().asString().unwrapOr(str).c_str();
}
std::string escENC(std::string str) {
	return matjson::Value(str).dump().c_str();
}

namespace FileCache {
    struct FileResult {
        std::string content;
        std::string error;
        std::filesystem::file_time_type time;
        bool ok = false;
    };

    struct IniResult {
        CSimpleIni ini;
        SI_Error err = SI_OK;
        std::string errorStr;
        std::filesystem::file_time_type time;
        bool ok = false;
    };

    inline std::unordered_map<std::string, FileResult> fileCache;
    inline std::unordered_map<std::string, IniResult> iniCache;
    inline std::recursive_mutex mtx;

    inline std::filesystem::file_time_type getWriteTime(const std::string& path) {
        std::error_code ec;
        return std::filesystem::last_write_time(path, ec);
    }

    inline FileResult& getText(const std::string& path) {
        std::lock_guard lock(mtx);

        auto now = getWriteTime(path);
        auto it = fileCache.find(path);

        if (it != fileCache.end() && it->second.time == now) {
            return it->second;
        }

        auto& res = fileCache[path];

        res.time = now;
        res.ok = false;
        res.error.clear();
        res.content.clear();

        auto read = file::readString(path.c_str());
        if (read.err()) {
            res.error = read.err().value();
        }
        else {
            res.ok = true;
            res.content = read.unwrap();
        }

        return res;
    }

    inline IniResult& getIni(const std::string& path) {
        std::lock_guard lock(mtx);

        auto now = getWriteTime(path);
        auto it = iniCache.find(path);

        if (it != iniCache.end() && it->second.time == now) {
            return it->second;
        }

        auto& res = iniCache[path];

        res.time = now;
        res.ok = false;
        res.errorStr.clear();
        res.ini.Reset();

        auto& file = getText(path);

        if (!file.ok) {
            res.ok = false;
            res.errorStr = file.error;
        }
        else {
            res.err = res.ini.LoadData(file.content);

            if (res.err < 0) {
                if (res.err == SI_FAIL) res.errorStr = "Generic failure";
                else if (res.err == SI_NOMEM) res.errorStr = "Out of memory";
                else if (res.err == SI_FILE) res.errorStr = std::string("File error\n") + strerror(errno);
                else res.errorStr = "Unknown error";
            }
            else {
                res.ok = true;
            }
        }

        return res;
    }

    inline void invalidate(const std::string& path) {
        std::lock_guard lock(mtx);
        fileCache.erase(path);
        iniCache.erase(path);
    }
}

$execute{
    for (auto path : {
        string::pathToString(Mod::get()->getConfigDir()),
        string::pathToString(Mod::get()->getSaveDir()),
        string::pathToString(Mod::get()->getTempDir())
    }) CCFileUtils::get()->addPriorityPath(path.c_str());
};

#ifndef LEVELS_MODIFICATION  

#include <Geode/modify/LocalLevelManager.hpp>
class $modify(MLE_LocalLevelManager, LocalLevelManager) {
    gd::string getMainLevelString(int id) {
        //log::debug("{}({})", __FUNCTION__, id);

        auto filename = "levels/" + utils::numToString(id) + ".string.txt";
        CCFileUtils::get()->m_fullPathCache.erase(filename.c_str());
        if (!fileExists(filename.c_str())) {
            auto newfp = Mod::get()->getConfigDir() / filename;
            file::createDirectoryAll(newfp.parent_path()).err();
            if (auto err = file::writeString(
                newfp, LocalLevelManager::getMainLevelString(id).c_str()
            ).err()) log::error("{}.writeString: {}", __FUNCTION__, err);
        }

        auto path = CCFileUtils::get()->fullPathForFilename(filename.c_str(), !"why");
        auto& res = FileCache::getText(path);
        if (!res.ok) {
            log::error("{}.readString: {}", __FUNCTION__, res.error);
            //whyyy the fuuuuck i done that
            std::string errlevel = R"(kS38,1_110_2_110_3_112_6_1000_7_1_15_0_18_0_8_1|1_0_2_0_3_0_6_1001_7_1_15_0_18_0_8_1|1_0_2_102_3_255_11_255_12_255_13_255_4_-1_6_1009_7_1_15_1_18_0_8_1|1_255_2_255_3_255_6_1002_5_1_7_1_15_0_18_0_8_1|1_40_2_125_3_255_11_255_12_255_13_255_4_-1_6_1013_7_1_15_1_18_0_8_1|1_40_2_125_3_255_11_255_12_255_13_255_4_-1_6_1014_7_1_15_1_18_0_8_1|1_255_2_255_3_255_6_1004_7_1_15_0_18_0_8_1|1_255_2_255_3_255_6_1003_7_1_15_0_18_0_8_1|1_125_2_255_3_0_11_255_12_255_13_255_4_-1_6_1005_5_1_7_1_15_1_18_0_8_1|1_0_2_255_3_255_11_255_12_255_13_255_4_-1_6_1006_5_1_7_1_15_1_18_0_8_1|,kA13,0,kA15,0,kA16,0,kA14,,kA6,0,kA7,0,kA25,0,kA17,0,kA18,0,kS39,0,kA2,1,kA3,0,kA8,0,kA4,0,kA9,0,kA10,0,kA22,1,kA23,0,kA24,0,kA27,0,kA40,0,kA48,0,kA41,0,kA42,0,kA28,0,kA29,0,kA31,0,kA32,0,kA36,0,kA43,0,kA44,0,kA45,0,kA46,0,kA47,0,kA33,0,kA34,0,kA35,0,kA37,0,kA38,0,kA39,0,kA19,0,kA26,0,kA20,0,kA21,0,kA11,0;
1,2925,2,-45,3,135,155,1,36,1,111,1,112,1,113,20,114,1;
1,1934,2,-45,3,105,155,1,13,1,36,1,406,1,421,1,422,0.5,10,0.5;
1,914,2,495,3,135,155,2,128,0.5,129,0.5,31,YXNk;
1,2899,2,-45,3,165,155,1,36,1,532,1;)";
            errlevel = string::replace(errlevel, "YXNk", ZipUtils::base64URLEncode(
                "Failed to load " + filename + "!\n" + res.error
            ).c_str());
            return errlevel;
        }

        return res.content;
    };
};

#include <Geode/modify/LevelTools.hpp>
class $modify(MLE_LevelTools, LevelTools) {
	inline static GJGameLevel* DefaultLevel = nullptr;
    static GJGameLevel* getLevel(int levelID, bool dontGetLevelString) {
        //log::debug("{}({}, {})", __FUNCTION__, levelID, dontGetLevelString);

		auto level = DefaultLevel ? DefaultLevel : LevelTools::getLevel(levelID, dontGetLevelString);

        if (levelID == -1) level->m_levelName = "{Coming Soon Page}";
        if (levelID == -2) level->m_levelName = "{The Tower Page}";

        level->m_levelID = levelID; // -1, -2 for listing exists. no default id pls

		if (levelID < 0) return level;

		auto filename = "levels/" + utils::numToString(levelID) + ".object.ini";
		CCFileUtils::get()->m_fullPathCache.erase(filename.c_str());
        if (!fileExists(filename.c_str())) {
			// Create default ini file
            auto newfp = Mod::get()->getConfigDir() / filename;
			file::createDirectoryAll(newfp.parent_path()).err();
            if (auto err = file::writeString(
                newfp, "; Automatically generated."
            ).err()) log::error("{}.writeString: {}", __FUNCTION__, err);
			// Write default str file
            LocalLevelManager::sharedState()->getMainLevelString(levelID);
        }

		auto path = CCFileUtils::get()->fullPathForFilename(filename.c_str(), !"why");

        auto& iniRes = FileCache::getIni(path);
        auto& Ini = iniRes.ini;

        //m_levelID 
        if (!(Ini.KeyExists("GJGameLevel", "m_levelID"))) Ini.SetLongValue(
            "GJGameLevel", "m_levelID", level->m_levelID, "; Level ID "
        );
        else level->m_levelID = Ini.GetLongValue("GJGameLevel", "m_levelID");

        //m_sLevelName
        if (!(Ini.KeyExists("GJGameLevel", "m_levelName"))) Ini.SetValue(
            "GJGameLevel", "m_levelName", escENC(level->m_levelName.c_str()).c_str(), "; Level name"
        );
        else level->m_levelName = escDEC(Ini.GetValue("GJGameLevel", "m_levelName")).c_str();

        //m_difficulty
        if (!(Ini.KeyExists("GJGameLevel", "m_difficulty"))) Ini.SetLongValue(
            "GJGameLevel", "m_difficulty",
            (int)level->m_difficulty,
            "; Difficulties that LevelPage layer supports:\n"
            "; undef = 0,\n"
            "; Easy = 1,\n"
            "; Normal = 2,\n"
            "; Hard = 3,\n"
            "; Harder = 4,\n"
            "; Insane = 5,\n"
            "; Demon = 6"
        );
        else level->m_difficulty = (GJDifficulty)Ini.GetLongValue("GJGameLevel", "m_difficulty");

        //m_stars
        if (!(Ini.KeyExists("GJGameLevel", "m_stars"))) Ini.SetLongValue(
            "GJGameLevel", "m_stars", level->m_stars.value(), "; Stars"
        );
        else level->m_stars = Ini.GetLongValue("GJGameLevel", "m_stars");

        //m_audioTrack
        if (!(Ini.KeyExists("GJGameLevel", "m_audioTrack"))) Ini.SetLongValue(
            "GJGameLevel", "m_audioTrack", level->m_audioTrack, "; Audio Track ID"
        );
        else level->m_audioTrack = Ini.GetLongValue("GJGameLevel", "m_audioTrack");

        //m_songID
        if (!(Ini.KeyExists("GJGameLevel", "m_songID"))) Ini.SetLongValue(
            "GJGameLevel", "m_songID", level->m_audioTrack, "; Song ID"
        );
        else level->m_audioTrack = Ini.GetLongValue("GJGameLevel", "m_songID");

        //m_songIDs
        if (!(Ini.KeyExists("GJGameLevel", "m_songIDs"))) Ini.SetValue(
            "GJGameLevel", "m_songIDs", escENC(level->m_songIDs.c_str()).c_str(), "; List of Song IDs"
        );
        else level->m_songIDs = escDEC(Ini.GetValue("GJGameLevel", "m_songIDs")).c_str();

        //m_sfxIDs
        if (!(Ini.KeyExists("GJGameLevel", "m_sfxIDs"))) Ini.SetValue(
            "GJGameLevel", "m_sfxIDs", escENC(level->m_sfxIDs.c_str()).c_str(), "; List of SFX IDs"
        );
        else level->m_songIDs = escDEC(Ini.GetValue("GJGameLevel", "m_sfxIDs")).c_str();

        //m_twoPlayerMode
        if (!(Ini.KeyExists("GJGameLevel", "m_twoPlayerMode"))) Ini.SetBoolValue(
            "GJGameLevel", "m_twoPlayerMode", level->m_twoPlayerMode, "; Capacity String"
        );
        else level->m_twoPlayerMode = Ini.GetBoolValue("GJGameLevel", "m_twoPlayerMode");

        //m_capacityString
        if (!(Ini.KeyExists("GJGameLevel", "m_capacityString"))) Ini.SetValue(
            "GJGameLevel", "m_capacityString", escENC(level->m_capacityString.c_str()).c_str(), "; Capacity String"
        );
        else level->m_capacityString = escDEC(Ini.GetValue("GJGameLevel", "m_capacityString")).c_str();

        if (!iniRes.ok) {
            level->m_levelName = "[INI ERROR]: " + iniRes.errorStr;
            level->m_difficulty = GJDifficulty::Harder;
        }

        Ini.SaveFile(path.c_str());
        FileCache::invalidate(path);

        return level;
    };
    static bool verifyLevelIntegrity(gd::string p0, int p1) {
        return true;
    }
    static gd::string getAudioFileName(int p0) {
        //log::debug("{}:{}({})", __func__, __LINE__, p0);
        if (auto as = fmt::format("audio.{}.mp3", p0); fileExists(as.c_str())) return as.c_str();
        if (auto as = fmt::format("audio/{}.mp3", p0); fileExists(as.c_str())) return as.c_str();
        return LevelTools::getAudioFileName(p0).c_str();
    };
};

#include <Geode/modify/MusicDownloadManager.hpp>
class $modify(MLE_MusicDownloadManager, MusicDownloadManager) {
    gd::string pathForSFX(int id) {
        //log::debug("{}:{}({})", __func__, __LINE__, id);
        std::filesystem::path ref = MusicDownloadManager::pathForSFX(id).c_str();
        if (auto as = fmt::format("sfx/{}{}", id, ref.extension()); fileExists(as.c_str())) {
            return CCFileUtils::get()->fullPathForFilename(as.c_str(), 0);
        }
        if (auto as = fmt::format("sfx.{}{}", id, ref.extension()); fileExists(as.c_str())) {
            return CCFileUtils::get()->fullPathForFilename(as.c_str(), 0);
        }
        return MusicDownloadManager::pathForSFX(id);
    };
    gd::string pathForSong(int id) {
        //log::debug("{}:{}({})", __func__, __LINE__, id);
        if (auto sc = CCScene::get())
            if (sc->getChildByType<LoadingLayer>(0))
                return MusicDownloadManager::pathForSong(id);
        std::filesystem::path ref = MusicDownloadManager::pathForSong(id).c_str();
        if (auto as = fmt::format("songs/{}{}", id, ref.extension()); fileExists(as.c_str())) {
            return CCFileUtils::get()->fullPathForFilename(as.c_str(), 0);
        }
        if (auto as = fmt::format("song.{}{}", id, ref.extension()); fileExists(as.c_str())) {
            return CCFileUtils::get()->fullPathForFilename(as.c_str(), 0);
        }
        return MusicDownloadManager::pathForSong(id);
    }
};

#endif//LEVELS_MODIFICATION

#ifndef LISTING_OVERTAKE

inline std::string createListingIDs(const std::vector<int>& list) {
    std::string new_listing;
    for (auto it = list.begin(); it != list.end(); ) {
        int start = *it, end = start;
        while (++it != list.end() && *it == end + 1) end++;

        if (end - start < 2) {
            new_listing += fmt::format("{}{}", start, end > start ? fmt::format(",{}", end) : "");
        }
        else {
            new_listing += fmt::format("{}:{}", start, end);
        }
        new_listing += ",";
    }
    if (!new_listing.empty()) new_listing.pop_back();
    return new_listing;
}
inline std::vector<int> parseListingIDs(std::string list) {
    auto rtn = std::vector<int>();

    for (auto entry : string::split(list, ",")) {
        if (string::contains(entry.c_str(), ":")) {
            auto seq = string::split(entry.c_str(), ":");
            auto start = utils::numFromString<int>(seq[0].c_str()).unwrapOr(0);
            auto end = utils::numFromString<int>(seq[1].c_str()).unwrapOr(0);
            bool ew = start > end;
            for (int q = start; ew ? q != (end - 1) : q != (end + 1); ew ? --q : ++q) {
                rtn.push_back(q);
            }
        }
        else {
            auto id = utils::numFromString<int>(entry).unwrapOr(0);
            rtn.push_back(id);
        }
    }

    return rtn;
}

#include <Geode/modify/BoomScrollLayer.hpp>
class $modify(BoomScrollLayerLevelSelectExt, BoomScrollLayer) {
    $override static BoomScrollLayer* create(cocos2d::CCArray * pages, int unk1, bool unk2, cocos2d::CCArray * unk3, DynamicScrollDelegate * delegate) {
        if (delegate and unk3) {
            if (auto layer = exact_cast<LevelSelectLayer*>(delegate)) { //is created for LevelSelectLayer
				auto file = "levels/_list.txt";
                CCFileUtils::get()->m_fullPathCache.erase(file);
                if (!fileExists(file)) { // Create default file
                    auto path = Mod::get()->getConfigDir() / file;
                    file::createDirectoryAll(path.parent_path()).err();
                    //fuck
					std::vector<int> ids;
					for (auto level : unk3->asExt<GJGameLevel*>()) ids.push_back(level->m_levelID);
                    //write
                    if (auto err = file::writeString(
                        path, createListingIDs(ids)
                    ).err()) log::error("{}.writeString: {}", __FUNCTION__, err);
                }
				auto path = CCFileUtils::get()->fullPathForFilename(file, !"why");
                auto read = FileCache::getText(path);

				if (!read.ok) {
					log::error("{}.readString: {}", __FUNCTION__, read.error);
                    if (layer) queueInMainThread( // hi Node IDs
                        [file, read, layer = Ref(layer)] {
                            auto label = CCLabelBMFont::create(
                                fmt::format("Error reading {}!\n{}", file, read.error).c_str(),
                                "bigFont.fnt"
                            );
                            label->setID("err-label"_spr);
                            label->setAlignment(kCCTextAlignmentCenter);
                            label->setWidth(500.000f);
                            label->setPosition(layer->getContentSize() / 2);
                            layer->addChild(label, 999);
                            limitNodeSize(label, layer->getContentSize() * 0.4f, 999.f, 0.1f);
                        }
                    );
				}

                unk3->removeAllObjects();

                for (auto id : parseListingIDs(read.content)) unk3->addObject(
                    GameLevelManager::get()->getMainLevel(id, 0)
                );

                if (!unk3->count()) {
                    unk3->addObject(GameLevelManager::get()->getMainLevel(-1, 0));
                    if (pages) for (auto page : CCArrayExt<CCNode>(pages)) page->setScale(0.f);
                }
            };
        }
        return BoomScrollLayer::create(pages, unk1, unk2, unk3, delegate);
    }
};

#include <Geode/modify/LevelSelectLayer.hpp>
class $modify(MLE_LevelSelectExt, LevelSelectLayer) {
    inline static int LastPlayedPage, LastPlayedPageLevelID, ForceNextTo;
    void keyDown(cocos2d::enumKeyCodes p0, double p1) {
        LevelSelectLayer::keyDown(p0, p1);
        if (auto scroll = typeinfo_cast<BoomScrollLayer*>(this->m_scrollLayer)) {
            MLE_LevelSelectExt::ForceNextTo = scroll->m_page;
        }
    }
    bool init(int instpage) {
        if (instpage + 1 == LastPlayedPageLevelID) instpage = LastPlayedPage;
        if (ForceNextTo) { instpage = ForceNextTo; ForceNextTo = 0; }
        return LevelSelectLayer::init(instpage);
    }
};

#include <Geode/modify/LevelPage.hpp>
class $modify(MLE_LevelPageExt, LevelPage) {
    void updateDynamicPage(GJGameLevel * pLevel) { //dynamic page update function is hell
        Ref level = pLevel;
        LevelPage::updateDynamicPage(level);
        if (auto difficultySprite = m_difficultySprite) {
            auto diffID = static_cast<int>(level->m_difficulty);

            auto sz = difficultySprite->getContentSize(); //limitNodeSize

            auto frameName = fmt::format("diffIcon_{:02d}_btn_001.png", diffID);
            if (CCSpriteFrameCache::get()->m_pSpriteFrames->objectForKey(frameName.c_str())) {
                auto frame = CCSpriteFrameCache::get()->spriteFrameByName(frameName.c_str());
                if (frame) difficultySprite->setDisplayFrame(frame);
            }
            else {
                auto image = CCSprite::create(frameName.c_str());
                if (image) difficultySprite->setDisplayFrame(image->displayFrame());
            }

            limitNodeSize(difficultySprite, sz, 999.f, 0.1f);
        }
    }

    $override bool init(GJGameLevel * level) {
        if (!LevelPage::init(level)) return false;
        return true;
    }
    void onPlay(cocos2d::CCObject * sender) {
        if (auto a = getParent()) if (auto scroll = typeinfo_cast<BoomScrollLayer*>(a->getParent())) {
            MLE_LevelSelectExt::LastPlayedPage = scroll->pageNumberForPosition(this->getPosition());
            MLE_LevelSelectExt::LastPlayedPageLevelID = this->m_level->m_levelID.value();
        }
        LevelPage::onPlay(sender);
    }
    void saveCurrentPageForForceNextTo() {
        if (auto a = getParent()) if (auto scroll = typeinfo_cast<BoomScrollLayer*>(a->getParent())) {
            MLE_LevelSelectExt::ForceNextTo = scroll->pageNumberForPosition(this->getPosition());
        }
    }
    void onSecretDoor(CCObject* sender) { saveCurrentPageForForceNextTo(); LevelPage::onSecretDoor(sender); }
    void onTheTower(CCObject* sender) { saveCurrentPageForForceNextTo(); LevelPage::onTheTower(sender); }
};

#endif//LISTING_OVERTAKE

#ifndef RANDOM_SHIT

#include <Geode/modify/EditorPauseLayer.hpp>
class $modify(MLE_EditorPauseLayer, EditorPauseLayer) {
    $override void saveLevel() {
        EditorPauseLayer::saveLevel();
        auto level = m_editorLayer->m_level;
        if (level->m_levelType == GJLevelType::Main) {
            auto string = CCFileUtils::get()->fullPathForFilename(("levels/" + utils::numToString(level->m_levelID.value()) + ".string.txt").c_str(), !"why");
			file::writeString(string.c_str(), level->m_levelString.c_str()).err();
            auto object = CCFileUtils::get()->fullPathForFilename(("levels/" + utils::numToString(level->m_levelID.value()) + ".object.ini").c_str(), !"why");
            file::writeString(object.c_str(), std::string("; Rewrited by ") + (GameManager::get()->m_playerName).c_str()).err();
            MLE_LevelTools::DefaultLevel = level;
            LevelTools::getLevel(level->m_levelID, true);
			MLE_LevelTools::DefaultLevel = nullptr;
        }
    }
};

#include <Geode/modify/EditorUI.hpp>
class $modify(MLE_EditorUI, EditorUI) {
    void showInfoPopup(float = 0.f) {
        auto pop = MDPopup::create(
            "Main Level Config Editor",
            "* Level settings button open special tools.""\n"
            "* Saving actually writes files.", "OK"
        );
        pop->m_scene = (this);
        pop->show();
    }
    $override bool init(LevelEditorLayer * editorLayer) {
        if (!EditorUI::init(editorLayer)) return false;
        if (m_editorLayer->m_level->m_levelType == GJLevelType::Main) scheduleOnce(
            schedule_selector(MLE_EditorUI::showInfoPopup), 0.f
        );
        return true;
    }
    void openSetupWindows(float = 0.f) {
        //what the fuck
        if (!CCScene::get()->getChildByType<LevelSettingsLayer>(0)) {
            LevelSettingsLayer::create(m_editorLayer->m_levelSettings, m_editorLayer)->show();
        }
        auto popup = createQuickPopup(
            "Main Level Config",
            "\n \n \n \n \n \n",
            "Close", nullptr, [editor = Ref(m_editorLayer)](void*, bool qwe) {}
        );
        Ref level = m_editorLayer->m_level;
        //m_levelName
		auto m_levelNameINP = TextInput::create(300.f, "");
		m_levelNameINP->setCallback([level](auto& str) { level->m_levelName = escDEC(str).c_str(); });
        m_levelNameINP->setString(escENC(level->m_levelName.c_str()).c_str());
		m_levelNameINP->setPosition({ 0.f, 115.000f });
        m_levelNameINP->getInputNode()->addChild(SimpleTextArea::create("Level Name:\n \n \n ", "bigFont.fnt", 0.5f), 0, 100);
        m_levelNameINP->getInputNode()->m_allowedChars = " !\"#$ % &'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
		popup->m_buttonMenu->addChild(m_levelNameINP);
        //m_difficulty
		auto m_difficultyINP = TextInput::create(67.f, "");
        m_difficultyINP->setCallback([level](auto& str) { level->m_difficulty = (GJDifficulty)utils::numFromString<int>(str).unwrapOr((int)level->m_difficulty); });
        m_difficultyINP->setString(utils::numToString((int)level->m_difficulty).c_str());
        m_difficultyINP->setPosition({ -110.000, 65.000f });
        m_difficultyINP->getInputNode()->addChild(SimpleTextArea::create("Diff:\n \n \n ", "bigFont.fnt", 0.5f, 67.f), 0, 100);
        m_difficultyINP->getInputNode()->m_allowedChars = "0123456789";
        Ref diffprev = CCSprite::create();
		diffprev->runAction(CCRepeatForever::create(CCSequence::create(CallFuncExt::create(
            [diffprev, level] {
                auto diffID = static_cast<int>(level->m_difficulty);
                auto frameName = fmt::format("diffIcon_{:02d}_btn_001.png", diffID);
                if (CCSpriteFrameCache::get()->m_pSpriteFrames->objectForKey(frameName.c_str())) {
                    auto frame = CCSpriteFrameCache::get()->spriteFrameByName(frameName.c_str());
                    if (frame) diffprev->setDisplayFrame(frame);
                }
                else {
                    auto image = CCSprite::create(frameName.c_str());
                    if (image) diffprev->setDisplayFrame(image->displayFrame());
                }
            }
        ), CCDelayTime::create(0.1f), nullptr)));
        diffprev->setPosition({ 33.000f, 15.000f });
        m_difficultyINP->getInputNode()->addChild(diffprev);
		popup->m_buttonMenu->addChild(m_difficultyINP);
        //m_stars
        auto m_starsINP = TextInput::create(67.f, "");
        m_starsINP->setCallback([level](auto& str) { level->m_stars = utils::numFromString<int>(str).unwrapOr(level->m_stars.value()); });
        m_starsINP->setString(utils::numToString(level->m_stars.value()).c_str());
        m_starsINP->setPosition({ -110.000, 15.000f });
        m_starsINP->getInputNode()->addChild(SimpleTextArea::create("Stars:\n \n \n ", "bigFont.fnt", 0.5f, 67.f), 0, 100);
        m_starsINP->getInputNode()->m_allowedChars = "0123456789";
        popup->m_buttonMenu->addChild(m_starsINP);
        //m_audioTrack
        auto m_audioTrackINP = TextInput::create(180.f, "");
        m_audioTrackINP->setCallback([level](auto& str) { level->m_audioTrack = utils::numFromString<int>(str).unwrapOr(level->m_audioTrack); });
        m_audioTrackINP->setString(utils::numToString(level->m_audioTrack).c_str());
        m_audioTrackINP->setPosition({ 58.000f, 56.000f });
        m_audioTrackINP->getInputNode()->addChild(SimpleTextArea::create("Track:\n \n \n ", "bigFont.fnt", 0.5f, 180.f), 0, 100);
        m_audioTrackINP->getInputNode()->m_allowedChars = "-0123456789";
        popup->m_buttonMenu->addChild(m_audioTrackINP);
        //coins replacer
        auto coinitem = CCMenuItemExt::createSpriteExtraWithFrameName(
            "secretCoin_b_01_001.png", 0.775f, [editor = Ref(m_editorLayer)](void*) {
                createQuickPopup(
                    "Replace coins?",
                    "Here you can change all\n<co>User Coins</c>\nin level to\n<cy>Secret Coins</c>",
                    "Nah", "Replace", [editor](void*, bool replace) {
                        if (!replace) return;
                        auto replaced = 0;
                        for (auto obj : CCArrayExt<GameObject*>(editor->m_objects)) if (obj) {
                            if (obj->m_objectID == 1329) {
                                obj->m_savedObjectType = GameObjectType::SecretCoin;
                                obj->m_objectType = GameObjectType::SecretCoin;
                                obj->m_objectID = 142;
                                obj->customSetup();
                                if (auto e = typeinfo_cast<EffectGameObject*>(obj)) e->m_secretCoinID = ++replaced;
                            }
                        };
                        Notification::create(
                            fmt::format("Changed {} coins", replaced),
                            CCSprite::createWithSpriteFrameName("GJ_coinsIcon_001.png")
                        )->show();
                        editor->m_level->m_coins = replaced;
                        EditorPauseLayer::create(editor)->saveLevel();
                    }
                );
            }
        );
		coinitem->setPosition({ 88.000f, 10.000f });
		popup->m_buttonMenu->addChild(coinitem);
        //GJ_trashBtn_001.png
        auto reset = CCMenuItemExt::createSpriteExtraWithFrameName(
            "GJ_trashBtn_001.png", 0.775f, [level](void*) {
                auto Deletion = createQuickPopup(
                    "Deletion.",
                    "Here you can reset config and string for level or remove level at all.\n<cr>Both of actions you can't undone and kicks you out editor!</c>",
                    "Reset", "Remove", [level](void*, bool remove) {
                        auto string = CCFileUtils::get()->fullPathForFilename(("levels/" + utils::numToString(level->m_levelID.value()) + ".string.txt").c_str(), !"why");
                        file::writeString(string.c_str(), LocalLevelManager::sharedState()->m_mainLevels[level->m_levelID].c_str()).err();
                        auto object = CCFileUtils::get()->fullPathForFilename(("levels/" + utils::numToString(level->m_levelID.value()) + ".object.ini").c_str(), !"why");
                        file::writeString(object.c_str(), std::string("; Reset. Rewrited by ") + (GameManager::get()->m_playerName).c_str()).err();
                        LevelTools::getLevel(level->m_levelID, true);
                        if (remove) {
                            auto path = CCFileUtils::get()->fullPathForFilename("levels/_list.txt", !"why");
                            auto read = file::readString(path.c_str()).unwrapOrDefault();
                            auto list = parseListingIDs(read);
                            list.erase(std::remove(list.begin(), list.end(), level->m_levelID.value()), list.end());
                            file::writeString(path.c_str(), createListingIDs(list)).err();
                        };
                        // kick out of editor yea
                        if (auto editor = GameManager::get()->getEditorLayer()) {
                            auto pause = EditorPauseLayer::create(editor);
                            pause->onExitNoSave(pause);
                        }
                    }
                );
				auto close = CCMenuItemExt::createSpriteExtraWithFrameName(
                    "GJ_closeBtn_001.png", 1.f, [Deletion](void*) {
                        Deletion->removeFromParent();
                    }
                );
                close->setPosition({ -162.000f, 136.000f });
                Deletion->m_buttonMenu->addChild(close);
                Deletion->setKeyboardEnabled(false);
                Deletion->setKeypadEnabled(false);
            }
        );
        reset->setPosition({ 128.000f, 10.000f });
		popup->m_buttonMenu->addChild(reset);
    };
    void onSettings(cocos2d::CCObject * sender) {
        EditorUI::onSettings(sender);
        if (m_editorLayer->m_level->m_levelType == GJLevelType::Main) scheduleOnce(
            schedule_selector(MLE_EditorUI::openSetupWindows), 0.f
        );
    }
};

#include <Geode/modify/GameObject.hpp>
class $modify(MLE_GameObjectExt, GameObject) {
    void customSetup(float) { customSetup(); };
    void customSetup() {
        if (auto v = this->getUserObject("org-"_spr + std::string("m_objectID"))) this->m_objectID = v->getTag();
        if (auto v = this->getUserObject("org-"_spr + std::string("m_objectType"))) this->m_objectType = (GameObjectType)v->getTag();
        if (auto v = this->getUserObject("org-"_spr + std::string("m_savedObjectType"))) this->m_savedObjectType = (GameObjectType)v->getTag();
        GameObject::customSetup();
    };
    void PlayLayerCustomSetup(float) {
        if (auto play = typeinfo_cast<PlayLayer*>(this)) {
            auto lvl = play->m_level;
            if (auto v = lvl->getUserObject("org-"_spr + std::string("m_localOrSaved"))) lvl->m_localOrSaved = v->getTag();
            if (auto v = lvl->getUserObject("org-"_spr + std::string("m_levelType"))) lvl->m_levelType = (GJLevelType)v->getTag();
        }
    };
    static auto valTagContainerObj(int val) { auto a = new CCObject(); a->autorelease(); a->setTag(val); return a; };
    static GameObject* objectFromVector(
        gd::vector<gd::string>&p0, gd::vector<void*>&p1, GJBaseGameLayer * p2, bool p3
    ) {
        auto rtn = GameObject::objectFromVector(p0, p1, p2, p3);
        if (!rtn) return rtn;
        if (auto editor = typeinfo_cast<LevelEditorLayer*>(p2)) {
            if (rtn) if (rtn->m_objectID == 142) {
                rtn->setUserObject("org-"_spr + std::string("m_objectID"), valTagContainerObj(rtn->m_objectID));
                rtn->m_objectID = 1329; //user coin object id
                rtn->setUserObject("org-"_spr + std::string("m_objectType"), valTagContainerObj((int)rtn->m_objectType));
                rtn->m_objectType = GameObjectType::UserCoin; //user coin object
                rtn->setUserObject("org-"_spr + std::string("m_savedObjectType"), valTagContainerObj((int)rtn->m_savedObjectType));
                rtn->m_savedObjectType = GameObjectType::UserCoin; //what
                rtn->scheduleOnce(schedule_selector(MLE_GameObjectExt::customSetup), 0.f);
            }
        };
        if (auto play = typeinfo_cast<PlayLayer*>(p2)) {
            if (rtn) if (rtn->m_objectID == 142) {
                auto lvl = play->m_level;
                lvl->setUserObject("org-"_spr + std::string("m_localOrSaved"), valTagContainerObj(lvl->m_localOrSaved));
                lvl->m_localOrSaved = true;
                lvl->setUserObject("org-"_spr + std::string("m_levelType"), valTagContainerObj((int)lvl->m_levelType));
                lvl->m_levelType = GJLevelType::Main;
                play->scheduleOnce(schedule_selector(MLE_GameObjectExt::PlayLayerCustomSetup), 0.f);
            }
        }
        return rtn;
    }
};

void openListingEditor() {
    auto path = CCFileUtils::get()->fullPathForFilename("levels/_list.txt", !"why");
    auto read = file::readString(path.c_str()).unwrapOrDefault();
    auto list = parseListingIDs(read);

    Ref pGJLevelList = GJLevelList::create();
    pGJLevelList->m_listType = GJLevelType::Editor;
    pGJLevelList->m_listDesc = ZipUtils::base64URLEncode(
        "This list dummy have <cg>realtime sync</c> with\nreal <cy>config file</c> of mod.");
    pGJLevelList->m_listName = "LEVEL SELECT PAGE LIST";
    for (auto id : list) pGJLevelList->addLevelToList(LevelTools::getLevel(id, 0));

    Ref layer = LevelListLayer::create(pGJLevelList);
    switchToScene(layer);
    //keyback leads to main menu
	auto CreatorLayerDummy = CreatorLayer::create();
    CreatorLayerDummy->setScale(0.f);
	layer->getParent()->addChild(CreatorLayerDummy);
	layer->setKeypadEnabled(false);
    layer->setKeyboardEnabled(false);
    //huh
    findFirstChildRecursive<CCMenuItem>(
        layer, [layer](CCMenuItem* item) {
            auto is = &isSpriteFrameName;
            if (is(item, "GJ_arrow_01_001.png")) item->m_pfnSelector = menu_selector(CreatorLayer::onBack);
            if (is(item, "GJ_deleteBtn_001.png")) item->setVisible(0);
            if (is(item, "GJ_updateBtn_001.png")) item->setVisible(0);
            if (is(item, "GJ_chatBtn_001.png")) item->setVisible(0);
            if (is(item, "GJ_plainBtn_001.png")) item->setVisible(0);
            if (is(item, "GJ_duplicateBtn_001.png")) item->setVisible(0);
            if (is(item, "GJ_shareBtn_001.png")) {
                item->setVisible(0);
                auto newb = CCMenuItemExt::createSpriteExtraWithFrameName(
                    "GJ_newBtn_001.png", 0.7f, [layer](void*) {
                        Ref input = TextInput::create(310.f, "ID");
                        input->getInputNode()->m_allowedChars = "-0123456789";
                        input->setPositionY(42.000f);
                        auto popup = createQuickPopup(
                            "Add ID:", "\n ", "Add", nullptr,
                            [input, layer](void*, bool) {
                                input->getString();
                                layer->m_levelList->addLevelToList(LevelTools::getLevel(
                                    utils::numFromString<int>(input->getString().c_str()).unwrapOrDefault(), 0
                                ));
                                layer->onRefreshLevelList(0);
                            }
                        );
                        popup->m_buttonMenu->addChild(input);
                    }
                );
                newb->setPosition(item->getPosition());
                item->getParent()->addChild(newb);
                //menu
				item->getParent()->setScale(1.175f);
				item->getParent()->setPositionX(item->getParent()->getPositionX() - 6.f);
            }
            return false;
        }
    );
    if (auto a = layer->getChildByType<CCTextInputNode>(0))
        a->setTouchEnabled(false);
    //updaterrr
    layer->runAction(CCRepeatForever::create(CCSequence::create(
        CCDelayTime::create(0.1f),
        CallFuncExt::create(
            [path, layer] {
                std::vector<int> list;
                auto strl = layer->m_levelList->m_levelsString;
                for (auto id : string::split(strl.c_str(), ",")) list.push_back(
                    utils::numFromString<int>(id.c_str()).unwrapOrDefault()
                );
                auto liststr = createListingIDs(list);
                file::writeString(path.c_str(), liststr.c_str()).err();
                if (auto l = layer->getChildByType<CCLabelBMFont>(0))
                    l->setString(liststr.c_str());
                if (auto l = layer->getChildByType<LoadingCircle>(0))
                    l->setVisible(false);
            }
        ),
        nullptr
    )));
}
$on_mod(Loaded) {
    ModPopupUIEvent().listen(
        [](FLAlertLayer* popup, std::string_view modID, std::optional<Mod*> mod) -> bool {
            if (modID == GEODE_MOD_ID) {
                auto settings = typeinfo_cast<CCMenuItemSpriteExtra*>(popup->querySelector("settings-button"));
                if (settings) {//accountBtn_settings_001.png
                    auto a = CCSprite::createWithSpriteFrameName("accountBtn_settings_001.png");
					a->setScale(0.725f);
					settings->setSprite(a);
					settings->setEnabled(true);
                    CCMenuItemExt::assignCallback<CCMenuItemSpriteExtra>(
                        settings, [popup](CCMenuItemSpriteExtra*) { openListingEditor(); }
                    );
                }
            }
            return ListenerResult::Propagate;
        }
    ).leak();
}

#endif 
