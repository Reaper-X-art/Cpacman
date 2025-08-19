#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define WIDTH  28
#define HEIGHT 31
#define MAX_GHOSTS 3

// Tiles
#define WALL   '#'
#define EMPTY  ' '
#define PELLET '.'
#define POWER  'o'


#define COLOR_PLAYER 1
#define COLOR_GHOST  2
#define COLOR_WALL   3
#define COLOR_PELLET 4

typedef struct {
    int x, y; 
    int dx, dy;
} Entity;

typedef struct {
    Entity player;
    Entity ghosts[MAX_GHOSTS];
    int ghostScatter; // chance to move randomly
    int score;
    int lives;
    int pelletsRemaining;
    int gameOver;
    int win;
} Game;

static const char *LEVEL[HEIGHT] = {
    "############################",
    "#............##............#",
    "#.####.#####.##.#####.####.#",
    "#o####.#####.##.#####.####o#",
    "#.####.#####.##.#####.####.#",
    "#..........................#",
    "#.####.##.########.##.####.#",
    "#.####.##.########.##.####.#",
    "#......##....##....##......#",
    "######.##### ## #####.######",
    "     #.##### ## #####.#     ",
    "     #.##          ##.#     ",
    "     #.## ###PP### ##.#     ",
    "######.## #      # ##.######",
    "      .   # G  G #   .      ",
    "######.## #      # ##.######",
    "     #.## ######## ##.#     ",
    "     #.##          ##.#     ",
    "     #.##### ## #####.#     ",
    "######.##### ## #####.######",
    "#............##............#",
    "#.####.#####.##.#####.####.#",
    "#o..##................##..o#",
    "###.##.##.########.##.##.###",
    "#......##....##....##......#",
    "#.##########.##.##########.#",
    "#..........................#",
    "#.##########.##.##########.#",
    "#..........................#",
    "############################",
    "                            "
};

static char mapBuf[HEIGHT][WIDTH+1];

static void map_init(Game *g){
    g->pelletsRemaining = 0;
    for(int y=0;y<HEIGHT;y++){
        for(int x=0;x<WIDTH;x++){
            char c = LEVEL[y][x];
            if(c=='P') {
                mapBuf[y][x] = PELLET; // spawn pellet under player start
                g->player.x = x; g->player.y = y; g->player.dx = 0; g->player.dy = 0;
            } else if(c=='G') {
                mapBuf[y][x] = PELLET; // pellet under ghost start
            } else if(c=='#' || c=='.' || c=='o' || c==' '){
                mapBuf[y][x] = c;
            } else if(c==' ') {
                mapBuf[y][x] = EMPTY;
            } else {
                mapBuf[y][x] = (c=='#')?WALL:((c=='.')?PELLET:((c=='o')?POWER:EMPTY));
            }
            if(mapBuf[y][x]==PELLET) g->pelletsRemaining++;
        }
        mapBuf[y][WIDTH]='\0';
    }
}

static int is_wall(int x,int y){
    if(x<0||x>=WIDTH||y<0||y>=HEIGHT) return 1;
    return mapBuf[y][x]==WALL;
}

static void place_ghosts(Game *g){
    int gi=0;
    for(int y=0;y<HEIGHT && gi<MAX_GHOSTS;y++){
        for(int x=0;x<WIDTH && gi<MAX_GHOSTS;x++){
            if(LEVEL[y][x]=='G'){
                g->ghosts[gi].x=x; g->ghosts[gi].y=y; g->ghosts[gi].dx=0; g->ghosts[gi].dy=0; gi++;
            }
        }
    }
}

static void init_colors(){
    if(!has_colors()) return;
    start_color();
    init_pair(COLOR_PLAYER, COLOR_YELLOW, COLOR_BLACK);
    init_pair(COLOR_GHOST,  COLOR_RED,    COLOR_BLACK);
    init_pair(COLOR_WALL,   COLOR_BLUE,   COLOR_BLACK);
    init_pair(COLOR_PELLET, COLOR_WHITE,  COLOR_BLACK);
}

static void draw_game(const Game *g){
    erase();
    // Map
    for(int y=0;y<HEIGHT;y++){
        for(int x=0;x<WIDTH;x++){
            char c = mapBuf[y][x];
            if(c==WALL){
                attron(COLOR_PAIR(COLOR_WALL));
                mvaddch(y, x, '#');
                attroff(COLOR_PAIR(COLOR_WALL));
            } else if(c==PELLET){
                attron(COLOR_PAIR(COLOR_PELLET));
                mvaddch(y, x, '.');
                attroff(COLOR_PAIR(COLOR_PELLET));
            } else if(c==POWER){
                attron(COLOR_PAIR(COLOR_PELLET));
                mvaddch(y, x, 'o');
                attroff(COLOR_PAIR(COLOR_PELLET));
            } else {
                mvaddch(y, x, ' ');
            }
        }
    }
    // Player
    attron(COLOR_PAIR(COLOR_PLAYER) | A_BOLD);
    mvaddch(g->player.y, g->player.x, 'C');
    attroff(COLOR_PAIR(COLOR_PLAYER) | A_BOLD);

    // Ghosts
    for(int i=0;i<MAX_GHOSTS;i++){
        attron(COLOR_PAIR(COLOR_GHOST) | A_BOLD);
        mvaddch(g->ghosts[i].y, g->ghosts[i].x, 'M');
        attroff(COLOR_PAIR(COLOR_GHOST) | A_BOLD);
    }

    // UI
    mvprintw(HEIGHT, 0, "Score: %d   Lives: %d   Pellets: %d   (WASD/arrows to move, Q to quit)",
             g->score, g->lives, g->pelletsRemaining);

    if(g->gameOver){
        const char *msg = g->win?"YOU WIN! Press R to restart or Q to quit":"GAME OVER! Press R to restart or Q to quit";
        mvprintw(HEIGHT/2, (WIDTH - (int)strlen(msg))/2, "%s", msg);
    }

    refresh();
}

