/**
 * Copyright (C) 2005 Michael K. McCarty & Fritz Bronner,
 * Copyright (c) 2020 Ryan Yoakum
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

// This file contains utiliity functions for astronaut / cosmonaut data

#include "astros.h"

#include "data.h"
#include "game_main.h"
#include "options.h"


/**
 * Count how many flight crews are available for mission assignment.
 *
 * \param plr  the player index.
 * \param program  the manned program index (per EquipMannedIndex).
 * \return  the number of assignable crews.
 */
int AvailableCrewsCount(int plr, int program)
{
    int count = 0;

    for (int i = 0; i < ASTRONAUT_CREW_MAX; i++) {
        const int commanderIndex = Data->P[plr].Crew[program][i][0];
        const Astros &commander  = Data->P[plr].Pool[commanderIndex - 1];

        if (commanderIndex != 0 && commander.Prime == 0 &&
            (options.feat_no_cTraining != 0 || commander.Moved == 0)) {
            count++;
        }
    }

    return count;
}


/**
 * Break all flight crews containing invalid astronauts/cosmonauts.
 *
 * \param player  the player index.
 */
void CheckFlightCrews(int player)
{
    int i, j, k, temp;

    for (k = 0; k < ASTRONAUT_POOLS + 1; k++) {
        for (j = 0; j < ASTRONAUT_CREW_MAX; j++) {
            temp = 0;

            if (Data->P[player].CrewCount[k][j] > 0) {
                for (i = 0; i < Data->P[player].CrewCount[k][j] + 1; i++) {
                    if (Data->P[player].Pool[Data->P[player].Crew[k][j][i] - 1].Status == AST_ST_DEAD ||
                        Data->P[player].Pool[Data->P[player].Crew[k][j][i] - 1].Status == AST_ST_RETIRED ||
                        Data->P[player].Pool[Data->P[player].Crew[k][j][i] - 1].Status == AST_ST_INJURED) {
                        temp++;
                    }
                }

                if (temp > 0) {
                    for (i = 0; i < Data->P[player].CrewCount[k][j] + 1; i++) {
                        Data->P[player].Pool[Data->P[player].Crew[k][j][i] - 1].oldAssign =
                            Data->P[player].Pool[Data->P[player].Crew[k][j][i] - 1].Assign;
                        Data->P[player].Pool[Data->P[player].Crew[k][j][i] - 1].Assign = 0;
                        Data->P[player].Pool[Data->P[player].Crew[k][j][i] - 1].Crew = 0;
                        Data->P[player].Pool[Data->P[player].Crew[k][j][i] - 1].Prime = 0;
                        Data->P[player].Pool[Data->P[player].Crew[k][j][i] - 1].Task = 0;
                        Data->P[player].Pool[Data->P[player].Crew[k][j][i] - 1].Moved = 0;

                        if (Data->P[player].Pool[Data->P[player].Crew[k][j][i] - 1].Special == 0) {
                            Data->P[player].Pool[Data->P[player].Crew[k][j][i] - 1].Special = 6;
                        }

                        Data->P[player].Crew[k][j][i] = 0;
                    }

                    Data->P[player].CrewCount[k][j] = 0;
                }
            }
        }
    }

    return;
}


/*
 * Is an astronaut happy with their crewmate?
 *
 * Astronaut compatibility is not transitive. An astronaut may dislike
 * a crewmate who is fine with them.
 *
 * Astronauts / Cosmonauts have a compatibility score of 1-10. They
 * feel compatible with others who have a score within
 * [compat - cl, compat + cr], wrapping.
 *
 * \param ast  the astronaut / cosmonaut whose feelings these are.
 * \param peer  their companion astronaut / cosmonaut.
 * \return  true if ast works well with peer, false otherwise.
 */
bool Compatible(const struct Astros &ast, const struct Astros &peer)
{
    int low = ast.Compat - ast.CL;
    int high = ast.Compat + ast.CR;

    return ((peer.Compat >= low && peer.Compat <= high) ||
            (peer.Compat + 10 >= low && peer.Compat + 10 <= high) ||
            (peer.Compat - 10 >= low && peer.Compat - 10 <= high));
}
