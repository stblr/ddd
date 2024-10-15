#pragma once

#include "payload/StorageScanner.hh"
#include "payload/ZIPFile.hh"

#include <common/Optional.hh>
#include <common/Ring.hh>
#include <common/UniquePtr.hh>
#include <common/storage/Storage.hh>
#include <game/MinimapConfig.hh>
#include <jsystem/JKRHeap.hh>

class CourseManager : public StorageScanner {
public:
    enum {
        MaxCourseCount = 256,
        MaxPackCount = 128,
    };

    class Pack {
    public:
        Pack(Ring<u32, MaxCourseCount> courseIndices);
        virtual ~Pack();

        const Ring<u32, MaxCourseCount> &courseIndices() const;
        Ring<u32, MaxCourseCount> &courseIndices();

        virtual const char *name() const = 0;
        virtual const char *author() const = 0;
        virtual const char *version() const = 0;

    protected:
        Ring<u32, MaxCourseCount> m_courseIndices;
    };

    class Course {
    public:
        Course(Array<u8, 32> archiveHash);
        virtual ~Course();

        Array<u8, 32> archiveHash() const;

        virtual u32 musicID() const = 0;
        virtual const char *name() const = 0;
        virtual const char *author() const = 0;
        virtual const char *version() const = 0;
        virtual const MinimapConfig *minimapConfig() const = 0;
        virtual void *loadThumbnail(JKRHeap *heap) const = 0;
        virtual void *loadNameImage(JKRHeap *heap) const = 0;
        virtual void *loadLogo(JKRHeap *heap) const = 0;
        virtual void *loadStaffGhost(JKRHeap *heap) const = 0;
        virtual void *loadCourse(u32 courseOrder, u32 raceLevel, JKRHeap *heap) const = 0;
        virtual bool isDefault() const = 0;
        virtual bool isCustom() const = 0;

    protected:
        Array<u8, 32> m_archiveHash;
    };

    void start();

    u32 racePackCount() const;
    u32 battlePackCount() const;
    const Pack &racePack(u32 index) const;
    const Pack &battlePack(u32 index) const;
    u32 raceCourseCount(u32 packIndex) const;
    u32 battleCourseCount(u32 packIndex) const;
    const Course &raceCourse(u32 packIndex, u32 index) const;
    const Course &battleCourse(u32 packIndex, u32 index) const;

    static void Init();
    static CourseManager *Instance();

private:
    enum {
        DefaultRaceCourseCount = 16,
        DefaultBattleCourseCount = 6,
        DefaultPackCount = 3,
    };

    class DefaultPack : public Pack {
    public:
        DefaultPack(Ring<u32, MaxCourseCount> courseIndices, Array<char, 32> name);
        ~DefaultPack() override;

        const char *name() const override;
        const char *author() const override;
        const char *version() const override;

    private:
        Array<char, 32> m_name;
    };

    class CustomPack : public Pack {
    public:
        CustomPack(Ring<u32, MaxCourseCount> courseIndices, Array<char, INIFieldSize> name,
                Array<char, INIFieldSize> author, Array<char, INIFieldSize> version);
        ~CustomPack();
        const char *name() const override;
        const char *author() const override;
        const char *version() const override;

    private:
        Array<char, INIFieldSize> m_name;
        Array<char, INIFieldSize> m_author;
        Array<char, INIFieldSize> m_version;
    };

    class DefaultCourse : public Course {
    public:
        DefaultCourse(Array<u8, 32> archiveHash, u32 courseID, const char *thumbnail,
                const char *nameImage);
        ~DefaultCourse() override;

        u32 musicID() const override;
        const char *name() const override;
        const char *author() const override;
        const char *version() const override;
        const MinimapConfig *minimapConfig() const override;
        void *loadThumbnail(JKRHeap *heap) const override;
        void *loadNameImage(JKRHeap *heap) const override;
        void *loadLogo(JKRHeap *heap) const override;
        void *loadStaffGhost(JKRHeap *heap) const override;
        void *loadCourse(u32 courseOrder, u32 raceLevel, JKRHeap *heap) const override;
        bool isDefault() const override;
        bool isCustom() const override;

    private:
        u32 m_courseID;
        const char *m_thumbnail;
        const char *m_nameImage;
    };

    class CustomCourse : public Course {
    public:
        CustomCourse(Array<u8, 32> archiveHash, u32 musicID, Array<char, INIFieldSize> name,
                Array<char, INIFieldSize> author, Array<char, INIFieldSize> version,
                Optional<MinimapConfig> minimapConfig, Array<char, 256> path,
                Array<char, 128> prefix);
        ~CustomCourse() override;

        u32 musicID() const override;
        const char *name() const override;
        const char *author() const override;
        const char *version() const override;
        const MinimapConfig *minimapConfig() const override;
        void *loadThumbnail(JKRHeap *heap) const override;
        void *loadNameImage(JKRHeap *heap) const override;
        void *loadLogo(JKRHeap *heap) const override;
        void *loadStaffGhost(JKRHeap *heap) const override;
        void *loadCourse(u32 courseOrder, u32 raceLevel, JKRHeap *heap) const override;
        bool isDefault() const override;
        bool isCustom() const override;

