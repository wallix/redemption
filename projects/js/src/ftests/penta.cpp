#include <emscripten.h>

typedef struct
{
    int x;
    int y;
} t_point;

typedef t_point * (t_dessin[5]);
typedef t_dessin * (t_rotations[8]);
enum { T=0,P=1,I=2,U=3,L=4,W=5,Z=6,X=7,F=8,J=9,Y=10,N=11, USED=12, EMPTY=13 };

typedef struct
{
    int sens;
    t_rotations dessins;
} t_pentamino;

typedef t_pentamino * t_formes[12]; /* T .. N */

typedef int t_terrain[16][14];

typedef struct 
{
    t_terrain terrain;
    int nb;
    // stack containing numbers of pieces on board
    int piece[13];
    // stack giving orientation of pieces on board 
    int sens[13];
    // flag saying if a piece is on board or not
    // (to avoid iteration on board to find first hole)
    int etat[13];
    // where we are trying to put the pieces
    t_point pos[13];
} t_penta;

void init(t_penta * penta);
int Pentas(t_penta * penta);

extern t_penta PENTA;

t_point A0 = {0,0};
t_point A1 = {0,1};
t_point A2 = {0,2};
t_point A3 = {0,3};
t_point A4 = {0,4};
t_point MB3 = {1,-3};
t_point MB2 = {1,-2};
t_point MB1 = {1,-1};
t_point B0 = {1,0};
t_point B1 = {1,1};
t_point B2 = {1,2};
t_point B3 = {1,3};
t_point MC2 = {2,-2};
t_point MC1 = {2,-1};
t_point C0 = {2,0};
t_point C1 = {2,1};
t_point C2 = {2,2};
t_point MD1 = {3,-1};
t_point D0 = {3,0};
t_point D1 = {3,1};
t_point E0 = {4,0};