static int valid_move(int x,int y){
    return !is_wall(x,y);
}

static void try_move(Entity *e, int ndx,int ndy){
    int nx=e->x+ndx, ny=e->y+ndy;
    if(valid_move(nx,ny)){ e->x=nx; e->y=ny; e->dx=ndx; e->dy=ndy; }
}

static void reset_positions(Game *g){
    // reset to initial positions defined in LEVEL
    for(int y=0;y<HEIGHT;y++){
        for(int x=0;x<WIDTH;x++){
            if(LEVEL[y][x]=='P'){ g->player.x=x; g->player.y=y; g->player.dx=0; g->player.dy=0; }
        }
    }
    place_ghosts(g);
}

static void eat_tile(Game *g){
    char *tile = &mapBuf[g->player.y][g->player.x];
    if(*tile==PELLET){
        *tile=EMPTY; g->score+=10; g->pelletsRemaining--; 
        if(g->pelletsRemaining<=0){ g->gameOver=1; g->win=1; }
    } else if(*tile==POWER){
        *tile=EMPTY; g->score+=50; g->pelletsRemaining--; 
    }
}

static int rand_dir(){
    int r = rand()%4; // 0:up 1:down 2:left 3:right
    int dx=0,dy=0; if(r==0){dy=-1;} else if(r==1){dy=1;} else if(r==2){dx=-1;} else {dx=1;}
    return (dy<<8) | (dx & 0xFF);
}

static void ghost_ai_step(Game *g, Entity *ghost){
    // Very simple: 70% greedy towards player, 30% random (scatter)
    int dx=0,dy=0;
    int chooseRandom = (rand()%100) < g->ghostScatter;
    if(!chooseRandom){
        int bestScore=1e9; int bdx=0,bdy=0;
        int dirs[4][2]={{1,0},{-1,0},{0,1},{0,-1}};
        for(int i=0;i<4;i++){
            int ndx=dirs[i][0], ndy=dirs[i][1];
            int nx=ghost->x+ndx, ny=ghost->y+ndy;
            if(valid_move(nx,ny)){
                int man = abs((g->player.x)-nx)+abs((g->player.y)-ny);
                if(man < bestScore && !(ndx==-ghost->dx && ndy==-ghost->dy)){
                    bestScore = man; bdx=ndx; bdy=ndy;
                }
            }
        }
        dx=bdx; dy=bdy;
        if(dx==0 && dy==0) chooseRandom=1; // dead end
    }
    if(chooseRandom){
        for(int tries=0;tries<8;tries++){
            int packed = rand_dir();
            int ndx = (signed char)(packed & 0xFF);
            int ndy = (signed char)((packed>>8)&0xFF);
            int nx=ghost->x+ndx, ny=ghost->y+ndy;
            if(valid_move(nx,ny) && !(ndx==-ghost->dx && ndy==-ghost->dy)){
                dx=ndx; dy=ndy; break;
            }
        }
    }
    try_move(ghost, dx, dy);
}

static int collided(const Entity *a,const Entity *b){
    return a->x==b->x && a->y==b->y;
}

static void new_game(Game *g){
    memset(g,0,sizeof(*g));
    g->lives=3; g->ghostScatter=30; g->score=0; g->gameOver=0; g->win=0;
    map_init(g);
    reset_positions(g);
}

int main(){
    srand((unsigned)time(NULL));
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE); // non-blocking input
    timeout(70); // ~14 FPS
    init_colors();

    Game g; new_game(&g);

    long tick=0; 
    while(1){
        int ch = getch();
        if(ch=='q' || ch=='Q') break;
        if(g.gameOver){
            if(ch=='r' || ch=='R'){ new_game(&g); }
            draw_game(&g); 
            continue;
        }
        // Input
        int ndx=0,ndy=0;
        switch(ch){
            case KEY_UP: case 'w': case 'W': ndy=-1; break;
            case KEY_DOWN: case 's': case 'S': ndy=1; break;
            case KEY_LEFT: case 'a': case 'A': ndx=-1; break;
            case KEY_RIGHT: case 'd': case 'D': ndx=1; break;
            default: break;
        }
        if(ndx||ndy) try_move(&g.player, ndx, ndy);

        // Eat pellets
        eat_tile(&g);

        // Ghost updates (slower than player for playability)
        if(tick%2==0){
            for(int i=0;i<MAX_GHOSTS;i++) ghost_ai_step(&g, &g.ghosts[i]);
        }

        // Collisions
        for(int i=0;i<MAX_GHOSTS;i++){
            if(collided(&g.player, &g.ghosts[i])){
                g.lives--; 
                if(g.lives<=0){ g.gameOver=1; g.win=0; }
                reset_positions(&g);
                break;
            }
        }

        draw_game(&g);
        tick++;
    }

    endwin();
    return 0;
}