    private:
        u32 m_musicID;
        Array<char, INIFieldSize> m_name;
        Array<char, INIFieldSize> m_author;
        Array<char, INIFieldSize> m_version;
        Optional<MinimapConfig> m_minimapConfig;
        Array<char, 256> m_path;
        Array<char, 128> m_prefix;
    };

    struct PackINI {
        Array<Array<char, INIFieldSize>, KartLocale::Language::Count> localizedNames;
        Array<char, INIFieldSize> fallbackName;
        Array<Array<char, INIFieldSize>, KartLocale::Language::Count> localizedAuthors;
        Array<char, INIFieldSize> fallbackAuthor;
        Array<char, INIFieldSize> version;
        Array<char, INIFieldSize> defaultCourses;
    };

    struct CourseINI {
        Array<Array<char, INIFieldSize>, KartLocale::Language::Count> localizedNames;
        Array<char, INIFieldSize> fallbackName;
        Array<Array<char, INIFieldSize>, KartLocale::Language::Count> localizedAuthors;
        Array<char, INIFieldSize> fallbackAuthor;
        Array<char, INIFieldSize> version;
        Array<char, INIFieldSize> defaultCourseName;
        Array<char, INIFieldSize> defaultMusicName;
    };

    CourseManager();

    OSThread &thread() override;
    void process() override;

    void addDefaultRaceCourses();
    void addDefaultBattleCourses();
    void addCustomPacksAndCourses(Array<char, 256> &path, Storage::NodeInfo &nodeInfo,
            Ring<u32, MaxCourseCount> &raceCourseIndices,
            Ring<u32, MaxCourseCount> &battleCourseIndices);
    void addCustomCourse(const Array<char, 256> &path, Ring<u32, MaxCourseCount> &raceCourseIndices,
            Ring<u32, MaxCourseCount> &battleCourseIndices);
    void addCustomRacePack(const Array<char, 256> &path, Ring<u32, MaxCourseCount> &courseIndices);
    void addCustomBattlePack(const Array<char, 256> &path,
            Ring<u32, MaxCourseCount> &courseIndices);
    void addCustomPack(const Array<char, 256> &path, Ring<u32, MaxCourseCount> &courseIndices,
            u32 defaultCourseOffset, u32 defaultCourseCount,
            Ring<UniquePtr<Pack>, MaxPackCount> &packs, const char *type);
    void sortRacePacksByName();
    void sortBattlePacksByName();
    void addDefaultRacePacks();
    void addDefaultBattlePacks();
    void addDefaultPacks(const Ring<UniquePtr<Course>, MaxCourseCount> &courses,
            Ring<UniquePtr<Pack>, MaxPackCount> &packs, const char *base, const char *type);
    void sortRacePackCoursesByName();
    void sortBattlePackCoursesByName();
    bool hashFile(ZIPFile &zipFile, const char *filePath, Array<u8, 32> &hash) const;
    bool hashCourseFile(ZIPFile &zipFile, const char *filePath, Array<u8, 32> &hash) const;
    void *loadFile(const char *zipPath, const char *filePath, JKRHeap *heap,
            u32 *size = nullptr) const;
    void *loadFile(ZIPFile &zipFile, const char *filePath, JKRHeap *heap,
            u32 *size = nullptr) const;
    void *loadLocalizedFile(const char *zipPath, const char *prefix, const char *suffix,
            JKRHeap *heap, u32 *size = nullptr) const;
    void *loadLocalizedFile(ZIPFile &zipFile, const char *prefix, const char *suffix, JKRHeap *heap,
            u32 *size = nullptr) const;
    void *loadLocalizedFile(const char *prefix, const char *suffix, JKRHeap *heap,
            u32 *size = nullptr) const;
    void *loadCourseFile(const char *zipPath, const char *filePath, JKRHeap *heap,
            u32 *size = nullptr) const;
    void *loadCourseFile(ZIPFile &zipFile, const char *filePath, JKRHeap *heap,
            u32 *size = nullptr) const;

    static bool GetDefaultCourseID(const char *name, u32 &courseID);
    static void SortPacksByName(Ring<UniquePtr<Pack>, MaxPackCount> &packs);
    static bool ComparePacksByName(const UniquePtr<Pack> &a, const UniquePtr<Pack> &b);
    static bool CompareRaceCourseIndicesByName(const u32 &a, const u32 &b);
    static bool CompareBattleCourseIndicesByName(const u32 &a, const u32 &b);

    Array<u8, 128 * 1024> m_stack;
    OSThread m_thread;
    JKRHeap *m_heap;
    Ring<UniquePtr<Course>, MaxCourseCount> m_raceCourses;
    Ring<UniquePtr<Course>, MaxCourseCount> m_battleCourses;
    Ring<UniquePtr<Pack>, MaxPackCount> m_racePacks;
    Ring<UniquePtr<Pack>, MaxPackCount> m_battlePacks;

    static CourseManager *s_instance;
};
