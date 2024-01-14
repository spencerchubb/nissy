#ifndef CUBE_H
#define CUBE_H

#include <stdio.h>

#include "env.h"
#include "pf.h"
#include "utils.h"

Cube        admissible_ep(Cube cube, PieceFilter f);
int         array_ep_to_epos(int *ep, int *eps_solved);
Cube        arrays_to_cube(CubeArray *arr, PieceFilter f);
Cube        compose(Cube c2, Cube c1); /* Use c2 as an alg on c1 */
Cube        compose_filtered(Cube c2, Cube c1, PieceFilter f);
void        cube_to_arrays(Cube cube, CubeArray *arr, PieceFilter f);
int         edge_slice(Edge e); /* E=0, S=1, M=2 */
bool        equal(Cube c1, Cube c2);
Cube        inverse_cube(Cube cube);
bool        is_admissible(Cube cube);
bool        is_solved(Cube cube);
void        epos_to_partial_ep(int epos, int *ep, int *ss);
void        epos_to_compatible_ep(int epos, int *ep, int *ss);
void        fix_eorleoud(CubeArray *arr);
void        fix_cofbcorl(CubeArray *arr);
Cube        fourval_to_cube(int eofb, int ep, int coud, int cp);
void        free_cubearray(CubeArray *arr, PieceFilter f);
Cube        move_via_arrays(CubeArray *arr, Cube c, PieceFilter pf);
CubeArray * new_cubearray(Cube cube, PieceFilter f);
Center      what_center_at(Cube cube, Center c);
Corner      what_corner_at(Cube cube, Corner c);
Edge        what_edge_at(Cube cube, Edge e);
Edge        where_is_edge(Cube cube, Edge e);

void        init_cube(void);

#endif