t_dessin NUL = { &A0, &A0, &A0, &A0, &A0};
t_dessin X1 = { &A0, &MB1, &B0, &B1, &C0 };
t_dessin I1 = { &A0, &A1, &A2, &A3, &A4 };
t_dessin I2 = { &A0, &B0, &C0, &D0, &E0 };
t_dessin T1 = { &A0, &B0, &C0, &B1, &B2 };
t_dessin T2 = { &A0, &B0, &MC1, &C0, &C1 };
t_dessin T3 = { &A0, &A1, &A2, &B1, &C1 };
t_dessin T4 = { &A0, &MB2, &MB1, &B0, &C0 };
t_dessin U1 = { &A0, &A1, &B1, &C0, &C1 };
t_dessin U2 = { &A0, &A1, &A2, &B0, &B2 };
t_dessin U3 = { &A0, &A1, &B0, &C0, &C1 };
t_dessin U4 = { &A0, &A2, &B0, &B1, &B2 };
t_dessin L1 = { &A0, &A1, &A2, &B2, &C2 };
t_dessin L2 = { &A0, &A1, &A2, &B0, &C0 };
t_dessin L3 = { &A0, &B0, &C0, &C1, &C2 };
t_dessin L4 = { &A0, &B0, &MC2, &MC1, &C0 };
t_dessin W1 = { &A0, &A1, &B1, &B2, &C2 };
t_dessin W2 = { &A0, &A1, &MB1, &B0, &MC1 };
t_dessin W3 = { &A0, &B0, &B1, &C1, &C2 };
t_dessin W4 = { &A0, &MB1, &B0, &MC2, &MC1 };
t_dessin Z1 = { &A0, &B0, &B1, &B2, &C2 };
t_dessin Z2 = { &A0, &A1, &B0, &MC1, &C0 };
t_dessin Z3 = { &A0, &MB2, &MB1, &B0, &MC2 };
t_dessin Z4 = { &A0, &A1, &B1, &C1, &C2 };
t_dessin P1 = { &A0, &A1, &A2, &B0, &B1 };
t_dessin P2 = { &A0, &B0, &B1, &C0, &C1 };
t_dessin P3 = { &A0, &A1, &MB1, &B0, &B1 };
t_dessin P4 = { &A0, &A1, &B0, &B1, &C1 };
t_dessin P5 = { &A0, &A1, &B0, &B1, &B2 };
t_dessin P6 = { &A0, &MB1, &B0, &MC1, &C0 };
t_dessin P7 = { &A0, &A1, &A2, &B1, &B2 };
t_dessin P8 = { &A0, &A1, &B0, &B1, &C0 };
t_dessin F1 = { &A0, &MB1, &B0, &B1, &MC1 };
t_dessin F2 = { &A0, &MB1, &B0, &C0, &C1 };
t_dessin F3 = { &A0, &MB2, &MB1, &B0, &MC1 };
t_dessin F4 = { &A0, &A1, &B1, &B2, &C1 };
t_dessin F5 = { &A0, &B0, &B1, &B2, &C1 };
t_dessin F6 = { &A0, &B0, &B1, &MC1, &C0 };
t_dessin F7 = { &A0, &MB1, &B0, &B1, &C1 };
t_dessin F8 = { &A0, &A1, &MB1, &B0, &C0 };
t_dessin J1 = { &A0, &A1, &A2, &A3, &B3 };
t_dessin J2 = { &A0, &A1, &B0, &C0, &D0 };
t_dessin J3 = { &A0, &B0, &B1, &B2, &B3 };
t_dessin J4 = { &A0, &B0, &C0, &MD1, &D0 };
t_dessin J5 = { &A0, &MB3, &MB2, &MB1, &B0 };
t_dessin J6 = { &A0, &A1, &B1, &C1, &D1 };
t_dessin J7 = { &A0, &A1, &A2, &A3, &B0 };
t_dessin J8 = { &A0, &B0, &C0, &D0, &D1 };
t_dessin Y1 = { &A0, &B0, &C0, &C1, &D0 };
t_dessin Y2 = { &A0, &MB2, &MB1, &B0, &B1 };
t_dessin Y3 = { &A0, &MB1, &B0, &C0, &D0 };
t_dessin Y4 = { &A0, &A1, &A2, &A3, &B1 };
t_dessin Y5 = { &A0, &B0, &MC1, &C0, &D0 };
t_dessin Y6 = { &A0, &A1, &A2, &A3, &B2 };
t_dessin Y7 = { &A0, &B0, &B1, &C0, &D0 };
t_dessin Y8 = { &A0, &MB1, &B0, &B1, &B2 };
t_dessin N1 = { &A0, &A1, &B1, &B2, &B3 };
t_dessin N2 = { &A0, &B0, &MC1, &C0, &MD1 };
t_dessin N3 = { &A0, &A1, &MB2, &MB1, &B0 };
t_dessin N4 = { &A0, &B0, &B1, &C1, &D1 };
t_dessin N5 = { &A0, &A1, &A2, &MB1, &B0 };
t_dessin N6 = { &A0, &B0, &C0, &C1, &D1 };
t_dessin N7 = { &A0, &A1, &A2, &B2, &B3 };
t_dessin N8 = { &A0, &MB1, &B0, &MC1, &MD1 };

