#include <stdio.h>
#include <stdint.h>

struct mStr {
    char Index;       /**< Mission Index Number */
    char Name[50];    /**< Name of Mission */
    char Abbr[24];    /**< Name of Mission Abbreviated */
    char Code[62];    /**< Mission Coding String */
    int8_t Alt[36];     /**< Alternate Sequence */
    int8_t AltD[36];    /**< Alternate Sequence for Deaths (Jt Missions Only) */
    int8_t Days,        /**< Duration Level */
           Dur,              /**< Duration Mission */
           Doc,              /**< Docking Mission */
           EVA,              /**< EVA Mission */
           LM,               /**< LM Mission */
           Jt,               /**< Joint Mission */
           Lun,              /**< Lunar Mission */
           mEq,              /**< Minimum Equipment */
           mCrew;            /**< Useful for Morgans Code */
    int8_t mVab[2];  /**< Hardware Requirements */
    char PCat[5],     /**< Prestige Category List */
         LMAd;             /**< LM Addition Points */
};

#define MAX_MISSIONS 62

int main()
{
    struct mStr mis[MAX_MISSIONS];

    FILE *f = fopen("../../data/gamedata/mission.dat", "rb");
    fread(mis, sizeof(struct mStr), MAX_MISSIONS, f);
    fclose(f);

    f = fopen("mission.dat", "wb");
    fwrite(mis, sizeof(struct mStr), MAX_MISSIONS, f);
    fclose(f);
}
