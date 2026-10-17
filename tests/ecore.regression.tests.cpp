#define SDL_MAIN_HANDLED

#include "engine/ecampaign.h"
#include "engine/etask.h"
#include "buildings/eroad.h"
#include "egamedir.h"
#include "fileIO/efileformat.h"
#include "pak/zeusfile.h"
#include "textures/egametextures.h"
#include "widgets/egamewidget.h"

// This test includes the production parser to access its local record types.
#include "engine/ecampaignreadpak.cpp"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <algorithm>
#include <stdexcept>
#include <thread>
#include <vector>

namespace {
void require(const bool condition, const char* const message) {
    if(!condition) throw std::runtime_error(message);
}

std::vector<char> serializedCampaign(const eCampaign& campaign) {
    std::vector<char> bytes;
    eWriteTarget target(&bytes);
    eWriteStream dst(target);
    dst.writeFormat("eZeus.epak");
    campaign.write(dst);
    return bytes;
}

void loadCampaignBytes(const std::vector<char>& bytes, eCampaign& campaign) {
    eReadSource source(&bytes);
    eReadStream src(source);
    src.readFormat();
    require(!src.failed() && src.format() == "eZeus.epak", "campaign header");
    campaign.read(src);
    require(!src.failed(), "campaign read");
    src.handlePostFuncs();
    require(!src.failed(), "campaign post-load references");
}

struct OwnedDirectory {
    std::filesystem::path path;
    ~OwnedDirectory() { if(!path.empty()) std::filesystem::remove_all(path); }
};

void writeBytes(const std::filesystem::path& path,
                const std::vector<char>& bytes) {
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    require(bool(file), "open test fixture");
    file.write(bytes.data(), bytes.size());
    require(bool(file), "write test fixture");
}

void setInt32(std::vector<char>& bytes, const std::size_t offset,
              const std::int32_t value) {
    require(offset + sizeof(value) <= bytes.size(), "fixture offset");
    std::memcpy(bytes.data() + offset, &value, sizeof(value));
}

std::int32_t int32At(const std::vector<char>& bytes, const std::size_t offset) {
    std::int32_t value = 0;
    require(offset + sizeof(value) <= bytes.size(), "count offset");
    std::memcpy(&value, bytes.data() + offset, sizeof(value));
    return value;
}

std::size_t colonyBoardCountOffset(const std::filesystem::path& path,
                                   const std::size_t priceCount) {
    std::ifstream file(path, std::ios::binary);
    require(bool(file), "open campaign cursor");
    eReadSource source(&file);
    eReadStream src(source);
    src.readFormat();
    int bitmap, parentEpisode, colonyEpisode, fundsCount, value;
    bool isPak;
    std::string name;
    eEpisodeType episodeType;
    eDifficulty difficulty;
    src >> bitmap;
    src >> isPak;
    if(isPak) src >> name;
    src >> name;
    src >> parentEpisode;
    src >> colonyEpisode;
    src >> episodeType;
    src >> fundsCount;
    require(fundsCount >= 0 && fundsCount <= 10, "fixture funds count");
    for(int i = 0; i < fundsCount; ++i) {
        ePlayerId player;
        src >> player;
        src >> value;
    }
    eDate date;
    date.read(src);
    for(std::size_t i = 0; i < priceCount; ++i) src >> value;
    src >> difficulty;
    eWorldBoard world;
    world.read(src);
    eGameBoard board(world);
    board.read(src);
    int playedCount;
    src >> playedCount;
    require(playedCount >= 0 && playedCount <= 4, "fixture played count");
    for(int i = 0; i < playedCount; ++i) src >> value;
    require(!src.failed(), "campaign cursor read");
    return static_cast<std::size_t>(file.tellg());
}

void testCampaignSerialization() {
    const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
    const auto name = "cf01-regression-" + std::to_string(stamp);
    const auto directory = std::filesystem::path(eGameDir::adventuresDir()) / name;
    require(!std::filesystem::exists(directory), "unique fixture directory");
    std::filesystem::create_directories(directory);
    OwnedDirectory cleanup{directory};
    const auto path = directory / (name + ".epak");

    eCampaign original;
    original.initialize(name);
    original.parentCityBoard().resize(2, 2);
    original.setInitialFunds(ePlayerId::player0, 1234);
    const auto bytes = serializedCampaign(original);
    writeBytes(path, bytes);

    const auto boardCountAt = colonyBoardCountOffset(path, original.prices().size());
    require(int32At(bytes, boardCountAt) == 4, "colony board fixture count");
    const auto playedCountAt = boardCountAt - sizeof(std::int32_t);
    require(int32At(bytes, playedCountAt) == 0, "played episode fixture count");
    const auto forColonyCountAt = bytes.size() - 4*sizeof(std::int32_t);
    const auto forParentCountAt = bytes.size() - 3*sizeof(std::int32_t);
    require(int32At(bytes, forColonyCountAt) == 0 &&
            int32At(bytes, forParentCountAt) == 0,
            "set-aside fixture counts");

    const auto lastEpisode = original.colonyEpisodes().back();
    original.colonyEpisodes().pop_back();
    const auto fewerEpisodes = serializedCampaign(original);
    original.colonyEpisodes().push_back(lastEpisode);
    const auto mismatch = std::mismatch(bytes.begin(), bytes.end(),
                                        fewerEpisodes.begin(), fewerEpisodes.end());
    const auto episodeCountAt = static_cast<std::size_t>(mismatch.first - bytes.begin());
    require(int32At(bytes, episodeCountAt) == 4 &&
            int32At(fewerEpisodes, episodeCountAt) == 3,
            "colony episode fixture count");

    const auto lastParent = original.parentCityEpisodes().back();
    original.parentCityEpisodes().pop_back();
    const auto fewerParents = serializedCampaign(original);
    original.parentCityEpisodes().push_back(lastParent);
    const auto parentMismatch = std::mismatch(bytes.begin(), bytes.end(),
                                              fewerParents.begin(), fewerParents.end());
    const auto parentCountAt =
        static_cast<std::size_t>(parentMismatch.first - bytes.begin());
    require(int32At(bytes, parentCountAt) == 1 &&
            int32At(fewerParents, parentCountAt) == 0,
            "parent episode fixture count");

    original.setInitialFunds(ePlayerId::player1, 5678);
    const auto moreFunds = serializedCampaign(original);
    const auto fundsMismatch = std::mismatch(bytes.begin(), bytes.end(),
                                             moreFunds.begin(), moreFunds.end());
    const auto fundsCountAt =
        static_cast<std::size_t>(fundsMismatch.first - bytes.begin());
    require(int32At(bytes, fundsCountAt) == 1 &&
            int32At(moreFunds, fundsCountAt) == 2,
            "funds fixture count");

    eCampaign loaded;
    require(loaded.load(name), "valid campaign load");
    require(loaded.parentCityBoard().width() == 2 &&
            loaded.parentCityBoard().height() == 2 &&
            loaded.initialFunds(ePlayerId::player0) == 1234 &&
            loaded.colonyEpisodes().size() == 4,
            "valid campaign state");

    const auto savedAgain = serializedCampaign(loaded);
    eCampaign reloaded;
    loadCampaignBytes(savedAgain, reloaded);
    require(reloaded.parentCityBoard().width() == 2 &&
            reloaded.initialFunds(ePlayerId::player0) == 1234 &&
            reloaded.colonyEpisodes().size() == 4,
            "save-load-save-reload state");

    auto newer = bytes;
    setInt32(newer, sizeof(std::int32_t) + std::string("eZeus.epak").size(),
             eFileFormat::version + 1);
    writeBytes(path, newer);
    eCampaign rejected;
    require(!rejected.load(name) && rejected.currentEpisode() == nullptr,
            "newer campaign version rejected");

    auto truncated = bytes;
    truncated.resize(truncated.size() / 2);
    writeBytes(path, truncated);
    eCampaign incomplete;
    require(!incomplete.load(name), "truncated campaign rejected");

    auto truncatedTail = bytes;
    truncatedTail.pop_back();
    writeBytes(path, truncatedTail);
    eCampaign incompleteTail;
    require(!incompleteTail.load(name), "late truncation rejected");

    for(const int count : {-1, 129}) {
        auto invalid = bytes;
        setInt32(invalid, boardCountAt, count);
        writeBytes(path, invalid);
        eCampaign rejectedCount;
        require(!rejectedCount.load(name) &&
                rejectedCount.currentEpisode() == nullptr,
                "invalid colony board count rejected");
    }
    for(const int count : {-1, 5}) {
        auto invalid = bytes;
        setInt32(invalid, episodeCountAt, count);
        writeBytes(path, invalid);
        eCampaign rejectedCount;
        require(!rejectedCount.load(name), "invalid colony episode count rejected");
    }
    for(const int count : {-1, 129}) {
        auto invalidParents = bytes;
        invalidParents.resize(parentCountAt + sizeof(std::int32_t));
        setInt32(invalidParents, parentCountAt, count);
        writeBytes(path, invalidParents);
        eCampaign rejectedParents;
        require(!rejectedParents.load(name), "invalid parent episode count rejected");
        require(rejectedParents.parentCityEpisodes().empty(),
                "invalid parent count rejected before partial allocation");
    }
    for(const auto offset : {playedCountAt, forColonyCountAt,
                             forParentCountAt}) {
        auto invalid = bytes;
        setInt32(invalid, offset, -1);
        writeBytes(path, invalid);
        eCampaign rejectedCount;
        require(!rejectedCount.load(name),
                "negative campaign collection count rejected");
    }
    auto invalidFunds = bytes;
    setInt32(invalidFunds, fundsCountAt, -1);
    const auto firstFund = invalidFunds.begin() + fundsCountAt + sizeof(std::int32_t);
    invalidFunds.erase(firstFund, firstFund + 2*sizeof(std::int32_t));
    writeBytes(path, invalidFunds);
    eCampaign rejectedFunds;
    require(!rejectedFunds.load(name), "negative funds count rejected");
}

void testBoardLifecycle() {
    eWorldBoard world;
    for(int cycle = 0; cycle < 8; ++cycle) {
        eGameBoard board(world);
        board.initialize(4, 4);
        auto* const retained = board.dtile(1, 1);
        require(retained != nullptr, "initial board tile");
        auto road = e::make_shared<eRoad>(board, eCityId::city0);
        std::weak_ptr<eRoad> removedRoad = road;
        board.dtile(3, 3)->setUnderBuilding(road);
        road.reset();
        board.resize(2, 2);
        require(board.dtile(1, 1) == retained &&
                board.dtile(3, 3) == nullptr, "board shrink and overlap");
        require(removedRoad.expired(), "removed tile releases its building");
        board.resize(5, 3);
        require(board.dtile(1, 1) == retained &&
                board.dtile(4, 2) != nullptr, "board expansion and overlap");
        board.initialize(1, 1);
        require(board.dtile(4, 2) == nullptr &&
                board.dtile(0, 0) != nullptr, "board reload dimensions");
        board.dtile(0, 0)->setAltitude(3, false);
        std::vector<char> bytes;
        eWriteTarget target(&bytes);
        eWriteStream dst(target);
        board.write(dst);
        eReadSource source(&bytes);
        eReadStream src(source);
        board.read(src);
        src.handlePostFuncs();
        require(!src.failed() && board.width() == 1 &&
                board.dtile(0, 0)->altitude() == 3,
                "board save and reload state");
    }
}

class CountingTask final : public eTask {
public:
    CountingTask(std::atomic<int>& started, std::atomic<int>& ran,
                 std::atomic<int>& finished, std::atomic<int>& destroyed) :
        eTask(eCityId::city0), mStarted(started), mRan(ran),
        mFinished(finished), mDestroyed(destroyed) {}
    ~CountingTask() override { ++mDestroyed; }
private:
    void run(eThreadBoard&) override {
        ++mStarted;
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        ++mRan;
    }
    void finish() override { ++mFinished; }
    std::atomic<int>& mStarted;
    std::atomic<int>& mRan;
    std::atomic<int>& mFinished;
    std::atomic<int>& mDestroyed;
};

void testThreadPoolLifecycle() {
    eWorldBoard world;
    eGameBoard board(world);
    board.initialize(2, 2);
    board.addCityToBoard(eCityId::city0);
    for(int cycle = 0; cycle < 8; ++cycle) {
        std::atomic<int> started{0};
        std::atomic<int> ran{0};
        std::atomic<int> finished{0};
        std::atomic<int> destroyed{0};
        {
            eThreadPool pool(board);
            pool.initialize(2, 2);
            pool.addBoard(eCityId::city0);
            pool.initialize(2, 2);
            for(int i = 0; i < 24; ++i) {
                pool.queueTask(new CountingTask(started, ran, finished,
                                                destroyed));
            }
            if(cycle % 2 == 0) {
                pool.waitFinished();
                pool.handleFinished();
                require(ran == 24 && finished == 24,
                        "queued tasks and finish callbacks complete");
            } else {
                const auto deadline = std::chrono::steady_clock::now() +
                                      std::chrono::seconds(2);
                while(started == 0 && std::chrono::steady_clock::now() < deadline) {
                    std::this_thread::yield();
                }
                require(started > 0, "worker running during shutdown");
            }
        }
        require(destroyed == 24, "queued and running tasks released");
    }
}

void setU16(std::vector<char>& bytes, const std::size_t offset,
            const std::uint16_t value) {
    bytes[offset] = static_cast<char>(value & 0xff);
    bytes[offset + 1] = static_cast<char>(value >> 8);
}

void testPakEventAlignment() {
    const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
    const auto path = std::filesystem::temp_directory_path() /
        ("cf01-events-" + std::to_string(stamp) + ".pak");
    struct OwnedFile {
        std::filesystem::path path;
        ~OwnedFile() { std::filesystem::remove(path); }
    } cleanup{path};
    constexpr std::size_t recordSize = 124;
    std::vector<char> bytes(39166, 0);
    bytes[recordSize] = 8;        // valid wage increase at index 1
    setU16(bytes, recordSize + 34, 6); // trigger valid index 6
    bytes[2*recordSize] = static_cast<char>(255); // invalid type
    bytes[3*recordSize] = 9;      // valid wage decrease
    bytes[4*recordSize] = 1;      // troops request with invalid city effect
    setU16(bytes, 4*recordSize + 84, 9);
    setU16(bytes, 4*recordSize + 94, 1);
    bytes[5*recordSize] = 4;      // god quest with invalid god
    setU16(bytes, 5*recordSize + 94, 255);
    bytes[6*recordSize] = 8;      // following valid event
    setU16(bytes, 6*recordSize + 34, 1000); // out-of-range reference
    writeBytes(path, bytes);

    eWorldBoard world;
    eGameBoard board(world);
    board.initialize(1, 1);
    eEpisode episode;
    episode.fBoard = &board;
    episode.fWorldBoard = &world;
    ZeusFile file(path.string());
    file.readVersion();
    file.seek(0);
    readEpisodeEvents(episode, file, 7, eCityId::city0,
                      std::vector<ePakGod>(2), ePakMonster{});
    const auto& events = episode.fEvents[eCityId::city0];
    require(events.size() == 7 && !events[0] && events[1] &&
            !events[2] && events[3] && !events[4] && !events[5] &&
            events[6], "PAK event placeholders preserve indices");
    require(events[1]->baseTrigger().eventCount() == 1 &&
            events[1]->baseTrigger().events().front() == events[6],
            "PAK trigger keeps original target index");
    require(events[6]->baseTrigger().eventCount() == 0,
            "out-of-range PAK reference ignored");
}

bool readSaveBytes(const std::vector<char>& bytes,
                   eGameWidgetSettings& settings,
                   stdsptr<eCampaign>& campaign) {
    eReadSource source(&bytes);
    eReadStream src(source);
    src.readFormat();
    if(src.failed() || src.format() != "eZeus.ez" ||
       src.formatVersion() > eFileFormat::version) return false;
    settings.read(src);
    if(src.failed()) return false;
    const auto loaded = std::make_shared<eCampaign>();
    loaded->read(src);
    if(src.failed()) return false;
    src.handlePostFuncs();
    if(src.failed()) return false;
    campaign = loaded;
    return true;
}

void testAuthenticSave() {
    const char* const fixture = std::getenv("EZEUS_LEGACY_SAVE_FIXTURE");
    if(!fixture || !*fixture) return;

    eSettings noTextures;
    noTextures.fTinyTextures = false;
    noTextures.fSmallTextures = false;
    noTextures.fMediumTextures = false;
    noTextures.fLargeTextures = false;
    eGameTextures::setSettings(noTextures);

    const std::filesystem::path path(fixture);
    std::ifstream file(path, std::ios::binary);
    require(bool(file), "open authentic save copy");
    const std::vector<char> bytes(std::istreambuf_iterator<char>{file}, {});
    require(bytes.size() > 64, "authentic save is nonempty");

    eGameWidgetSettings settings;
    stdsptr<eCampaign> campaign;
    require(readSaveBytes(bytes, settings, campaign), "load authentic save");
    const auto state = [](eCampaign& c) {
        const auto& date = c.date();
        return std::vector<long long>{
            c.parentCityBoard().width(), c.parentCityBoard().height(),
            static_cast<long long>(c.parentCityBoard().characters().size()),
            static_cast<long long>(c.worldBoard().cities().size()),
            static_cast<long long>(c.parentCityEpisodes().size()),
            static_cast<long long>(c.colonyEpisodes().size()),
            c.currentEpisodeId(), static_cast<int>(c.currentEpisodeType()),
            c.initialFunds(ePlayerId::player0),
            date.year(), static_cast<int>(date.month()), date.day()
        };
    };
    const auto originalState = state(*campaign);
    require(originalState[0] > 0 && originalState[1] > 0 &&
            originalState[4] > 0, "authentic gameplay state");

    const auto runId = std::chrono::steady_clock::now()
                           .time_since_epoch().count();
    const auto output = path.parent_path() /
        (path.stem().string() + "-roundtrip-" + std::to_string(runId) + ".ez");
    require(output != path && !std::filesystem::exists(output),
            "round-trip destination is new");
    std::ofstream saved(output, std::ios::binary);
    require(bool(saved), "open separate round-trip save");
    eWriteTarget target(&saved);
    eWriteStream dst(target);
    dst.writeFormat("eZeus.ez");
    settings.write(dst);
    campaign->write(dst);
    saved.close();
    require(bool(saved), "write round-trip save");

    std::ifstream reloadFile(output, std::ios::binary);
    require(bool(reloadFile), "open round-trip save");
    const std::vector<char> reloadedBytes(
        std::istreambuf_iterator<char>{reloadFile}, {});
    eGameWidgetSettings reloadedSettings;
    stdsptr<eCampaign> reloaded;
    require(readSaveBytes(reloadedBytes, reloadedSettings, reloaded),
            "reload round-trip save");
    require(state(*reloaded) == originalState &&
            reloadedSettings.fPaused == settings.fPaused &&
            reloadedSettings.fSpeed == settings.fSpeed &&
            reloadedSettings.fBookmarks == settings.fBookmarks,
            "round-trip gameplay and widget state");

    auto future = bytes;
    setInt32(future, 12, eFileFormat::version + 1);
    eGameWidgetSettings rejectedSettings;
    stdsptr<eCampaign> rejected;
    require(!readSaveBytes(future, rejectedSettings, rejected) && !rejected,
            "reject newer save version");
    auto malformed = bytes;
    malformed[4] = 'x';
    require(!readSaveBytes(malformed, rejectedSettings, rejected) && !rejected,
            "reject malformed save format");
    auto truncated = bytes;
    truncated.resize(truncated.size() - 32);
    require(!readSaveBytes(truncated, rejectedSettings, rejected) && !rejected,
            "reject truncated save");

    std::cout << "LEGACY_STATE";
    for(const auto value : originalState) std::cout << ' ' << value;
    std::cout << '\n';
}

void run(const char* name, void (*test)(), int& failures) {
    try {
        test();
        std::cout << "PASS " << name << '\n';
    } catch(const std::exception& error) {
        ++failures;
        std::cerr << "FAIL " << name << ": " << error.what() << '\n';
    }
}
}

int main() {
    int failures = 0;
    run("campaign serialization", testCampaignSerialization, failures);
    run("board lifecycle", testBoardLifecycle, failures);
    run("thread pool lifecycle", testThreadPoolLifecycle, failures);
    run("PAK event alignment", testPakEventAlignment, failures);
    if(std::getenv("EZEUS_LEGACY_SAVE_FIXTURE"))
        run("authentic save round-trip", testAuthenticSave, failures);
    return failures == 0 ? 0 : 1;
}