t_pentamino P_X = { 1, {&X1, &NUL, &NUL, &NUL, &NUL, &NUL, &NUL, &NUL}};
t_pentamino P_I = { 2, {&I1, &I2, &NUL, &NUL, &NUL, &NUL, &NUL, &NUL}};
t_pentamino P_T = { 4, {&T1, &T2, &T3, &T4, &NUL, &NUL, &NUL, &NUL}};
t_pentamino P_U = { 4, {&U1, &U2, &U3, &U4, &NUL, &NUL, &NUL, &NUL}};
t_pentamino P_L = { 4, {&L1, &L2, &L3, &L4, &NUL, &NUL, &NUL, &NUL}};
t_pentamino P_W = { 4, {&W1, &W2, &W3, &W4, &NUL, &NUL, &NUL, &NUL}};
t_pentamino P_Z = { 4, {&Z1, &Z2, &Z3, &Z4, &NUL, &NUL, &NUL, &NUL}};
t_pentamino P_P = { 8, {&P1, &P2, &P3, &P4, &P5, &P6, &P7, &P8}};
t_pentamino P_F = { 8, {&F1, &F2, &F3, &F4, &F5, &F6, &F7, &F8}};
t_pentamino P_J = { 8, {&J1, &J2, &J3, &J4, &J5, &J6, &J7, &J8}};
t_pentamino P_Y = { 8, {&Y1, &Y2, &Y3, &Y4, &Y5, &Y6, &Y7, &Y8}};
t_pentamino P_N = { 8, {&N1, &N2, &N3, &N4, &N5, &N6, &N7, &N8}};

t_formes FORMES = {&P_T, &P_P, &P_I, &P_U, &P_L, &P_W, &P_Z, &P_X, &P_F, &P_J, &P_Y, &P_N};

t_terrain TERRAIN_INITIAL = {
{ USED, USED, USED, USED, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, USED, USED, USED, USED, USED},
{ USED, USED, USED, USED, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, USED, USED, USED, USED, USED},
{ USED, USED, USED, USED, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, USED, USED, USED, USED, USED},
{ USED, USED, USED, USED, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, USED, USED, USED, USED, USED},
{ USED, USED, USED, USED, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, USED, USED, USED, USED, USED},
{ USED, USED, USED, USED, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, USED, USED, USED, USED, USED},
{ USED, USED, USED, USED, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, USED, USED, USED, USED, USED},
{ USED, USED, USED, USED, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, USED, USED, USED, USED, USED},
{ USED, USED, USED, USED, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, USED, USED, USED, USED, USED},
{ USED, USED, USED, USED, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, USED, USED, USED, USED, USED},
{ USED, USED, USED, USED, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, USED, USED, USED, USED, USED},
{ USED, USED, USED, USED, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, USED, USED, USED, USED, USED},
{ USED, USED, USED, USED, USED, USED, USED, USED, USED, USED, USED, USED, USED, USED},
{ USED, USED, USED, USED, USED, USED, USED, USED, USED, USED, USED, USED, USED, USED},
{ USED, USED, USED, USED, USED, USED, USED, USED, USED, USED, USED, USED, USED, USED},
{ USED, USED, USED, USED, USED, USED, USED, USED, USED, USED, USED, USED, USED, USED}
};

t_penta PENTA;

void init_terrain(t_penta * penta)
{
    int q,p;
    for (q=0; q < 16; q++)
    {
        for (p=0; p < 14; p++)
        {
            penta->terrain[q][p] = TERRAIN_INITIAL[q][p];
        }
    }
    return;
}

void init(t_penta * penta)
{
    int q;
    init_terrain(penta);
    penta->nb = T;
    for (q = 0; q < 12; q++)
    {
        penta->etat[q] = 0;
        penta->piece[q] = T;
        penta->sens[q] = 0;
        penta->pos[q].x = 0;
        penta->pos[q].y = 4;
    }
    return;
}

int fit(t_penta * penta)
{
    t_terrain * t = &penta->terrain;
    t_point * p = &penta->pos[penta->nb];
    int piece = penta->piece[penta->nb];
    int sens = penta->sens[penta->nb];

    if (0!=(penta->etat[piece]))
    {
        return 0;
    }
 
    int q = 0;
    t_dessin * dessin = FORMES[piece]->dessins[sens];
    for (q = 0; q < 5; q++)
    {
        if ((*t)[p->x+(*dessin)[q]->x][p->y+(*dessin)[q]->y] != EMPTY)
        {
            return 0;
        }
    }
    return 1;
}

