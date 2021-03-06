#pragma once
#include "CoreState.h"
#include "SPM_Spec.h"
#include "ShiroPhysoMino.h"
#include <string>
#include <utility>
#include <vector>

#define TGM_CELL_MONO -5
#define TGM_FIELD_W_LIMITER -30
#define TGM_WALL TGM_FIELD_W_LIMITER

#define TGM_CELL_BRACKETS (1 << 24)
#define TGM_CELL_GHOST (1 << 25)
#define TGM_CELL_GEM (1 << 26)

enum TGM_grade {
    tgm_grade_none = -1,

    tgm_grade_9 = 0,
    tgm_grade_8,
    tgm_grade_7,
    tgm_grade_6,
    tgm_grade_5,
    tgm_grade_4,
    tgm_grade_3,
    tgm_grade_2,
    tgm_grade_1,

    tgm_grade_S1,
    tgm_grade_S2,
    tgm_grade_S3,
    tgm_grade_S4,
    tgm_grade_S5,
    tgm_grade_S6,
    tgm_grade_S7,
    tgm_grade_S8,
    tgm_grade_S9,

    tgm_grade_S10,
    tgm_grade_S11,
    tgm_grade_S12,
    tgm_grade_S13,

    tgm_grade_m1,
    tgm_grade_m2,
    tgm_grade_m3,
    tgm_grade_m4,
    tgm_grade_m5,
    tgm_grade_m6,
    tgm_grade_m7,
    tgm_grade_m8,
    tgm_grade_m9,

    tgm_grade_M,
    tgm_grade_MK,
    tgm_grade_MV,
    tgm_grade_MO,
    tgm_grade_MM,
    tgm_grade_GM
};

enum TGM_medal {
    tgm_medal_none = 0,

    tgm_medal_bronze,
    tgm_medal_silver,
    tgm_medal_gold,
    tgm_medal_platinum
};

struct TGM_frameCounters : public SPM_frameCounters {
    int postLock;
    int postLockExpirePoint;

    int holdFlash;
    int holdFlashExpirePoint;
};

class TGM_Mino : public ActivatedPolyomino
{
public:
    TGM_Mino(Polyomino& p, SPM_minoID ID, SPM_point position)
        : ActivatedPolyomino(p, ID, position) {
        brackets = false;
        mono = false;
        ghost = false;
    }

    virtual int codedCellValue() override {
        if(mono) {
            return TGM_CELL_MONO;
        }
        else if(brackets) {
            return (ID + 1) | TGM_CELL_BRACKETS;
        }
        else if(ghost) {
            return (ID + 1) | TGM_CELL_GHOST;
        }

        return ID + 1;
    }

    bool brackets;
    bool mono;
    bool ghost; // only draw the outline around the piece
};

struct TGM_sectionData {
    long time;
    long coolTime;
    long levelStopTime;

    int numTetrises;
};

struct TGM_Player {
    TGM_Player();

    SPM_Randomizer *randomizer;

    TGM_Mino *mino;
    std::vector<TGM_Mino *> previews;
    TGM_Mino *hold;

    SPM_playPhase playPhase;

    SPM_frameTimings timings;
    TGM_frameCounters counters;

    std::vector<SPM_minoID> minoSequence;
    unsigned int minoSeqIndex;

    TGM_grade grade;
    int internalGrade;
    int gradePoints;
    int gradeDecayCounter;
    int gradeUp_timestamp;

    int level;
    int section;
    int score;

    bool mroll_unlocked;
    long section_timestamp;



    struct placementData {
        int softDropCounter;
        int sonicDropHeight;
        int activePieceTime;
        int speed;
        int levelStopCounter;

        bool lockHeld;
        int levelInc;
        int lastClear;
        int combo;
        int comboSimple;
    };

    struct statistics {
        int numSingles;
        int numDoubles;
        int numTriples;
        int numTetrises;
        int numPentrises;

        std::vector<TGM_sectionData> sectionData;
    };
};

struct TGM_GameState {
    uint32_t stateFlags;

    int garbageDelay;
    double rank;
    int music;
};

//enum TGM_levelEventType {tgm_event_garbage_on,};

class TGM_LevelEvent {
public:
    TGM_LevelEvent(int level) : level(level) {}
    virtual ~TGM_LevelEvent() {}

    // TODO, figure out the state structures class TGM will use, and pass state off to this function
    virtual void execute() = 0;

    int level;
};

class TGM_SpeedEvent : public TGM_LevelEvent {
public:
    TGM_SpeedEvent(int level, SPM_frameTimings timings)
        : TGM_LevelEvent(level)
    {
        this->timings = timings;
    }

    void execute();

protected:
    SPM_frameTimings timings;
};

class TGM_MusicEvent : public TGM_LevelEvent {

};

struct TGM_Mode {
    SPM_Spec *spec;
    SPM_Randomizer *randomizer;

    std::vector<std::pair<int, SPM_frameTimings>> speedCurve;

    int numPlayers;
    std::vector<SPM_offset> playerSpawnOffsets;

    //std::vector<std::unique_ptr<TGM_LevelEvent>>
    // TODO: music list, garbage speed updates, garbage on/off, invisible on/off, etc.
    // have a list of LevelEvents

    // maybe have member functions like onLinesCleared() ??
    // account for small things like garbage counter behavior when lines are cleared (different between TGM+ and Shirase, for example)

    // TODO: piece spawn positions
    // TODO: include all multi-editor options like forced bracket mode, infinite floorkicks etc.

    int startingLevel;
    int finalLevel;
    bool enableLevelStops;

    bool overrideFieldSize;
    Shiro::Grid *startingField;
};

struct TGM_Stage {
    int index;
    std::string name;

    Shiro::Grid *startingField;
};

struct Staged_TGM_Mode {

};

class TGM : public ShiroPhysoMino {
public:
    TGM(CoreState&, SPM_Spec *, TGM_Mode&);
    ~TGM();

    int init();
    int quit();
    int input();
    int frame();
    int draw();

protected:
    std::vector<TGM_Player> players;

    int recoveries;
    bool isRecovering;

    TGM_medal medalST;
    TGM_medal medalSK;
    TGM_medal medalCO;
    TGM_medal medalAC;

    TGM_medal medalRE;
    TGM_medal medalRO;

    unsigned long medalST_timestamp;
    unsigned long medalSK_timestamp;
    unsigned long medalCO_timestamp;
    unsigned long medalAC_timestamp;

    unsigned long medalRE_timestamp;
    unsigned long medalRO_timestamp;

private:
    TGM_Mode *mode;
};

class Staged_TGM : public TGM {

};