void put(t_penta * penta)
{
    int q;
    t_terrain * t;
    t_point * p;
    int piece;
    int sens;
    t_dessin * dessin;

    t = &penta->terrain;
    p = &penta->pos[penta->nb];
    piece = penta->piece[penta->nb];

    sens = penta->sens[penta->nb];
    dessin = FORMES[piece]->dessins[sens];
    penta->etat[piece] = 1;
    for (q = 0; q < 5; q++)
    {
        (*t)[p->x+(*dessin)[q]->x][p->y+(*dessin)[q]->y] = piece;
    }
    return;
}

void remove(t_penta * penta)
{
    int q;
    t_terrain * t;
    t_point * p;
    int piece;
    int sens;
    t_dessin * dessin;

    t = &penta->terrain;
    p = &penta->pos[penta->nb];
    piece = penta->piece[penta->nb];

    penta->etat[piece] = 0;
    sens = penta->sens[penta->nb];
    dessin = FORMES[piece]->dessins[sens];
    for (q = 0; q < 5; q++)
    {
        (*t)[p->x+(*dessin)[q]->x][p->y+(*dessin)[q]->y] = EMPTY;
    }
    return;
}

void new_position(t_penta * penta)
{
    int qx;int qy;
    for(qx = 0 ; qx < 12 ; qx++)
    {
        for(qy = 4 ; qy < 5+4 ; qy++)
        {
            if (penta->terrain[qx][qy] == EMPTY)
            {
                penta->pos[penta->nb].x = qx;
                penta->pos[penta->nb].y = qy;
                return;
            }
        }
    }
    return;
}

// retourne 0 lorsque toutes les solutions ont ete trouvees
// ou le nombre de pentaminos poses sur le jeu
int Pentas(t_penta * penta)
{
    for(;;)
    {
        while ((penta->nb<12) && (penta->piece[penta->nb] <= N))
        {
            while (penta->sens[penta->nb] < FORMES[penta->piece[penta->nb]]->sens)
            {
                if (fit(penta))
                {
                    // si oui put la piece et passer a la piece suivante
                    put(penta);penta->nb++;
                    if (penta->nb < 12)
                    {
                        new_position(penta);
                        penta->piece[penta->nb] = T;
                        penta->sens[penta->nb] = 0;
                    }
                    return penta->nb;
                }
                else
                {
                    penta->sens[penta->nb]++;
                }
            }
            // passer a la piece suivante
            penta->piece[penta->nb]++;
            penta->sens[penta->nb] = 0;
        }

        // continuer au niveau precedent
        if (penta->nb < 1)
        {
            // on a essaye toutes les pieces
            // dans toutes les positions
            return 0;
        }
        // remove la derniere piece testee
        penta->nb--;remove(penta);
        if (penta->nb >= 0)
            new_position(penta);
        // passer au sens suivant de la piece suivante 
        // (sinon on va reessayer la meme chose)
        penta->sens[penta->nb]++;
    }
    return 0;
}

// ================== Code containing external calls ===================
void save_solution()
{
    int qx;
    int qy;
                        
    for(qy = 4+4 ; qy >= 4 ; qy--) // 8, 7, 6, 5, 4
    {
        for(qx = 0 ; qx < 12 ; qx++) // 0, 1, 2, 3, 4, ... 11
        {
            int piece = PENTA.terrain[qx][qy];
            EM_ASM_({
                square($0, $1, $0+1, $1+1, $2);
            }, qx, qy-4, piece);
        }
    }
    return;
}

extern "C" {
    void run_main()
    {  
        static int num = 0;
        if (!num) {
            init(&PENTA);
        }
        for (;;){
            switch (Pentas(&PENTA))
            {
            case 12:
                EM_ASM_({
                    Module.print('Solution found ' + $0);
                }, num);
                save_solution();
                num++;
                return;
            case 0:
                return;
            default:
                break;
            } 
        }
        return;  
    }  
}
